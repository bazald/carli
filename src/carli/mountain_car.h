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

  class Feature;
  class Feature : public ::Feature<Feature> {
    Feature & operator=(const Feature &) = delete;

  public:
    enum Axis : char {X, X_DOT};

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
        case X:     os << 'x';     break;
        case X_DOT: os << "x-dot"; break;
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

    void print_impl(std::ostream &os) const {
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
    typedef std::pair<double, double> double_pair;

    Environment() {
      Environment::init_impl();
    }

    const float & get_x() const {return m_x;}
    const float & get_x_dot() const {return m_x_dot;}
    const float & get_value(const Feature::Axis &index) const {return *(&m_x + index);}
    bool is_random_start() const {return m_random_start;}
    bool is_reward_negative() const {return m_reward_negative;}

    void set_x(const float &x_) {m_x = x_;}
    void set_x_dot(const float &x_dot_) {m_x_dot = x_dot_;}
    void set_random_start(const bool &random_start_) {m_random_start = random_start_;}
    void set_reward_negative(const bool &reward_negative_) {m_reward_negative = reward_negative_;}

    bool success() const {
      return MCarAtGoal();
    }

  private:
    void init_impl() {
      MCarInit();
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
      MCarStep(int(dynamic_cast<const Move &>(action).direction));

      return m_reward_negative ? -1 : success() ? 1 : 0;
    }

    void print_impl(std::ostream &os) const {
      os << "Mountain Car:" << std::endl;
      os << " (" << m_x << ", " << m_x_dot << ')' << std::endl;
    }

    void MCarInit();         ///< initialize car state
    void MCarStep(int a);    ///< update car state for given action
    bool MCarAtGoal() const; ///< is car at goal?

    Zeni::Random m_random_init;

    float m_x = 0.0f;
    float m_x_dot = 0.0f;

    double m_cart_force = 0.001;
    double m_grav_force = 0.0025;

    bool m_random_start = false;
    bool m_reward_negative = true;
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    typedef std::pair<double, double> point_type;
    typedef std::pair<point_type, point_type> line_segment_type;

    Agent(const std::shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env)
    {
      set_credit_assignment(Credit_Assignment::INV_LOG_UPDATE_COUNT);
      set_discount_rate(1.0);
      set_learning_rate(0.3);
      set_on_policy(false);
      set_epsilon(0.1);
      set_pseudoepisode_threshold(10);
      m_features_complete = false;
    }

    void print_value_function_grid(std::ostream &os) const {
      std::set<line_segment_type> line_segments;
      for(auto &value : m_value_function) {
        os << *value.first << ":" << std::endl;
        const auto line_segments2 = generate_value_function_grid_sets(value.second);
        merge_value_function_grid_sets(line_segments, line_segments2);
        print_value_function_grid_set(os, line_segments2);
      }
      os << "all:" << std::endl;
      print_value_function_grid_set(os, line_segments);
    }

    void print_update_count_grid(std::ostream &os) const {
      std::map<line_segment_type, size_t> update_counts;
      for(auto &value : m_value_function) {
        os << *value.first << ":" << std::endl;
        const auto update_counts2 = generate_update_count_maps(value.second);
        merge_update_count_maps(update_counts, update_counts2);
        print_update_count_map(os, update_counts2);
      }
      os << "all:" << std::endl;
      print_update_count_map(os, update_counts);
    }

    void print_policy(std::ostream &os, const size_t &granularity) {
      auto env = std::dynamic_pointer_cast<Environment>(get_env());
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
        os << std::endl;
      }

      env->set_x(x_bak);
      env->set_x_dot(x_dot_bak);
      regenerate_lists();
    }

  private:
    std::set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &trie, const line_segment_type &extents = line_segment_type(point_type(-1.2, 0.6), point_type(-0.07, 0.07))) const {
      std::set<line_segment_type> line_segments;
      if(trie) {
        std::for_each(trie->begin(trie), trie->end(trie), [this,&line_segments,&extents](const feature_trie_type &trie2) {
          auto new_extents = extents;
          const auto &key = trie2.get_key();
          if(key->axis == Feature::X) {
            new_extents.first.first = key->bound_lower;
            new_extents.second.first = key->bound_higher;
          }
          else {
            new_extents.first.second = key->bound_lower;
            new_extents.second.second = key->bound_higher;
          }

          if(new_extents.first.first != extents.first.first)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.first.second), std::make_pair(new_extents.first.first, new_extents.second.second)));
          if(new_extents.first.second != extents.first.second)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.first.second), std::make_pair(new_extents.second.first, new_extents.first.second)));
          if(new_extents.second.first != extents.second.first)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.second.first, new_extents.first.second), std::make_pair(new_extents.second.first, new_extents.second.second)));
          if(new_extents.second.second != extents.second.second)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.second.second), std::make_pair(new_extents.second.first, new_extents.second.second)));

          if(trie2.get_deeper()) {
            const auto line_segments2 = this->generate_value_function_grid_sets(trie2.get_deeper(), new_extents);
            this->merge_value_function_grid_sets(line_segments, line_segments2);
          }
        });
      }
      return line_segments;
    }

    std::map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &trie, const line_segment_type &extents = line_segment_type(point_type(-1.2, 0.6), point_type(-0.07, 0.07)), const size_t &update_count = 0) const {
      std::map<line_segment_type, size_t> update_counts;
      if(trie) {
        std::for_each(trie->begin(trie), trie->end(trie), [this,&update_counts,&extents,&update_count](const feature_trie_type &trie2) {
          auto new_extents = extents;
          const auto &key = trie2.get_key();
          if(key->axis == Feature::X) {
            new_extents.first.first = key->bound_lower;
            new_extents.second.first = key->bound_higher;
          }
          else {
            new_extents.first.second = key->bound_lower;
            new_extents.second.second = key->bound_higher;
          }

          const auto update_count2 = update_count + (trie2.get() ? trie2->update_count : 0);

          update_counts[new_extents] = update_count2;

          if(trie2.get_deeper()) {
            const auto update_counts2 = this->generate_update_count_maps(trie2.get_deeper(), new_extents, update_count2);
            this->merge_update_count_maps(update_counts, update_counts2);
          }
        });
      }
      return update_counts;
    }

    void print_value_function_grid_set(std::ostream &os, const std::set<line_segment_type> &line_segments) const {
      for(const line_segment_type &line_segment : line_segments)
        os << line_segment.first.first << ',' << line_segment.first.second << '/' << line_segment.second.first << ',' << line_segment.second.second << std::endl;
    }

    void print_update_count_map(std::ostream &os, const std::map<line_segment_type, size_t> &update_counts) const {
      for(const auto &rect : update_counts)
        os << rect.first.first.first << ',' << rect.first.first.second << '/' << rect.first.second.first << ',' << rect.first.second.second << '=' << rect.second << std::endl;
    }

    void merge_value_function_grid_sets(std::set<line_segment_type> &combination, const std::set<line_segment_type> &additions) const {
      for(const line_segment_type &line_segment : additions)
        combination.insert(line_segment);
    }

    void merge_update_count_maps(std::map<line_segment_type, size_t> &combination, const std::map<line_segment_type, size_t> &additions) const {
      for(const auto &rect : additions)
        combination[rect.first] += rect.second;
    }

    void generate_features() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

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

    bool generate_feature_ranged(const std::shared_ptr<const Environment> &env, feature_trie &trie, const Feature::List * const &tail, Feature::List * &tail_next) {
      auto match = std::find_if(trie->begin(trie), trie->end(trie), [&tail](const feature_trie_type &trie)->bool {return trie.get_key()->compare(**tail) == 0;});

      if(match && (!match->get() || match->get()->type != Q_Value::Type::FRINGE)) {
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
      (new Move(Move::IDLE))->candidates.insert_before(m_candidates);
      (new Move(Move::RIGHT))->candidates.insert_before(m_candidates);
    }

    void update() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->success() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
    }
  };

}

#endif
