#ifndef BLOCKS_WORLD_H
#define BLOCKS_WORLD_H

#include "agent.h"
#include "environment.h"

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
    int compare(const On_Top &) const {
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
    int compare(const In_Place &) const {
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
        auto &os_ = os;
        std::for_each(stack.rbegin(), stack.rend(), [&os_](const block_id &id) {
          os_ << ' ' << id;
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
      init();
    }

  private:
    void generate_features() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_features);

      block_id place_counter = 3;

      auto not_in_place = [this,&place_counter]() {
        while(place_counter) {
          feature_type * out_place = new In_Place(place_counter, false);
          out_place->features.insert_in_order<feature_type::List::compare_default>(m_features);
          --place_counter;
        }
      };
      auto is_on_top = [this](const block_id &top, const block_id &bottom, const block_id &num_blocks) {
        for(block_id non_bottom = 1; non_bottom <= num_blocks; ++non_bottom) {
          if(top != non_bottom) {
            feature_type * on_top = new On_Top(top, non_bottom, bottom == non_bottom);
            on_top->features.insert_in_order<feature_type::List::compare_default>(m_features);
          }
        }
      };

      std::for_each(env->get_blocks().begin(), env->get_blocks().end(), [this,&not_in_place,&is_on_top,&place_counter](const Environment::Stack &stack) {
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
          const auto &this_ = this;
          auto &place_counter_ = place_counter;
          std::find_if(stack.rbegin(), stack.rend(), [this_,&place_counter_](const block_id &id)->bool{
            if(place_counter_ != id)
              return true;

            feature_type * in_place = new In_Place(id);
            in_place->features.insert_in_order<feature_type::List::compare_default>(this_->m_features);
            --place_counter_;

            return false;
          });

          not_in_place();
        }
      });

      not_in_place();
    }

    void generate_candidates() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      assert(!m_candidates);

      std::for_each(env->get_blocks().begin(), env->get_blocks().end(), [this,&env](const Environment::Stack &stack_src) {
        if(stack_src.size() != 1) {
          action_type * move = new Move(*stack_src.begin(), 0);
          move->candidates.insert_before(m_candidates);
        }

        const auto &this_ = this;
        std::for_each(env->get_blocks().begin(), env->get_blocks().end(), [this_,&stack_src](const Environment::Stack &stack_dest) {
          if(&stack_src == &stack_dest)
            return;

          action_type * move = new Move(*stack_src.begin(), *stack_dest.begin());
          move->candidates.insert_before(this_->m_candidates);
        });
      });
    }

    void update() {
      auto env = std::dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->get_blocks() == env->get_goal() ? SUCCESS : NON_TERMINAL;
    }
  };

}

#endif
