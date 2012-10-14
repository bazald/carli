#ifndef BLOCKS_WORLD_H
#define BLOCKS_WORLD_H

#include "agent.h"
#include "environment.h"
#include "random.h"

#include <algorithm>
#include <list>
#include <stdexcept>

#include <iostream>

namespace Blocks_World {
  
  typedef int block_id;

  struct On_Top;
  struct In_Place;
  struct On_Top;

  struct Feature;
  struct Feature : ::Feature<Feature, On_Top> {
    Feature(const bool &present_ = true)
     : ::Feature<Feature, On_Top>(present_)
    {
    }

    virtual int compare(const Feature &rhs) const = 0;
    virtual int compare(const In_Place &rhs) const = 0;
    virtual int compare(const On_Top &rhs) const = 0;
  };

  typedef Feature feature_type;

  struct In_Place : public feature_type {
    In_Place()
     : block(block_id())
    {
    }

    In_Place(const block_id &block_, const bool &present_ = true)
     : feature_type(present_),
     block(block_)
    {
    }

    In_Place * clone() const {
      return new In_Place(block);
    }

    void print_impl(std::ostream &os) const {
      os << "in-place(" << block << ')';
    }

    int compare(const Feature &rhs) const {
      return -rhs.compare(*this);
    }
    int compare(const In_Place &rhs) const {
      return present ^ rhs.present ? rhs.present - present : block - rhs.block;
    }
    int compare(const On_Top &rhs) const {
      return -1;
    }

    block_id block;
  };

  struct On_Top : public feature_type {
    On_Top()
     : top(block_id()),
     bottom(block_id())
    {
    }

    On_Top(const block_id &top_, const block_id &bottom_, const bool &present_ = true)
     : feature_type(present_),
     top(top_),
     bottom(bottom_)
    {
    }

    On_Top * clone() const {
      return new On_Top(top, bottom);
    }

    void print_impl(std::ostream &os) const {
      os << "on-top(" << top << ',' << bottom << ')';
    }

    int compare(const Feature &rhs) const {
      return -rhs.compare(*this);
    }
    int compare(const In_Place &rhs) const {
      return 1;
    }
    int compare(const On_Top &rhs) const {
      return present ^ rhs.present ? rhs.present - present : top != rhs.top ? top - rhs.top : bottom - rhs.bottom;
    }

    block_id top;
    block_id bottom;
  };

  struct Move;
  typedef Action<Move, Move> action_type;

  struct Move : public action_type {
    Move()
     : block(block_id()),
     dest(block_id())
    {
    }

    Move(const block_id &block_, const block_id &dest_)
     : block(block_),
     dest(dest_)
    {
    }

    Move * clone() const {
      return new Move(block, dest);
    }

    int compare(const Move &rhs) const {
      return block != rhs.block ? block - rhs.block : dest - rhs.dest;
    }

    void print_impl(std::ostream &os) const {
      os << "move(" << block << ',' << dest << ')';
    }

    block_id block;
    block_id dest;
  };

  class Environment : public ::Environment<action_type> {
  public:
    Environment() {
      Environment::init_impl();

      {
        m_goal.push_front(Stack());
        Stack &stack = *m_goal.begin();
        stack.push_front(3);
        stack.push_front(2);
        stack.push_front(1);
      }
    }

    typedef std::list<block_id> Stack;
    typedef std::list<Stack> Stacks;

    const Stacks & get_blocks() const {return m_blocks;}
    const Stacks & get_goal() const {return m_goal;}

  private:
    void init_impl() {
      m_blocks.clear();

      {
        m_blocks.push_front(Stack());
        Stack &stack = *m_blocks.begin();
        stack.push_front(2);
      }
      {
        m_blocks.push_front(Stack());
        Stack &stack = *m_blocks.begin();
        stack.push_front(1);
        stack.push_front(3);
      }
    }

    reward_type transition_impl(const action_type &action) {
      Stacks::iterator src = std::find_if(m_blocks.begin(), m_blocks.end(), [&action](Stack &stack)->bool {
        return !stack.empty() && *stack.begin() == static_cast<const Move &>(action).block;
      });
      Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&action](Stack &stack)->bool {
        return !stack.empty() && *stack.begin() == static_cast<const Move &>(action).dest;
      });

      if(src == m_blocks.end())
        throw std::runtime_error("Attempt to move Block from under another Block.");
      if(dest == m_blocks.end())
        dest = m_blocks.insert(m_blocks.begin(), Stack());

      src->pop_front();
      dest->push_front(static_cast<const Move &>(action).block);

      if(src->empty())
        m_blocks.erase(src);

      return reward_type(-1);
    }

    void print_impl(std::ostream &os) const {
      os << "Blocks World:" << std::endl;
      std::for_each(m_blocks.begin(), m_blocks.end(), [&os](const Stack &stack) {
        std::for_each(stack.rbegin(), stack.rend(), [&os](const block_id &id) {
          os << ' ' << id;
        });
        os << std::endl;
      });
    }

    Stacks m_blocks;
    Stacks m_goal;
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    Agent(const std::shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env)
    {
      init_impl();
    }
    
    ~Agent() {
    }

  private:
    void init_impl() {
      m_next = decide(false);
    }

    reward_type act_impl() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      m_current = std::move(m_next);

      Q_Value * const value_current = get_value(m_features, *m_current, Q_Value::current_offset());

      const reward_type reward = get_env()->transition(*m_current);

      m_metastate = env->get_blocks() == env->get_goal() ? SUCCESS : NON_TERMINAL;

      if(m_metastate == NON_TERMINAL) {
        m_next = decide(true);

        Q_Value * const value_best = get_value(m_features, *m_next, Q_Value::next_offset());

        td_update(&value_current->current, reward, &value_best->next, 1.0, 1.0);

        m_next = decide(false);
      }
      else
        td_update(&value_current->current, reward, nullptr, 1.0, 1.0);

      return reward;
    }

    std::unique_ptr<action_type> decide(const bool &greedy) {
      m_features->destroy();

      m_features = generate_features();
      print_list(std::cerr, " Features:\n ", " ", m_features);

      action_list candidates = generate_candidates();
      print_list(std::cerr, " Candidates:\n ", " ", candidates);

      auto action = greedy ? choose_greedily(m_features, candidates)
                           : choose_epsilon_greedily(m_features, candidates, 0.1);
      auto chosen = std::unique_ptr<action_type>(action->clone());

      candidates->destroy();

      return chosen;
    }

    const Blocks_World::Environment::action_type * choose_epsilon_greedily(const feature_list &features, const action_list &candidates, const double &epsilon) {
      if(random.frand_lt() < epsilon)
        return choose_randomly(candidates);
      else
        return choose_greedily(features, candidates);
    }

    const Blocks_World::Environment::action_type * choose_greedily(const feature_list &features, const action_list &candidates) {
      std::cerr << "  choose_greedily" << std::endl;

      double value = double();
      const Blocks_World::Environment::action_type * action = nullptr;
      std::for_each(candidates->begin(), candidates->end(), [this,features,&action,&value](const Blocks_World::Environment::action_type &action_) {
        if(!action) {
          action = &action_;
          value = sum_value(&action_, get_value(features, action_, Q_Value::next_offset())->next);
        }
        else {
          const double value_ = sum_value(&action_, get_value(features, action_, Q_Value::next_offset())->next);
          if(value_ > value) {
            action = &action_;
            value = value_;
          }
        }
      });

      return action;
    }

    const Blocks_World::Environment::action_type * choose_randomly(const action_list &candidates) {
      std::cerr << "  choose_randomly" << std::endl;

      int counter = 0;
      std::for_each(candidates->begin(), candidates->end(), [&counter](const Blocks_World::Environment::action_type &) {
        ++counter;
      });

      counter = random.rand_lt(counter) + 1;
      const Blocks_World::Environment::action_type * action = nullptr;
      std::for_each(candidates->begin(), candidates->end(), [&counter,&action](const Blocks_World::Environment::action_type &action_) {
        if(!--counter)
          action = &action_;
      });

      return action;
    }

    void td_update(Q_Value::List * const &current, const reward_type &reward, const Q_Value::List * const &next, const double &alpha, const double &gamma) {
      if(!current)
        return;

      const double approach = reward + (next ? gamma * sum_value(nullptr, *next) : 0.0);

      double count = double();
      double old = double();
      for_each(current->begin(), current->end(), [&count,&old](const Q_Value &value) {
        ++count;
        old += value;
      });

      const double delta = alpha * (approach - old) / count;
      for_each(current->begin(), current->end(), [&delta](Q_Value &value) {
        value += delta;
      });

      std::cerr << " td_update: " << old << " <" << alpha << "= " << reward << " + " << gamma << " * " << (next ? sum_value(nullptr, *next) : 0.0) << std::endl;
      std::cerr << "            " << delta << " = " << alpha << " * (" << approach << " - " << old << ") / " << count << std::endl;

      old = double();
      for_each(current->begin(), current->end(), [&old](const Q_Value &value) {
        old += value;
      });

      std:: cerr << "            " << old << std::endl;
    }

    double sum_value(const action_type * const &action, const Q_Value::List &value_list) {
      if(action)
        std::cerr << "   sum_value(" << *action << ") = {";

      double sum = double();
      for_each(value_list.begin(), value_list.end(), [&action,&sum](const Q_Value &value) {
        if(action)
          std::cerr << ' ' << value;

        sum += value;
      });

      if(action)
        std::cerr << " }" << std::endl;

      return sum;
    }

    template <typename LIST>
    void print_list(std::ostream &os, const std::string &head, const std::string &pre, const LIST &list) const {
      if(list) {
        os << head;
        std::for_each(list->begin(), list->end(), [&os,&pre](decltype(*list->begin()) &value) {
          os << pre << value;
        });
        os << std::endl;
      }
    }

    feature_list generate_features() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      feature_list features = nullptr;

      block_id place_counter = 3;

      auto not_in_place = [&features,&place_counter]() {
        while(place_counter) {
          feature_type * out_place = new In_Place(place_counter, false);
          out_place->features.insert_in_order(features);
          --place_counter;
        }
      };
      auto is_on_top = [&features](const block_id &top, const block_id &bottom, const block_id &num_blocks) {
        for(block_id non_bottom = 1; non_bottom <= num_blocks; ++non_bottom) {
          if(top != non_bottom) {
            feature_type * on_top = new On_Top(top, non_bottom, bottom == non_bottom);
            on_top->features.insert_in_order(features);
          }
        }
      };

      std::for_each(env->get_blocks().begin(), env->get_blocks().end(), [&features,&not_in_place,&is_on_top,&place_counter](const Environment::Stack &stack) {
        auto it = stack.begin();
        auto itn = ++stack.begin();
        auto iend = stack.end();

        while(itn != iend) {
          is_on_top(*it, *itn, 3);

          it = itn;
          ++itn;
        }

        is_on_top(*it, 0, 3);

        if(place_counter == *stack.rbegin()) {
          std::find_if(stack.rbegin(), stack.rend(), [&features,&place_counter](const block_id &id) {
            if(place_counter != id)
              return true;

            feature_type * in_place = new In_Place(id);
            in_place->features.insert_in_order(features);
            --place_counter;

            return false;
          });

          not_in_place();
        }
      });

      not_in_place();

      return features;
    }

    action_list generate_candidates() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      action_list candidates = nullptr;

      std::for_each(env->get_blocks().begin(), env->get_blocks().end(), [&candidates,&env](const Environment::Stack &stack_src) {
        if(stack_src.size() != 1) {
          action_type * move = new Move(*stack_src.begin(), 0);
          move->candidates.insert_before(candidates);
        }

        std::for_each(env->get_blocks().begin(), env->get_blocks().end(), [&candidates,&stack_src](const Environment::Stack &stack_dest) {
          if(&stack_src == &stack_dest)
            return;

          action_type * move = new Move(*stack_src.begin(), *stack_dest.begin());
          move->candidates.insert_before(candidates);
        });
      });

      return candidates;
    }

    Zeni::Random random;
  };

}

#endif
