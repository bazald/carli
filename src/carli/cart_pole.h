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

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_)
     : Feature_Ranged<Feature>(axis_, bound_lower_, bound_higher_, depth_)
    {
    }

    Feature * clone() const {
      return new Feature(Axis(this->axis), this->bound_lower, this->bound_higher, this->depth);
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

    const float & get_x() const {return m_x;}
    const float & get_x_dot() const {return m_x_dot;}
    const float & get_theta() const {return m_theta;}
    const float & get_theta_dot() const {return m_theta_dot;}
    const float & get_value(const Feature_Axis &index) const {return *(&m_x + index);}
    bool is_ignoring_x() const {return m_ignore_x;}

    void set_x(const float &x_) {m_x = x_;}
    void set_x_dot(const float &x_dot_) {m_x = x_dot_;}
    void set_theta_dot(const float &theta_dot_) {m_theta = theta_dot_;}
    void set_theta(const float &theta_) {m_theta = theta_;}

    bool failed() const {
      return get_box(m_x, m_x_dot, m_theta, m_theta_dot) < 0;
    }

  private:
    void init_impl() {
      m_x = 0.0;
      m_x_dot = 0.0;
      m_theta = 0.0;
      m_theta_dot = 0.0;

      m_random_motion = Zeni::Random(m_random_init.rand());
    }

    reward_type transition_impl(const action_type &action) {
      cart_pole(dynamic_cast<const Move &>(action).direction == Move::RIGHT, &m_x, &m_x_dot, &m_theta, &m_theta_dot);

      if(m_ignore_x) {
        m_x = 0.0;
        m_x_dot = 0.0;
      }

      return failed() ? -1.0 : 0.0;
    }

    void print_impl(ostream &os) const {
      os << "Cart Pole:" << endl;
      os << " (" << m_x << ", " << m_x_dot << ", " << m_theta << ", " << m_theta_dot << ')' << endl;
    }

    static double prob_push_right(const double &s);
    float pole_random();
    void cart_pole(int action, float *x, float *x_dot, float *theta, float *theta_dot);
    static int get_box(float x, float x_dot, float theta, float theta_dot);

    Zeni::Random m_random_init;
    Zeni::Random m_random_motion;

    float m_x = 0.0f;
    float m_x_dot = 0.0f;
    float m_theta = 0.0f;
    float m_theta_dot = 0.0f;
    const bool m_ignore_x = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["ignore-x"]).get_value();
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

    map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie, const line_segment_type &extents = line_segment_type(point_type(), point_type(1.0, 1.0)), const size_t &update_count = 0) const {
      return generate_ucm_for_axes(trie, feature_type::Axis::THETA, feature_type::Axis::THETA_DOT);
    }

    void generate_features() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_features);

      Feature::List * x_tail = nullptr;
      Feature::List * x_dot_tail = nullptr;
      if(!m_ignore_x) {
        x_tail = &(new Feature(Feature::X, -2.4, 2.4, 0))->features;
        x_tail = x_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
        x_dot_tail = &(new Feature(Feature::X_DOT, -10, 10, 0))->features;
        x_dot_tail = x_dot_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
      }
      Feature::List * theta_tail = &(new Feature(Feature::THETA, -0.2094384, 0.2094384, 0))->features;
      theta_tail = theta_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
      Feature::List * theta_dot_tail = &(new Feature(Feature::THETA_DOT, -10, 10, 0))->features;
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
            if(generate_feature_ranged(env, trie, x_tail, x_tail_next))
              continue;
            if(generate_feature_ranged(env, trie, x_dot_tail, x_dot_tail_next))
              continue;
          }
          if(generate_feature_ranged(env, trie, theta_tail, theta_tail_next))
            continue;
          if(generate_feature_ranged(env, trie, theta_dot_tail, theta_dot_tail_next))
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
      else if(get_step_count() > 9999)
        m_metastate = Metastate::SUCCESS;
      else
        m_metastate = Metastate::NON_TERMINAL;
    }

    const bool m_ignore_x = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["ignore-x"]).get_value();
  };

}

#endif
