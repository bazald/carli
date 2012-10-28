#ifndef PUDDLE_WORLD_H
#define PUDDLE_WORLD_H

#include "agent.h"
#include "environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>
#include <array>

namespace Puddle_World {
  
  typedef int block_id;

  class Feature;
  class Feature : public ::Feature<Feature> {
    Feature & operator=(const Feature &);

  public:
    enum Axis {X, Y};

    Feature()
     : ::Feature<Feature>(true)
    {
    }

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_)
     : ::Feature<Feature>(true),
     axis(axis_),
     bound_lower(bound_lower_),
     bound_higher(bound_higher_),
     depth(depth_)
    {
    }

    int compare_pi(const Feature &rhs) const {
      return depth != rhs.depth ? (depth < rhs.depth ? -1 : 1) :
             axis != rhs.axis ? axis - rhs.axis :
             bound_lower != rhs.bound_lower ? (bound_lower < rhs.bound_lower ? -1 : 1) :
             (bound_higher == rhs.bound_higher ? 0 : bound_higher < rhs.bound_higher ? -1 : 1);
    }

    double midpt() const {
      return (bound_lower + bound_higher) / 2.0;
    }

    Feature * clone() const {
      return new Feature(axis, bound_lower, bound_higher, depth);
    }

    void print_impl(std::ostream &os) const {
      os << (axis == X ? 'x' : 'y') << '(' << bound_lower << ',' << bound_higher << ':' << depth << ')';
    }

    Axis axis;
    double bound_lower; ///< inclusive
    double bound_higher; ///< exclusive

    size_t depth; ///< 0 indicates unsplit
  };

  typedef Feature feature_type;

  class Move;
  typedef Action<Move, Move> action_type;

  class Move : public action_type {
  public:
    enum Direction {NORTH, SOUTH, EAST, WEST};

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

    void print_impl(std::ostream &os) const {
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
  public:
    typedef std::pair<double, double> double_pair;

    Environment()
     : m_init_x(0.0, 1.0),
     m_init_y(0.0, 1.0),
     m_goal_x(0.95, 1.0),
     m_goal_y(0.95, 1.0),
     m_terminal_reward(0.0)
    {
      Environment::init_impl();
    }

    typedef std::list<block_id> Stack;
    typedef std::list<Stack> Stacks;

    const double_pair & get_position() const {return m_position;}
    const double_pair & get_goal_x() const {return m_goal_x;}
    const double_pair & get_goal_y() const {return m_goal_y;}

    bool goal_reached() const {
      return m_goal_x.first <= m_position.first  && m_position.first  < m_goal_x.second &&
             m_goal_y.first <= m_position.second && m_position.second < m_goal_y.second;
    }

  private:
    void init_impl() {
      do {
        m_position.first = m_init_x.first + m_random_init.frand_lt() * (m_init_x.second - m_init_x.first);
        m_position.first = m_init_y.first + m_random_init.frand_lt() * (m_init_y.second - m_init_y.first);
      } while(goal_reached());

      m_random_motion = Zeni::Random(m_random_init.rand());
    }

    reward_type transition_impl(const action_type &action) {
      const double shift = (m_random_motion.frand_lt() - 0.5) * 0.02; ///< Should really be Gaussian, stddev = 0.01

      switch(dynamic_cast<const Move &>(action).direction) {
        case Move::NORTH: m_position.second += 0.05 + shift; break;
        case Move::SOUTH: m_position.second -= 0.05 + shift; break;
        case Move::EAST:  m_position.first  += 0.05 + shift; break;
        case Move::WEST:  m_position.first  -= 0.05 + shift; break;
        default: abort();
      }

      if(m_position.first < 0.0)
        m_position.first = 0.0;
      else if(m_position.first > 1.0)
        m_position.first = 1.0;
      if(m_position.second < 0.0)
        m_position.second = 0.0;
      else if(m_position.second > 1.0)
        m_position.second = 1.0;

      double reward = -1.0;

      reward -= 400.0 * horizontal_puddle_reward(0.1, 0.45, 0.75, 0.1);
      reward -= 400.0 * vertical_puddle_reward(0.45, 0.4, 0.8, 0.1);

      return reward;
    }

    double horizontal_puddle_reward(const double &left, const double &right, const double &y, const double &radius) {
      double dist;

      if(m_position.first < left)
        dist = pythagoras(m_position.first - left, m_position.second - y);
      else if(m_position.first < right)
        dist = std::abs(m_position.second - y);
      else
        dist = pythagoras(m_position.first - right, m_position.second - y);

      return std::max(0.0, radius - dist);
    }

    double vertical_puddle_reward(const double &x, const double &bottom, const double &top, const double &radius) {
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
    TYPE pythagoras(const TYPE &lhs, const TYPE &rhs) {
      return sqrt(squared(lhs) + squared(rhs));
    }

    template <typename TYPE>
    TYPE squared(const TYPE &value) {
      return value * value;
    }

    void print_impl(std::ostream &os) const {
      os << "Puddle World:" << std::endl;
      os << " (" << m_position.first << ", " << m_position.second << ')' << std::endl;
    }

    Zeni::Random m_random_init;
    Zeni::Random m_random_motion;

    double_pair m_position;

    double_pair m_init_x;
    double_pair m_init_y;

    double_pair m_goal_x;
    double_pair m_goal_y;
    double m_terminal_reward;
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    Agent(const std::shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env)
    {
      set_learning_rate(0.3);
      set_discount_rate(1.0);
      set_on_policy(false);
      set_epsilon(0.1);
      set_pseudoepisode_threshold(10);
      m_features_complete = false;

      m_credit_assignment = [this](Q_Value::List * const &value_list){return this->assign_credit_inv_log_update_count(value_list);};

//       m_split_test = [](Q_Value * const &, const size_t &depth)->bool{return depth < Binary_Log<16>::value * 2 + 1;};

      m_split_test = [this](Q_Value * const &q, const size_t &depth)->bool{
        if(depth < Binary_Log<8>::value * 2 + 1)
          return true;

        if(!q)
          return false;
        if(q->split)
          return true;

        q->split |= q->pseudoepisode_count > 0 &&
                    this->get_mean_cabe().outlier_above(q->cabe, -0.5);

        return q->split;
      };

      init();
    }

  private:
    void generate_features() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_features);

      Feature::List * x_tail = &(new Feature(Feature::X, 0.0, 1.0, 0))->features;
      x_tail = x_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);
      Feature::List * y_tail = &(new Feature(Feature::Y, 0.0, 1.0, 0))->features;
      y_tail = y_tail->insert_in_order<feature_type::List::compare_default>(m_features, false);

      std::array<feature_trie, 4> tries = {{get_trie(Move(Move::NORTH)),
                                            get_trie(Move(Move::SOUTH)),
                                            get_trie(Move(Move::EAST)),
                                            get_trie(Move(Move::WEST))}};

      for(;;) {
        Feature::List * x_tail_next = nullptr;
        Feature::List * y_tail_next = nullptr;

        std::for_each(tries.begin(), tries.end(), [this,&env,&x_tail,&y_tail,&x_tail_next,&y_tail_next](feature_trie &trie) {
          auto &x_tail_ = x_tail;
          auto &y_tail_ = y_tail;

          auto match = std::find_if(trie->begin(trie), trie->end(trie), [&x_tail_](const feature_trie_type &trie)->bool {return trie.get_key()->compare(**x_tail_) == 0;});
          if(match != nullptr) {
            auto feature = match->get_key();
            const auto midpt = feature->midpt();
            if(env->get_position().first < midpt)
              x_tail_next = &(new Feature(Feature::X, feature->bound_lower, midpt, feature->depth + 1))->features;
            else
              x_tail_next = &(new Feature(Feature::X, midpt, feature->bound_higher, feature->depth + 1))->features;
            x_tail_next = x_tail_next->insert_in_order<feature_type::List::compare_default>(m_features, false);
            trie = match->get_deeper();
            return;
          }

          match = std::find_if(trie->begin(trie), trie->end(trie), [&y_tail_](const feature_trie_type &trie)->bool {return trie.get_key()->compare(**y_tail_) == 0;});
          if(match != nullptr) {
            auto feature = match->get_key();
            const auto midpt = feature->midpt();
            if(env->get_position().second < midpt)
              y_tail_next = &(new Feature(Feature::Y, feature->bound_lower, midpt, feature->depth + 1))->features;
            else
              y_tail_next = &(new Feature(Feature::Y, midpt, feature->bound_higher, feature->depth + 1))->features;
            y_tail_next = y_tail_next->insert_in_order<feature_type::List::compare_default>(m_features, false);
            trie = match->get_deeper();
          }
        });

        if(x_tail_next)
          x_tail = x_tail_next;
        if(y_tail_next)
          y_tail = y_tail_next;

        if(!x_tail_next && !y_tail_next)
          break;
      }
    }

    void generate_candidates() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_candidates);

      (new Move(Move::NORTH))->candidates.insert_before(m_candidates);
      (new Move(Move::SOUTH))->candidates.insert_before(m_candidates);
      (new Move(Move::EAST))->candidates.insert_before(m_candidates);
      (new Move(Move::WEST))->candidates.insert_before(m_candidates);
    }

    void update() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->goal_reached() ? SUCCESS : NON_TERMINAL;
    }
  };

}

#endif
