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

    Feature(const Axis &axis_, const std::shared_ptr<double> &bound_lower_, const std::shared_ptr<double> &bound_upper_, const size_t &depth_, const bool &upper_, const double &midpt_, const bool &midpt_raw_, const size_t &midpt_update_count_ = 1u)
     : Feature_Ranged<Feature>(axis_, bound_lower_, bound_upper_, depth_, upper_, midpt_, midpt_raw_, midpt_update_count_)
    {
    }

    Feature * clone() const {
      return new Feature(Axis(this->axis), this->bound_lower, this->bound_upper, this->depth, this->upper, this->midpt_raw, false, this->midpt_update_count);
    }

    void print(ostream &os) const {
      switch(axis) {
        case X:     os << 'x';     break;
        case X_DOT: os << "x-dot"; break;
        default: abort();
      }

      os << '(' << *bound_lower << ',' << *bound_upper << ':' << depth << ')';
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

    const double & get_x() const {return m_x;}
    const double & get_x_dot() const {return m_x_dot;}
    const double & get_value(const Feature_Axis &index) const {return *(&m_x + index);}
    bool is_random_start() const {return m_random_start;}
    bool is_reward_negative() const {return m_reward_negative;}

    void set_x(const double &x_) {m_x = x_;}
    void set_x_dot(const double &x_dot_) {m_x_dot = x_dot_;}

    bool success() const {
      return m_x >= m_goal_position;
    }

  private:
    void init_impl() {
      if(m_random_start) {
        m_x = m_random_init.frand_lt() * (m_goal_position - m_min_position) + m_min_position;
        m_x_dot = m_random_init.frand_lt() * (2 * m_max_velocity) - m_max_velocity;
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
      const int a = int(static_cast<const Move &>(action).direction);

      assert(0 <= a && a <= 2);

      m_x_dot += (a - 1) * m_cart_force + cos(3 * m_x) * -m_grav_force;
      m_x_dot = std::max(-m_max_velocity, std::min(m_max_velocity, m_x_dot));

      m_x += m_x_dot;
      m_x = std::max(m_min_position, std::min(m_max_position, m_x));

      if(m_x == m_min_position && m_x_dot < 0)
        m_x_dot = 0;

      return m_reward_negative ? -1 : success() ? 1 : 0;
    }

    void print_impl(ostream &os) const {
      os << "Mountain Car:" << endl;
      os << " (" << m_x << ", " << m_x_dot << ')' << endl;
    }

    Zeni::Random m_random_init;

    double m_x = 0.0;
    double m_x_dot = 0.0;

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
     : ::Agent<feature_type, action_type>(env),
     m_min_x(std::make_shared<double>(-1.2)),
     m_max_x(std::make_shared<double>(0.6)),
     m_min_x_dot(std::make_shared<double>(-0.07)),
     m_max_x_dot(std::make_shared<double>(0.07))
    {
//      m_features_complete = false;
    }

//    void print_policy(ostream &os, const size_t &granularity) {
//      auto env = dynamic_pointer_cast<Environment>(get_env());
//      const auto x_bak = env->get_x();
//      const auto x_dot_bak = env->get_x_dot();
//
//      for(size_t x_dot = granularity; x_dot != 0lu; --x_dot) {
//        env->set_x_dot(((x_dot - 0.5) / granularity) * 0.14 - 0.07);
//        for(size_t x = 0lu; x != granularity; ++x) {
//          env->set_x(((x + 0.5) / granularity) * 1.8 - 1.2);
//          regenerate_lists();
//          auto action = choose_greedy();
//          switch(static_cast<const Move &>(*action).direction) {
//            case Move::LEFT:  os << '-'; break;
//            case Move::IDLE:  os << '0'; break;
//            case Move::RIGHT: os << '+'; break;
//            default: abort();
//          }
//        }
//        os << endl;
//      }
//
//      env->set_x(x_bak);
//      env->set_x_dot(x_dot_bak);
//      regenerate_lists();
//    }

  private:
//    set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &trie) const {
//      return generate_vfgs_for_axes(trie, feature_type::Axis::X, feature_type::Axis::X_DOT);
//    }

//    map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie) const {
//      return generate_ucm_for_axes(trie, feature_type::Axis::X, feature_type::Axis::X_DOT);
//    }

    void generate_features() {
//      auto env = dynamic_pointer_cast<const Environment>(get_env());
//
//      for(const action_type &action_ : *m_candidates) {
//        auto &features = get_feature_list(action_);
//        assert(!features);
//
//        Feature::List * x_tail = &(new Feature(Feature::X, m_min_x, m_max_x, 0, false, env->get_x(), true))->features;
//        x_tail = x_tail->insert_in_order<feature_type::List::compare_default>(features, false);
//        Feature::List * x_dot_tail = &(new Feature(Feature::X_DOT, m_min_x_dot, m_max_x_dot, 0, false, env->get_x_dot(), true))->features;
//        x_dot_tail = x_dot_tail->insert_in_order<feature_type::List::compare_default>(features, false);
//
//        feature_trie trie = get_trie(action_);
//
//        for(;;) {
//          if(generate_feature_ranged(env, features, trie, x_tail, env->get_x()))
//            continue;
//          if(generate_feature_ranged(env, features, trie, x_dot_tail, env->get_x_dot()))
//            continue;
//
//          break;
//        }
//      }
    }

//    void generate_candidates() {
//      auto env = dynamic_pointer_cast<const Environment>(get_env());
//
//      assert(!m_candidates);
//
//      (new Move(Move::LEFT))->candidates.insert_before(m_candidates);
//      (new Move(Move::IDLE))->candidates.insert_before(m_candidates);
//      (new Move(Move::RIGHT))->candidates.insert_before(m_candidates);
//    }

    void update() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->success() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
    }

    std::shared_ptr<double> m_min_x;
    std::shared_ptr<double> m_max_x;
    std::shared_ptr<double> m_min_x_dot;
    std::shared_ptr<double> m_max_x_dot;
  };

}

#endif
