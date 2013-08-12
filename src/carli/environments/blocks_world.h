#ifndef BLOCKS_WORLD_H
#define BLOCKS_WORLD_H

#include "../agent.h"
#include "../environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>

namespace Blocks_World {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  typedef int block_id;

  class In_Place;
  class On_Top;

  class Feature;
  class Feature : public Feature_Present<Feature, On_Top> {
  public:
    Feature(const bool &present_)
     : Feature_Present<Feature, On_Top>(present_)
    {
    }

    virtual Feature * clone() const = 0;

    virtual int compare_axis(const Feature &rhs) const = 0;
    virtual int compare_axis(const In_Place &rhs) const = 0;
    virtual int compare_axis(const On_Top &rhs) const = 0;
  };

  typedef Feature feature_type;

  class In_Place : public feature_type {
  public:
    In_Place(const block_id &block_, const bool &present_)
     : feature_type(present_),
     block(block_)
    {
    }

    In_Place * clone() const {
      return new In_Place(block, this->present);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const In_Place &rhs) const {
      return block - rhs.block;
    }
    int compare_axis(const On_Top &) const {
      return -1;
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "in-place(" << block << ')';
    }

    Rete::Symbol_Ptr_C symbol_constant() const {
      abort();
    }

    block_id block;
  };

  class On_Top : public feature_type {
  public:
    On_Top(const block_id &top_, const block_id &bottom_, const bool &present_)
     : feature_type(present_),
     top(top_),
     bottom(bottom_)
    {
    }

    On_Top * clone() const {
      return new On_Top(top, bottom, present);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const In_Place &) const {
      return 1;
    }
    int compare_axis(const On_Top &rhs) const {
      return top != rhs.top ? top - rhs.top : bottom - rhs.bottom;
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "on-top(" << top << ',' << bottom << ')';
    }

    Rete::Symbol_Ptr_C symbol_constant() const {
      abort();
    }

    block_id top;
    block_id bottom;
  };

  class Move;
  typedef Action<Move, Move> action_type;

  class Move : public action_type {
  public:
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

    void print_impl(ostream &os) const {
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

      return -1.0;
    }

    void print_impl(ostream &os) const {
      os << "Blocks World:" << endl;
      for(const Stack &stack : m_blocks) {
        for(const block_id &id : stack)
          os << ' ' << id;
        os << endl;
      }
    }

    Stacks m_blocks;
    Stacks m_goal;
  };

  class Agent : public ::Agent<feature_type, action_type> {
  public:
    Agent(const std::shared_ptr<environment_type> &env)
     : ::Agent<feature_type, action_type>(env)
    {
//      m_features_complete = true;
    }

  private:
    void generate_features() {
//      auto env = dynamic_pointer_cast<const Environment>(get_env());
//
//      for(const action_type &action_ : *m_candidates) {
//        auto &features = get_feature_list(action_);
//        assert(!features);
//
//        block_id place_counter = 3;
//
//        auto not_in_place = [&features,&place_counter]() {
//          while(place_counter) {
//            feature_type * out_place = new In_Place(place_counter, false);
//            out_place->features.insert_in_order<feature_type::List::compare_default>(features);
//            --place_counter;
//          }
//        };
//        auto is_on_top = [&features](const block_id &top, const block_id &bottom, const block_id &num_blocks) {
//          for(block_id non_bottom = 1; non_bottom <= num_blocks; ++non_bottom) {
//            if(top != non_bottom) {
//              feature_type * on_top = new On_Top(top, non_bottom, bottom == non_bottom);
//              on_top->features.insert_in_order<feature_type::List::compare_default>(features);
//            }
//          }
//        };
//
//        for(const Environment::Stack &stack : env->get_blocks()) {
//          auto it = stack.begin();
//          auto itn = ++stack.begin();
//          auto iend = stack.end();
//
//          while(itn != iend) {
//            is_on_top(*it, *itn, 3);
//
//            it = itn;
//            ++itn;
//          }
//
//          is_on_top(*it, 0, 3);
//
//          if(place_counter == *stack.rbegin()) {
//            std::find_if(stack.rbegin(), stack.rend(), [&features,&place_counter](const block_id &id)->bool{
//              if(place_counter != id)
//                return true;
//
//              feature_type * in_place = new In_Place(id, true);
//              in_place->features.insert_in_order<feature_type::List::compare_default>(features);
//              --place_counter;
//
//              return false;
//            });
//
//            not_in_place();
//          }
//        }
//
//        not_in_place();
//      }
    }

//    void generate_candidates() {
//      auto env = dynamic_pointer_cast<const Environment>(get_env());
//
//      assert(!m_candidates);
//
//      for(const Environment::Stack &stack_src : env->get_blocks()) {
//        if(stack_src.size() != 1) {
//          action_type * move = new Move(*stack_src.begin(), 0);
//          move->candidates.insert_before(m_candidates);
//        }
//
//        for(const Environment::Stack &stack_dest : env->get_blocks()) {
//          if(&stack_src == &stack_dest)
//            continue;
//
//          action_type * move = new Move(*stack_src.begin(), *stack_dest.begin());
//          move->candidates.insert_before(m_candidates);
//        }
//      }
//    }

    void update() {
      auto env = dynamic_pointer_cast<const Environment>(get_env());

      m_metastate = env->get_blocks() == env->get_goal() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
    }

    Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    Rete::Symbol_Variable_Ptr_C m_second_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Second);
    Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    Rete::Symbol_Identifier_Ptr_C m_s_id = std::make_shared<Rete::Symbol_Identifier>("S1");
    Rete::Symbol_Constant_String_Ptr_C m_clear_attr = std::make_shared<Rete::Symbol_Constant_String>("clear");
    Rete::Symbol_Constant_String_Ptr_C m_in_place_attr = std::make_shared<Rete::Symbol_Constant_String>("in-place");
    Rete::Symbol_Constant_String_Ptr_C m_on_top_attr = std::make_shared<Rete::Symbol_Constant_String>("on-top");
    Rete::Symbol_Constant_String_Ptr_C m_name_attr = std::make_shared<Rete::Symbol_Constant_String>("name");
    Rete::Symbol_Constant_String_Ptr_C m_a_value = std::make_shared<Rete::Symbol_Constant_String>("a");
    Rete::Symbol_Constant_String_Ptr_C m_b_value = std::make_shared<Rete::Symbol_Constant_String>("b");
    Rete::Symbol_Constant_String_Ptr_C m_c_value = std::make_shared<Rete::Symbol_Constant_String>("c");

    Rete::WME_Ptr_C m_x_wme;
    Rete::WME_Ptr_C m_y_wme;

    std::array<std::shared_ptr<action_type::derived_type>, 9> m_action = {{std::make_shared<Move>(1, 0),
                                                                           std::make_shared<Move>(1, 2),
                                                                           std::make_shared<Move>(1, 3),
                                                                           std::make_shared<Move>(2, 0),
                                                                           std::make_shared<Move>(2, 1),
                                                                           std::make_shared<Move>(2, 3),
                                                                           std::make_shared<Move>(3, 0),
                                                                           std::make_shared<Move>(3, 1),
                                                                           std::make_shared<Move>(3, 2),}};
  };

}

#endif
