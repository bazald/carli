#ifndef PUDDLE_WORLD_H
#define PUDDLE_WORLD_H

#include "agent.h"
#include "environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>
#include <vector>

namespace Puddle_World {

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
    enum Axis : int {X, Y};

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_, const double &midpt_, const size_t &midpt_update_count_ = 1u)
     : Feature_Ranged<Feature>(axis_, bound_lower_, bound_higher_, depth_, midpt_, midpt_update_count_)
    {
    }

    Feature * clone() const {
      return new Feature(Axis(this->axis), this->bound_lower, this->bound_higher, this->depth, this->midpt, this->midpt_update_count);
    }

    void print(ostream &os) const {
      switch(axis) {
        case X: os << 'x'; break;
        case Y: os << 'y'; break;
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
    enum Direction : char {NORTH, SOUTH, EAST, WEST};

    Move(const Direction &direction_ = NORTH)
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
        case NORTH: os << "north"; break;
        case SOUTH: os << "south"; break;
        case EAST:  os << "east";  break;
        case WEST:  os << "west";  break;
        default: abort();
      }

      os << ')';
    }

    Direction direction;
  };

  class Environment : public ::Environment<action_type> {
    typedef std::array<double, 4> Puddle;

  public:
    typedef pair<double, double> double_pair;

    Environment()
     : m_horizontal_puddles({{{0.1, 0.45, 0.75, 0.1}}}),
     m_vertical_puddles({{{0.45, 0.4, 0.8, 0.1}}})
    {
      if(m_random_start) {
        m_init_x = double_pair(0.0, 1.0);
        m_init_y = double_pair(0.0, 1.0);
      }
      else {
        m_init_x = double_pair(0.15, 0.45);
        m_init_y = double_pair(0.15, 0.45);
      }

      Environment::init_impl();
    }

    const double_pair & get_position() const {return m_position;}
    const double & get_value(const Feature_Axis &index) const {return *(&m_position.first + index);}
    bool is_random_start() const {return m_random_start;}

    void set_position(const double_pair &position_) {m_position = position_;}

    bool goal_reached() const {
      if(m_goal_dynamic)
        return m_goal_x.first <= m_position.first  && m_position.first  < m_goal_x.second &&
               m_goal_y.first <= m_position.second && m_position.second < m_goal_y.second;
      else
        return m_position.first + m_position.second > 1.9;
    }

  private:
    void init_impl() {
      do {
        m_position.first = m_init_x.first + m_random_init.frand_lt() * (m_init_x.second - m_init_x.first);
        m_position.second = m_init_y.first + m_random_init.frand_lt() * (m_init_y.second - m_init_y.first);
      } while(goal_reached());

      m_random_motion = Zeni::Random(m_random_init.rand());
    }

    void alter_impl() {
      /// Too extreme
//       m_horizontal_puddles.at(0)[1] = 0.3;
//       m_vertical_puddles.at(0)[2] = 0.6;
//       m_horizontal_puddles.push_back({{0.6, 0.75, 0.5, 0.1}});
//       m_goal_dynamic = true;
//       m_goal_x = make_pair(0.4, 0.5);
//       m_goal_y = make_pair(0.7, 0.8);

      if(get_scenario() < 100)
        return;

      else if(get_scenario() < 110) {
        m_goal_dynamic = true;
        m_goal_x = make_pair(0.8, 0.85);
        m_goal_y = make_pair(0.9, 1.0);
      }
      else if(get_scenario() < 120) {
        m_goal_dynamic = true;
        m_goal_x = make_pair(0.7, 0.75);
        m_goal_y = make_pair(0.9, 1.0);
      }
      else if(get_scenario() < 130) {
        m_goal_dynamic = true;
        m_goal_x = make_pair(0.5, 0.55);
        m_goal_y = make_pair(0.9, 1.0);
      }
      else if(get_scenario() < 200) {
        m_goal_dynamic = true;
        m_goal_x = make_pair(0.1, 0.15);
        m_goal_y = make_pair(0.9, 1.0);
      }

      else if(get_scenario() < 210) {
        m_horizontal_puddles.at(0)[3] = 0.15;
        m_vertical_puddles.at(0)[3] = 0.15;
      }
      else if(get_scenario() < 220) {
        m_horizontal_puddles.at(0)[3] = 0.2;
        m_vertical_puddles.at(0)[3] = 0.2;
      }
      else if(get_scenario() < 300) {
        m_horizontal_puddles.at(0)[3] = 0.4;
        m_vertical_puddles.at(0)[3] = 0.4;
      }

      else if(get_scenario() < 310) {
        m_noise = 0.02;
      }
      else if(get_scenario() < 320) {
        m_noise = 0.04;
      }
      else if(get_scenario() < 400) {
        m_noise = 0.08;
      }
    }

    reward_type transition_impl(const action_type &action) {
      const double shift = m_random_motion.frand_gaussian() * 0.01;
      const double step_size = shift + 0.05;

      switch(static_cast<const Move &>(action).direction) {
        case Move::NORTH: m_position.second += step_size; break;
        case Move::SOUTH: m_position.second -= step_size; break;
        case Move::EAST:  m_position.first  += step_size; break;
        case Move::WEST:  m_position.first  -= step_size; break;
        default: abort();
      }

      if(m_position.first < 0.0)
        m_position.first = 0.0;
      else if(m_position.first >= 1.0)
        m_position.first = 1.0 - DBL_EPSILON;
      if(m_position.second < 0.0)
        m_position.second = 0.0;
      else if(m_position.second >= 1.0)
        m_position.second = 1.0 - DBL_EPSILON;

      ++m_step_count;

      double reward = -1.0;

      for(const Puddle &puddle : m_horizontal_puddles)
        reward -= 400.0 * horizontal_puddle_reward(puddle[0], puddle[1], puddle[2], puddle[3]);
      for(const Puddle &puddle : m_vertical_puddles)
        reward -= 400.0 * vertical_puddle_reward(puddle[0], puddle[1], puddle[2], puddle[3]);

      return reward;
    }

    double horizontal_puddle_reward(const double &left, const double &right, const double &y, const double &radius) const {
      double dist;

      if(m_position.first < left)
        dist = pythagoras(m_position.first - left, m_position.second - y);
      else if(m_position.first < right)
        dist = std::abs(m_position.second - y);
      else
        dist = pythagoras(m_position.first - right, m_position.second - y);

      return std::max(0.0, radius - dist);
    }

    double vertical_puddle_reward(const double &x, const double &bottom, const double &top, const double &radius) const {
      double dist;

      if(m_position.second < bottom)
        dist = pythagoras(m_position.first - x, m_position.second - bottom);
      else if(m_position.second < top)
        dist = std::abs(m_position.first - x);
      else
        dist = pythagoras(m_position.first - x, m_position.second - top);

      return std::max(0.0, radius - dist);
    }

    template <typename TYPE>
    TYPE pythagoras(const TYPE &lhs, const TYPE &rhs) const {
      return sqrt(squared(lhs) + squared(rhs));
    }

    template <typename TYPE>
    TYPE squared(const TYPE &value) const {
      return value * value;
    }

    void print_impl(ostream &os) const {
      os << "Puddle World:" << endl;
      os << " (" << m_position.first << ", " << m_position.second << ')' << endl;
    }

    Zeni::Random m_random_init;
    Zeni::Random m_random_motion;

    double_pair m_position;

    double_pair m_init_x;
    double_pair m_init_y;

    bool m_goal_dynamic = false;
    double_pair m_goal_x;
    double_pair m_goal_y;

    size_t m_step_count = 0lu;
    const bool m_random_start = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["random-start"]).get_value();

    std::vector<Puddle> m_horizontal_puddles;
    std::vector<Puddle> m_vertical_puddles;

    double m_noise = 0.01;
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
      const auto position = env->get_position();

      for(size_t y = granularity; y != 0lu; --y) {
        for(size_t x = 0lu; x != granularity; ++x) {
          env->set_position(Environment::double_pair((x + 0.5) / granularity, (y - 0.5) / granularity));
          regenerate_lists();
          auto action = choose_greedy();
          switch(static_cast<const Move &>(*action).direction) {
            case Move::NORTH: os << 'N'; break;
            case Move::SOUTH: os << 'S'; break;
            case Move::EAST:  os << 'E'; break;
            case Move::WEST:  os << 'W'; break;
            default: abort();
          }
        }
        os << endl;
      }

      env->set_position(position);
      regenerate_lists();
    }

  private:
    set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &trie) const {
      return generate_vfgs_for_axes(trie, feature_type::Axis::X, feature_type::Axis::Y);
    }

    map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie) const {
      return generate_ucm_for_axes(trie, feature_type::Axis::X, feature_type::Axis::Y);
    }

    void generate_features() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      for(const action_type &action_ : *m_candidates) {
        auto &features = get_feature_list(action_);
        assert(!features);

        Feature::List * x_tail = &(new Feature(Feature::X, 0.0, 1.0, 0, env->get_position().first))->features;
        x_tail = x_tail->insert_in_order<feature_type::List::compare_default>(features, false);
        Feature::List * y_tail = &(new Feature(Feature::Y, 0.0, 1.0, 0, env->get_position().second))->features;
        y_tail = y_tail->insert_in_order<feature_type::List::compare_default>(features, false);

        feature_trie trie = get_trie(action_);

        for(;;) {
          if(generate_feature_ranged(env, features, trie, x_tail, env->get_position().first))
            continue;
          if(generate_feature_ranged(env, features, trie, y_tail, env->get_position().second))
            continue;

          break;
        }
      }
    }

    void generate_candidates() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_candidates);

      (new Move(Move::NORTH))->candidates.insert_before(m_candidates);
      (new Move(Move::SOUTH))->candidates.insert_before(m_candidates);
      (new Move(Move::EAST))->candidates.insert_before(m_candidates);
      (new Move(Move::WEST))->candidates.insert_before(m_candidates);
    }

    void update() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->goal_reached() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
    }
  };

}

#endif
