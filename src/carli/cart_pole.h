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

  class Feature;
  class Feature : public ::Feature<Feature> {
    Feature & operator=(const Feature &);

  public:
    enum Axis {X, X_DOT, THETA, THETA_DOT};

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

    bool precedes(const Feature &rhs) const {
      const auto mdpt = midpt();
      return axis == rhs.axis && (rhs.bound_lower == mdpt || rhs.bound_higher == mdpt);
    }

    double midpt() const {
      return (bound_lower + bound_higher) / 2.0;
    }

    Feature * clone() const {
      return new Feature(axis, bound_lower, bound_higher, depth);
    }

    void print_impl(std::ostream &os) const {
      switch(axis) {
        case X:         os << 'x';         break;
        case X_DOT:     os << "x-dot";     break;
        case THETA:     os << "theta";     break;
        case THETA_DOT: os << "theta-dot"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_higher << ':' << depth << ')';
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
    enum Direction {LEFT, RIGHT};

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

    void print_impl(std::ostream &os) const {
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
    typedef std::pair<double, double> double_pair;

    Environment() {
      Environment::init_impl();
    }

    const float & get_x() const {return m_x;}
    const float & get_x_dot() const {return m_x_dot;}
    const float & get_theta() const {return m_theta;}
    const float & get_theta_dot() const {return m_theta_dot;}
    const float & get_value(const Feature::Axis &index) const {return *(&m_x + index);}
    bool is_ignoring_x() const {return m_ignore_x;}

    void set_x(const float &x_) {m_x = x_;}
    void set_x_dot(const float &x_dot_) {m_x = x_dot_;}
    void set_theta_dot(const float &theta_dot_) {m_theta = theta_dot_;}
    void set_theta(const float &theta_) {m_theta = theta_;}
    void set_ignore_x(const bool &ignore_x_) {m_ignore_x = ignore_x_;}

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

    void print_impl(std::ostream &os) const {
      os << "Cart Pole:" << std::endl;
      os << " (" << m_x << ", " << m_x_dot << ", " << m_theta << ", " << m_theta_dot << ')' << std::endl;
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
    bool m_ignore_x = false;
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    typedef std::pair<double, double> point_type;
    typedef std::pair<point_type, point_type> line_segment_type;

    Agent(const std::shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env),
     m_ignore_x(false)
    {
      set_credit_assignment(INV_LOG_UPDATE_COUNT);
      set_discount_rate(1.0);
      set_learning_rate(0.3);
      set_on_policy(false);
      set_epsilon(0.1);
      set_pseudoepisode_threshold(10);
      m_features_complete = false;
    }

    bool is_ignoring_x() const {return m_ignore_x;}
    void set_ignore_x(const bool &ignore_x_) {m_ignore_x = ignore_x_;}

    void print_value_function_grid(std::ostream &os) const {
//       std::set<line_segment_type> line_segments;
//       std::for_each(m_value_function.begin(), m_value_function.end(), [this,&os,&line_segments](decltype(*m_value_function.begin()) &value) {
//         os << *value.first << ":" << std::endl;
//         const auto line_segments2 = this->generate_value_function_grid_sets(value.second);
//         this->merge_value_function_grid_sets(line_segments, line_segments2);
//         this->print_value_function_grid_set(os, line_segments2);
//       });
//       os << "all:" << std::endl;
//       print_value_function_grid_set(os, line_segments);
    }

    void print_update_count_grid(std::ostream &os) const {
//       std::map<line_segment_type, size_t> update_counts;
//       std::for_each(m_value_function.begin(), m_value_function.end(), [this,&os,&update_counts](decltype(*m_value_function.begin()) &value) {
//         os << *value.first << ":" << std::endl;
//         const auto update_counts2 = this->generate_update_count_maps(value.second);
//         this->merge_update_count_maps(update_counts, update_counts2);
//         this->print_update_count_map(os, update_counts2);
//       });
//       os << "all:" << std::endl;
//       print_update_count_map(os, update_counts);
    }

    void print_policy(std::ostream &os, const size_t &granularity) {
//       auto env = std::dynamic_pointer_cast<Environment>(get_env());
//       const auto position = env->get_position();
// 
//       for(size_t y = granularity; y != 0lu; --y) {
//         for(size_t x = 0lu; x != granularity; ++x) {
//           env->set_position(Environment::double_pair((x + 0.5) / granularity, (y - 0.5) / granularity));
//           regenerate_lists();
//           auto action = choose_greedy();
//           switch(dynamic_cast<const Move &>(*action).direction) {
//             case Move::NORTH: os << 'N'; break;
//             case Move::SOUTH: os << 'S'; break;
//             case Move::EAST:  os << 'E'; break;
//             case Move::WEST:  os << 'W'; break;
//             default: abort();
//           }
//         }
//         os << std::endl;
//       }
// 
//       env->set_position(position);
//       regenerate_lists();
    }

  private:
//     std::set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &trie, const line_segment_type &extents = line_segment_type(point_type(), point_type(1.0, 1.0))) const {
//       std::set<line_segment_type> line_segments;
//       if(trie) {
//         std::for_each(trie->begin(trie), trie->end(trie), [this,&line_segments,&extents](const feature_trie_type &trie2) {
//           auto new_extents = extents;
//           const auto &key = trie2.get_key();
//           if(key->axis == Feature::X) {
//             new_extents.first.first = key->bound_lower;
//             new_extents.second.first = key->bound_higher;
//           }
//           else {
//             new_extents.first.second = key->bound_lower;
//             new_extents.second.second = key->bound_higher;
//           }
// 
//           if(new_extents.first.first != extents.first.first)
//             line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.first.second), std::make_pair(new_extents.first.first, new_extents.second.second)));
//           if(new_extents.first.second != extents.first.second)
//             line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.first.second), std::make_pair(new_extents.second.first, new_extents.first.second)));
//           if(new_extents.second.first != extents.second.first)
//             line_segments.insert(std::make_pair(std::make_pair(new_extents.second.first, new_extents.first.second), std::make_pair(new_extents.second.first, new_extents.second.second)));
//           if(new_extents.second.second != extents.second.second)
//             line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.second.second), std::make_pair(new_extents.second.first, new_extents.second.second)));
// 
//           if(trie2.get_deeper()) {
//             const auto line_segments2 = this->generate_value_function_grid_sets(trie2.get_deeper(), new_extents);
//             this->merge_value_function_grid_sets(line_segments, line_segments2);
//           }
//         });
//       }
//       return line_segments;
//     }
// 
//     std::map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie, const line_segment_type &extents = line_segment_type(point_type(), point_type(1.0, 1.0)), const size_t &update_count = 0) const {
//       std::map<line_segment_type, size_t> update_counts;
//       if(trie) {
//         std::for_each(trie->begin(trie), trie->end(trie), [this,&update_counts,&extents,&update_count](const feature_trie_type &trie2) {
//           auto new_extents = extents;
//           const auto &key = trie2.get_key();
//           if(key->axis == Feature::X) {
//             new_extents.first.first = key->bound_lower;
//             new_extents.second.first = key->bound_higher;
//           }
//           else {
//             new_extents.first.second = key->bound_lower;
//             new_extents.second.second = key->bound_higher;
//           }
// 
//           const auto update_count2 = update_count + (trie2.get() ? trie2->update_count : 0);
// 
//           update_counts[new_extents] = update_count2;
// 
//           if(trie2.get_deeper()) {
//             const auto update_counts2 = this->generate_update_count_maps(trie2.get_deeper(), new_extents, update_count2);
//             this->merge_update_count_maps(update_counts, update_counts2);
//           }
//         });
//       }
//       return update_counts;
//     }
// 
//     void print_value_function_grid_set(std::ostream &os, const std::set<line_segment_type> &line_segments) const {
//       std::for_each(line_segments.begin(), line_segments.end(), [&os](const line_segment_type &line_segment) {
//         os << line_segment.first.first << ',' << line_segment.first.second << '-' << line_segment.second.first << ',' << line_segment.second.second << std::endl;
//       });
//     }
// 
//     void print_update_count_map(std::ostream &os, const std::map<line_segment_type, size_t> &update_counts) const {
//       std::for_each(update_counts.begin(), update_counts.end(), [&os](const std::pair<line_segment_type, size_t> &rect) {
//         os << rect.first.first.first << ',' << rect.first.first.second << '-' << rect.first.second.first << ',' << rect.first.second.second << '=' << rect.second << std::endl;
//       });
//     }
// 
//     void merge_value_function_grid_sets(std::set<line_segment_type> &combination, const std::set<line_segment_type> &additions) const {
//       std::for_each(additions.begin(), additions.end(), [&combination](const line_segment_type &line_segment) {
//         combination.insert(line_segment);
//       });
//     }
// 
//     void merge_update_count_maps(std::map<line_segment_type, size_t> &combination, const std::map<line_segment_type, size_t> &additions) const {
//       std::for_each(additions.begin(), additions.end(), [&combination](const std::pair<line_segment_type, size_t> &rect) {
//         combination[rect.first] += rect.second;
//       });
//     }

    void generate_features() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

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

    bool generate_feature_ranged(const std::shared_ptr<const Environment> &env, feature_trie &trie, const Feature::List * const &tail, Feature::List * &tail_next) {
      auto match = std::find_if(trie->begin(trie), trie->end(trie), [&tail](const feature_trie_type &trie)->bool {return trie.get_key()->compare(**tail) == 0;});

      if(match && match->get() && match->get()->type != Q_Value::FRINGE) {
        auto feature = match->get_key();
        const auto midpt = feature->midpt();
        if(env->get_value(feature->axis) < midpt)
          tail_next = &(new Feature(feature->axis, feature->bound_lower, midpt, feature->depth + 1))->features;
        else
          tail_next = &(new Feature(feature->axis, midpt, feature->bound_higher, feature->depth + 1))->features;
        tail_next = tail_next->insert_in_order<feature_type::List::compare_default>(m_features, false);
        trie = match->get_deeper();
        return true;
      }

      return false;
    }

    void generate_candidates() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_candidates);

      (new Move(Move::LEFT))->candidates.insert_before(m_candidates);
      (new Move(Move::RIGHT))->candidates.insert_before(m_candidates);
    }

    void update() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      if(env->failed())
        m_metastate = FAILURE;
      else if(get_step_count() > 9999)
        m_metastate = SUCCESS;
      else
        m_metastate = NON_TERMINAL;
    }

    bool m_ignore_x;
  };

}

#endif
