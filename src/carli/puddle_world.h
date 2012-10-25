#ifndef PUDDLE_WORLD_H
#define PUDDLE_WORLD_H

#include "agent.h"
#include "environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>

namespace Puddle_World {
  
  typedef int block_id;

  class Feature;
  class Feature : public ::Feature<Feature> {
    Feature & operator=(const Feature &);

  public:
    enum Axis {X, Y};

    Feature(const bool &present_ = true)
     : ::Feature<Feature>(present_)
    {
    }

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_, const bool &present_ = true)
     : ::Feature<Feature>(present_),
     axis(axis_),
     bound_lower(bound_lower_),
     bound_higher(bound_higher_),
     depth(depth_)
    {
    }

    int compare_pi(const Feature &rhs) const {
      return depth != rhs.depth ? depth - rhs.depth :
             axis != rhs.axis ? axis - rhs.axis :
             bound_lower != rhs.bound_lower ? bound_lower - rhs.bound_lower :
             bound_higher - rhs.bound_higher;
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
      const float shift = (m_random_motion.frand_lt() - 0.5) * 0.02; ///< Should really be Gaussian, stddev = 0.01f

      switch(dynamic_cast<const Move &>(action).direction) {
        case Move::NORTH: m_position.second += 0.05f + shift; break;
        case Move::SOUTH: m_position.second -= 0.05f + shift; break;
        case Move::EAST:  m_position.first  += 0.05f + shift; break;
        case Move::WEST:  m_position.first  -= 0.05f + shift; break;
        default: abort();
      }

      if(m_position.first < 0.0f)
        m_position.first = 0.0f;
      else if(m_position.first > 1.0f)
        m_position.first = 1.0f;
      if(m_position.second < 0.0f)
        m_position.second = 0.0f;
      else if(m_position.second > 1.0f)
        m_position.second = 1.0f;

      return -1.0 - 400.0 * (horizontal_puddle_reward(0.1, 0.45, 0.25, 0.1) +
                             vertical_puddle_reward(0.45, 0.2, 0.6, 0.1));
    }

    double horizontal_puddle_reward(const double &left, const double &right, const double &y, const double &radius) {
      double dist;

      if(m_position.first < left)
        dist = pythagoras(m_position.first - left, m_position.second - y);
      else if(m_position.first < right)
        dist = abs(m_position.second - y);
      else
        dist = pythagoras(m_position.first - right, m_position.second - y);

      return std::max(0.0, radius - dist);
    }

    double vertical_puddle_reward(const double &x, const double &bottom, const double &top, const double &radius) {
      double dist;

      if(m_position.second < bottom)
        dist = pythagoras(m_position.first - x, m_position.second - bottom);
      else if(m_position.second < top)
        dist = abs(m_position.first - x);
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
      set_learning_rate(0.2);
      set_discount_rate(1.0);
      set_on_policy(false);
      set_epsilon(0.1);
      set_pseudoepisode_threshold(20);

      m_split_test = [](Q_Value * const &q, const size_t &depth)->bool{
        if(!q)
          return false;
        if(q->split)
          return true;

//         q->split |= q->update_count > 5;
        q->split |= depth < 4;

        return q->split;
      };

      init();
    }

  private:
    void generate_features() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_features);

      (new Feature(Feature::X, 0.0, 1.0, 0))->features.insert_in_order<feature_type::List::compare_default>(m_features);
      (new Feature(Feature::Y, 0.0, 1.0, 0))->features.insert_in_order<feature_type::List::compare_default>(m_features);
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
