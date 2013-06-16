#ifndef CART_POLE_H
#define CART_POLE_H

#include "agent.h"
#include "environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>

namespace Cart_Pole {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::make_pair;
  using std::map;
  using std::ostream;
  using std::pair;
  using std::set;
  using std::shared_ptr;

  class Feature;
  class Feature : public Feature_Ranged<Feature> {
  public:
    enum Axis : int {X, X_DOT, THETA, THETA_DOT};

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_, const double &midpt_, const size_t &midpt_update_count_ = 1u)
     : Feature_Ranged<Feature>(axis_, bound_lower_, bound_higher_, depth_, midpt_, midpt_update_count_)
    {
    }

    Feature * clone() const {
      return new Feature(Axis(this->axis), this->bound_lower, this->bound_higher, this->depth, this->midpt, this->midpt_update_count);
    }

    void print(ostream &os) const {
      switch(axis) {
        case X:         os << 'x';         break;
        case X_DOT:     os << "x-dot";     break;
        case THETA:     os << "theta";     break;
        case THETA_DOT: os << "theta-dot"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_higher << ':' << depth << ')';
    }
  };

  typedef Feature feature_type;

  class Move;
  typedef Action<Move, Move> action_type;

  class Move : public action_type {
  public:
    enum Direction : char {LEFT, RIGHT};

    Move(const Direction &direction_ = LEFT)
     : direction(direction_)
    {
    }

    Move * clone() const {
      return new Move(direction);
    }

    int compare(const Move &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "move(";

      switch(direction) {
        case LEFT:  os << "left";  break;
        case RIGHT: os << "right"; break;
        default: abort();
      }

      os << ')';
    }

    Direction direction;
  };

  class Environment : public ::Environment<action_type> {
  public:
    typedef pair<double, double> double_pair;

    Environment() {
      Environment::init_impl();
    }

    const double & get_x() const {return m_x;}
    const double & get_x_dot() const {return m_x_dot;}
    const double & get_theta() const {return m_theta;}
    const double & get_theta_dot() const {return m_theta_dot;}
    const double & get_value(const Feature_Axis &index) const {return *(&m_x + index);}
    const double & get_max_x() const {return m_max_x;}
    const double & get_max_x_dot() const {return m_max_x_dot;}
    const double & get_max_theta() const {return m_max_theta;}
    const double & get_max_theta_dot() const {return m_max_theta_dot;}
    bool has_goal() const {return m_has_goal;}
    bool is_ignoring_x() const {return m_ignore_x;}

    void set_x(const double &x_) {m_x = x_;}
    void set_x_dot(const double &x_dot_) {m_x = x_dot_;}
    void set_theta_dot(const double &theta_dot_) {m_theta = theta_dot_;}
    void set_theta(const double &theta_) {m_theta = theta_;}

    bool success() const {
      return m_has_goal &&
             (m_ignore_x || (m_x >= 4.1 && m_x <= 4.5)) &&
             m_theta >= -0.06981317007 && m_theta <= 0.06981317007; ///< pi / 45.0
    }

    bool failed() const {
      return m_x < -m_max_x || m_x > m_max_x ||
             m_theta < -m_max_theta || m_theta > m_max_theta;
    }

  private:
    void init_impl() {
      m_x = 0.0;
      m_x_dot = 0.0;
      m_theta = 0.0;
      m_theta_dot = 0.0;

      if(m_has_goal) {
        do {
          if(!m_ignore_x) {
            m_x = m_random_init.frand_lte() * (m_max_x - 0.2) - m_max_x / 2.0; ///< Inner half of range
            if(m_x >= 4.1 && m_x <= 4.5)
              m_x += 0.4;
          }

          m_theta = m_random_init.frand_lte() * (m_max_theta - 0.06981317007) - m_max_theta / 2.0; ///< Inner half of range
          if(m_theta >= -0.06981317007 && m_theta <= 0.06981317007)
            m_theta += 2 * 0.06981317007;
        } while(success());
      }

      m_random_motion = Zeni::Random(m_random_init.rand());
    }

    reward_type transition_impl(const action_type &action) {
      const bool move_right = static_cast<const Move &>(action).direction == Move::RIGHT;

      const double force = move_right > 0 ? FORCE_MAG : -FORCE_MAG;
      const double costheta = cos(m_theta);
      const double sintheta = sin(m_theta);

      const double temp = (force + POLEMASS_LENGTH * m_theta_dot * m_theta_dot * sintheta) / TOTAL_MASS;

      const double thetaacc = (GRAVITY * sintheta - costheta* temp) /
                               (LENGTH * (FOURTHIRDS - MASSPOLE * costheta * costheta / TOTAL_MASS));

      if(!m_ignore_x) {
        const double xacc  = temp - POLEMASS_LENGTH * thetaacc* costheta / TOTAL_MASS;

        m_x += TAU * m_x_dot;
        m_x_dot += std::max(-m_max_x_dot, std::min(m_max_x_dot, m_x_dot + TAU * xacc));
      }

      m_theta += TAU * m_theta_dot;
      m_theta_dot = std::max(-m_max_theta_dot, std::min(m_max_theta_dot, m_theta_dot + TAU * thetaacc));

      return success() ? 1.0 : failed() ? -1.0 : 0.0;
    }

    void print_impl(ostream &os) const {
      os << "Cart Pole:" << endl;
      os << " (" << m_x << ", " << m_x_dot << ", " << m_theta << ", " << m_theta_dot << ')' << endl;
    }

    Zeni::Random m_random_init;
    Zeni::Random m_random_motion;

    double m_x = 0.0;
    double m_x_dot = 0.0;
    double m_theta = 0.0;
    double m_theta_dot = 0.0;

    const double GRAVITY = 9.8;
    const double MASSCART = 1.0;
    const double MASSPOLE = 0.1;
    const double TOTAL_MASS = MASSPOLE + MASSCART;
    const double LENGTH = 0.5;      /* actually half the pole's length */
    const double POLEMASS_LENGTH = MASSPOLE * LENGTH;
    const double FORCE_MAG = 10.0;
    const double TAU = 0.02;      /* seconds between state updates */
    const double FOURTHIRDS = 1.3333333333333;

    const bool m_has_goal = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["set-goal"]).get_value();
    const bool m_ignore_x = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["ignore-x"]).get_value();

    const double m_max_x = m_has_goal ? 10.0 : 2.4;
    const double m_max_x_dot = m_has_goal ? 4.0 : 10.0;
    const double m_max_theta = m_has_goal ? 1.57079632679 : 0.2094384; ///< pi/2.0 : 12 degrees
    const double m_max_theta_dot = m_has_goal ? 2.0 : 10.0;
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    Agent(const shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env),
     m_ignore_x(false)
    {
      m_features_complete = false;
    }

    bool is_ignoring_x() const {return m_ignore_x;}

  private:
    set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &trie) const {
      return generate_vfgs_for_axes(trie, feature_type::Axis::THETA, feature_type::Axis::THETA_DOT);
    }

    map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie) const {
      return generate_ucm_for_axes(trie, feature_type::Axis::THETA, feature_type::Axis::THETA_DOT);
    }

    void generate_features() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_features);

      Feature::List * x_tail = nullptr;
      Feature::List * x_dot_tail = nullptr;
      if(!m_ignore_x) {
        x_tail = &(new Feature(Feature::X, -env->get_max_x(), env->get_max_x(), 0, env->get_x()))->features;
        x_tail = x_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
        x_dot_tail = &(new Feature(Feature::X_DOT, -env->get_max_x_dot(), env->get_max_x_dot(), 0, env->get_x_dot()))->features;
        x_dot_tail = x_dot_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
      }
      Feature::List * theta_tail = &(new Feature(Feature::THETA, -env->get_max_theta(), env->get_max_theta(), 0, env->get_theta()))->features;
      theta_tail = theta_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
      Feature::List * theta_dot_tail = &(new Feature(Feature::THETA_DOT, -env->get_max_theta_dot(), env->get_max_theta_dot(), 0, env->get_theta_dot()))->features;
      theta_dot_tail = theta_dot_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);

      std::array<feature_trie, 2> tries = {{get_trie(Move(Move::LEFT)),
                                            get_trie(Move(Move::RIGHT))}};

      for(;;) {
        Feature::List * x_tail_next = nullptr;
        Feature::List * x_dot_tail_next = nullptr;
        Feature::List * theta_tail_next = nullptr;
        Feature::List * theta_dot_tail_next = nullptr;

        for(feature_trie &trie : tries) {
          if(!m_ignore_x) {
            if(generate_feature_ranged(env, trie, x_tail, x_tail_next, env->get_x()))
              continue;
            if(generate_feature_ranged(env, trie, x_dot_tail, x_dot_tail_next, env->get_x_dot()))
              continue;
          }
          if(generate_feature_ranged(env, trie, theta_tail, theta_tail_next, env->get_theta()))
            continue;
          if(generate_feature_ranged(env, trie, theta_dot_tail, theta_dot_tail_next, env->get_theta_dot()))
            continue;
        }

        if(x_tail_next)
          x_tail = x_tail_next;
        if(x_dot_tail_next)
          x_dot_tail = x_dot_tail_next;
        if(theta_dot_tail_next)
          theta_dot_tail = theta_dot_tail_next;
        if(theta_tail_next)
          theta_tail = theta_tail_next;

        if(!x_tail_next && !x_dot_tail_next && !theta_tail_next && !theta_dot_tail_next)
          break;
      }
    }

    void generate_candidates() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_candidates);

      (new Move(Move::LEFT))->candidates.insert_before(m_candidates);
      (new Move(Move::RIGHT))->candidates.insert_before(m_candidates);
    }

    void update() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      if(env->failed())
        m_metastate = Metastate::FAILURE;
      else if(env->has_goal() ? env->success() : get_step_count() > 9999)
        m_metastate = Metastate::SUCCESS;
      else
        m_metastate = Metastate::NON_TERMINAL;
    }

    const bool m_ignore_x = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["ignore-x"]).get_value();
  };

}

#endif
