#ifndef BLOCKS_WORLD_H
#define BLOCKS_WORLD_H

#include "environment.h"
#include "linked_list.h"

#include <algorithm>
#include <stdexcept>

namespace Blocks_World {
  
  typedef int block_id;

  struct Move;
  struct Move : public Zeni::Pool_Allocator<Move> {
    typedef Zeni::Linked_List<Move> List;
    typedef List::iterator iterator;

    Move()
     : block(block_id()),
     dest(block_id()),
     candidates(this)
    {
    }

    Move(const block_id &block_, const block_id &dest_)
     : block(block_),
     dest(dest_),
     candidates(this)
    {
    }

    block_id block;
    block_id dest;

    List candidates;
  };

  class Environment : public ::Environment<Move> {
  public:
    Environment() {
      Environment::init_impl();
      m_candidates = generate_candidates();
    }

    ~Environment() {
      destroy_candidates(m_candidates);
    }

  private:
    typedef std::list<block_id> Stack;
    typedef std::list<Stack> Stacks;

    void init_impl() {
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
        return !stack.empty() && *stack.begin() == action.block;
      });
      Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&action](Stack &stack)->bool {
        return !stack.empty() && *stack.begin() == action.dest;
      });

      if(src == m_blocks.end())
        throw std::runtime_error("Attempt to move Block from under another Block.");
      if(dest == m_blocks.end())
        dest = m_blocks.insert(m_blocks.begin(), Stack());

      src->pop_front();
      dest->push_front(action.block);

      if(src->empty())
        m_blocks.erase(src);

      if(m_blocks.size() == 1) {
        m_metastate = SUCCESS;
        block_id id = 0;
        std::for_each(m_blocks.begin()->begin(), m_blocks.begin()->end(), [this,&id](const block_id &id_) {
          if(++id != id_)
            m_metastate = NON_TERMINAL;
        });
      }
      else
        m_metastate = NON_TERMINAL;

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

      if(m_candidates) {
        std::for_each(m_candidates->candidates.begin(), m_candidates->candidates.end(), [&os](const Move &move) {
          os << " (" << move.block << ',' << move.dest << ')';
        });
        os << std::endl;
      }
    }

    candidate_type generate_candidates() {
      action_type::iterator candidates;
      std::for_each(m_blocks.begin(), m_blocks.end(), [&candidates,this](const Stack &stack_src) {
        if(stack_src.size() != 1) {
          action_type * move = new Move(*stack_src.begin(), 0);
          move->candidates.insert_before(candidates);
          candidates = &move->candidates;
        }

        std::for_each(m_blocks.begin(), m_blocks.end(), [&candidates,&stack_src](const Stack &stack_dest) {
          if(&stack_src == &stack_dest)
            return;

          action_type * move = new Move(*stack_src.begin(), *stack_dest.begin());
          move->candidates.insert_before(candidates);
          candidates = &move->candidates;
        });
      });

      return candidates.get();
    }

    void destroy_candidates(candidate_type candidates) {
      if(candidates)
        candidates->candidates.destroy();
    }

    Stacks m_blocks;
  };

}

#endif
