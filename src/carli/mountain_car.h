#ifndef MOUNTAIN_CAR_H
#define MOUNTAIN_CAR_H

#include "agent.h"
#include "environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>

namespace Mountain_Car {

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
    enum Axis : int {X, X_DOT};

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_)
     : Feature_Ranged<Feature>(axis_, bound_lower_, bound_higher_, depth_)
    {
    }

    Feature * clone() const {
      return new Feature(Axis(this->axis), this->bound_lower, this->bound_higher, this->depth);
    }

    void print(ostream &os) const {
      switch(axis) {
        case X:     os << 'x';     break;
        case X_DOT: os << "x-dot"; break;
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
    enum Direction : char {LEFT = 0, IDLE = 1, RIGHT = 2};

    Move(const Direction &direction_ = IDLE)
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
        case IDLE:  os << "idle";  break;
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
    const float & get_value(const Feature_Axis &index) const {return *(&m_x + index);}
    bool is_random_start() const {return m_random_start;}
    bool is_reward_negative() const {return m_reward_negative;}

    void set_x(const float &x_) {m_x = x_;}
    void set_x_dot(const float &x_dot_) {m_x_dot = x_dot_;}

    bool success() const {
      return m_x >= m_goal_position;
    }

  private:
    void init_impl() {
      if(m_random_start) {
        m_x = float(m_random_init.frand_lt() * (m_goal_position - m_min_position) + m_min_position);
        m_x_dot = float(m_random_init.frand_lt() * (2 * m_max_velocity) - m_max_velocity);
      }
      else {
        m_x = -0.5;
        m_x_dot = 0.0;
      }
    }

    void alter_impl() {
      if(get_scenario() < 400)
        return;

      else if(get_scenario() < 410) {
        m_cart_force = 0.0009;
      }
      else if(get_scenario() < 420) {
        m_cart_force = 0.0008;
      }
      else if(get_scenario() < 500) {
        m_cart_force = 0.0006;
      }
    }

    reward_type transition_impl(const action_type &action) {
      const int a = int(dynamic_cast<const Move &>(action).direction);

      assert(0 <= a && a <= 2);

      m_x_dot += float((a - 1) * m_cart_force + cos(3 * m_x) * -m_grav_force);
      m_x_dot = std::max(float(-m_max_velocity), std::min(float(m_max_velocity), m_x_dot));

      m_x += float(m_x_dot);
      m_x = std::max(float(m_min_position), std::min(float(m_max_position), m_x));

      if(m_x == m_min_position && m_x_dot < 0)
        m_x_dot = 0;

      return m_reward_negative ? -1 : success() ? 1 : 0;
    }

    void print_impl(ostream &os) const {
      os << "Mountain Car:" << endl;
      os << " (" << m_x << ", " << m_x_dot << ')' << endl;
    }

    Zeni::Random m_random_init;

    float m_x = 0.0f;
    float m_x_dot = 0.0f;

    const double m_min_position = -1.2;
    const double m_max_position = 0.6;
    const double m_max_velocity = 0.07;
    const double m_goal_position = 0.5;

    double m_cart_force = 0.001;
    double m_grav_force = 0.0025;

    const bool m_random_start = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["random-start"]).get_value();
    const bool m_reward_negative = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["reward-negative"]).get_value();
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    Agent(const shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env)
    {
      m_features_complete = false;
    }

    void print_policy(ostream &os, const size_t &granularity) {
      auto env = dynamic_pointer_cast<Environment>(get_env());
      const auto x_bak = env->get_x();
      const auto x_dot_bak = env->get_x_dot();

      for(size_t x_dot = granularity; x_dot != 0lu; --x_dot) {
        env->set_x_dot(float(((x_dot - 0.5) / granularity) * 0.14 - 0.07));
        for(size_t x = 0lu; x != granularity; ++x) {
          env->set_x(float(((x + 0.5) / granularity) * 1.8 - 1.2));
          regenerate_lists();
          auto action = choose_greedy();
          switch(dynamic_cast<const Move &>(*action).direction) {
            case Move::LEFT:  os << '0'; break;
            case Move::IDLE:  os << '-'; break;
            case Move::RIGHT: os << '1'; break;
            default: abort();
          }
        }
        os << endl;
      }

      env->set_x(x_bak);
      env->set_x_dot(x_dot_bak);
      regenerate_lists();
    }

  private:
    set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &trie) const {
      return generate_vfgs_for_axes(trie, feature_type::Axis::X, feature_type::Axis::X_DOT);
    }

    map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie) const {
      return generate_ucm_for_axes(trie, feature_type::Axis::X, feature_type::Axis::X_DOT);
    }

    void generate_features() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_features);

      Feature::List * x_tail = &(new Feature(Feature::X, -1.2, 0.6, 0))->features;
      x_tail = x_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
      Feature::List * x_dot_tail = &(new Feature(Feature::X_DOT, -0.07, 0.07, 0))->features;
      x_dot_tail = x_dot_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);

      std::array<feature_trie, 3> tries = {{get_trie(Move(Move::LEFT)),
                                            get_trie(Move(Move::IDLE)),
                                            get_trie(Move(Move::RIGHT))}};

      for(;;) {
        Feature::List * x_tail_next = nullptr;
        Feature::List * x_dot_tail_next = nullptr;

        for(feature_trie &trie : tries) {
          if(generate_feature_ranged(env, trie, x_tail, x_tail_next))
            continue;
          if(generate_feature_ranged(env, trie, x_dot_tail, x_dot_tail_next))
            continue;
        }

        if(x_tail_next)
          x_tail = x_tail_next;
        if(x_dot_tail_next)
          x_dot_tail = x_dot_tail_next;

        if(!x_tail_next && !x_dot_tail_next)
          break;
      }
    }

    void generate_candidates() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_candidates);

      (new Move(Move::LEFT))->candidates.insert_before(m_candidates);
      (new Move(Move::IDLE))->candidates.insert_before(m_candidates);
      (new Move(Move::RIGHT))->candidates.insert_before(m_candidates);
    }

    void update() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->success() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
    }
  };

}

#endif
