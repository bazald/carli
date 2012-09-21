#ifndef BLOCKS_WORLD_H
#define BLOCKS_WORLD_H

#include "environment.h"
#include "linked_list.h"

namespace Blocks_World {

  class Block;
  class Block : public Zeni::Pool_Allocator<Block> {
    Block(const Block &);
    Block & operator=(const Block &);

  public:
    typedef Zeni::Linked_List<Block> List;
    typedef List::iterator iterator;

    template <typename OPERATION>
    OPERATION for_each_stack(OPERATION op) const {
      Zeni::for_each(stacks.begin(), stacks.end(), [&op](const Block * const &ptr) {
        op(ptr);
      });

      return op;
    }

    template <typename OPERATION>
    OPERATION for_each_stack(OPERATION op) {
      Zeni::for_each(stacks.begin(), stacks.end(), [&op](Block * const &ptr) {
        op(ptr);
      });

      return op;
    }

    template <typename OPERATION>
    OPERATION for_each_block(OPERATION op) const {
      Zeni::for_each(blocks.begin(), blocks.end(), [&op](const Block * const &ptr) {
        op(ptr);
      });

      return op;
    }

    template <typename OPERATION>
    OPERATION for_each_block(OPERATION op) {
      Zeni::for_each(blocks.begin(), blocks.end(), [&op](Block * const &ptr) {
        op(ptr);
      });

      return op;
    }

    template <typename OPERATION>
    OPERATION for_each(OPERATION op) const {
      for_each_stack([&op](const Block * const &stack) {
        stack->for_each_block(op);
      });

      return op;
    }

    template <typename OPERATION>
    OPERATION for_each(OPERATION op) {
      for_each_stack([&op](Block * const &stack) {
        stack->for_each_block(op);
      });

      return op;
    }

    Block(const int &id_ = 0)
    : id(id_),
    stacks(this),
    blocks(this)
    {
    }

    int id;
    List stacks;
    List blocks;
  };

  class Blocks {
    Blocks(const Blocks &);
    Blocks operator=(const Blocks &);

  public:
    Blocks(Block * const &head_ = nullptr)
     : head(head_)
    {
    }

    ~Blocks() {
      if(head) {
        head->for_each([](Block * const &ptr) {
          delete ptr;
        });
      }
    }

    Blocks(Blocks &&rhs) {
      head = rhs.head;
      rhs.head = 0;
    }

    Blocks & operator=(Blocks &&rhs) {
      head = rhs.head;
      rhs.head = 0;
      return *this;
    }

    const Block * get() const {
      return head;
    }
    Block * get() {
      return head;
    }
    const Block & operator*() const {
      return *head;
    }
    Block & operator*() {
      return *head;
    }
    const Block * operator->() const {
      return head;
    }
    Block * operator->() {
      return head;
    }

  private:
    Block * head;
  };

  struct Move {
    Block * block;
    Block * from;
    Block * to;
  };

}

template <>
class Environment_Functions<Blocks_World::Blocks, Blocks_World::Move> : public Environment_Types<Blocks_World::Blocks, Blocks_World::Move> {
public:
  state_type initial_state() const {
    auto a = new Blocks_World::Block(0);
    auto * const b = new Blocks_World::Block(1);
    auto * const c = new Blocks_World::Block(2);

    c->blocks.insert_after(&a->blocks);
    b->stacks.insert_after(&a->stacks);

    return a;
  }

  result_type transition(const state_type &state, const action_type &action) const {
    return result_type();
  }

  meta_state_type meta_state(const state_type &state) const {
    return NON_TERMINAL;
  }
};

namespace Blocks_World {

  typedef ::Environment<Blocks_World::Blocks, Blocks_World::Move> Environment;

}

#endif
