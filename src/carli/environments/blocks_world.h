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
  class Feature : public Feature_Present {
  public:
    Feature(const bool &present_)
     : Feature_Present(present_)
    {
    }

    virtual Feature * clone() const = 0;

    int compare_axis(const ::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

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

  class Move : public Action {
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

    int compare(const Action &rhs) const {
      return compare(debuggable_cast<const Move &>(rhs));
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

  class Environment : public ::Environment {
  public:
    Environment();

    typedef std::list<block_id> Stack;
    typedef std::list<Stack> Stacks;

    const Stacks & get_blocks() const {return m_blocks;}
    const Stacks & get_goal() const {return m_goal;}

  private:
    void init_impl();

    reward_type transition_impl(const Action &action);

    void print_impl(ostream &os) const;

    Stacks m_blocks;
    Stacks m_goal;
  };

  class Agent : public ::Agent {
  public:
    Agent(const std::shared_ptr< ::Environment> &env);
    ~Agent();

  private:
    void generate_rete();

    void generate_features();

    void update();

    const Rete::Symbol_Identifier_Ptr_C & get_block_id(const block_id &block);
    const Rete::Symbol_Constant_String_Ptr_C & get_block_name(const block_id &block);

    Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    Rete::Symbol_Identifier_Ptr_C m_s_id = std::make_shared<Rete::Symbol_Identifier>("S1");
    Rete::Symbol_Identifier_Ptr_C m_input_id = std::make_shared<Rete::Symbol_Identifier>("I1");
    Rete::Symbol_Identifier_Ptr_C m_a_id = std::make_shared<Rete::Symbol_Identifier>("A");
    Rete::Symbol_Identifier_Ptr_C m_b_id = std::make_shared<Rete::Symbol_Identifier>("B");
    Rete::Symbol_Identifier_Ptr_C m_c_id = std::make_shared<Rete::Symbol_Identifier>("C");
    Rete::Symbol_Identifier_Ptr_C m_table_id = std::make_shared<Rete::Symbol_Identifier>("TABLE");
    Rete::Symbol_Constant_String_Ptr_C m_input_attr = std::make_shared<Rete::Symbol_Constant_String>("input");
    Rete::Symbol_Constant_String_Ptr_C m_action_attr = std::make_shared<Rete::Symbol_Constant_String>("action");
    Rete::Symbol_Constant_String_Ptr_C m_index_attr = std::make_shared<Rete::Symbol_Constant_String>("index");
    Rete::Symbol_Constant_String_Ptr_C m_dest_attr = std::make_shared<Rete::Symbol_Constant_String>("dest");
    Rete::Symbol_Constant_String_Ptr_C m_block_attr = std::make_shared<Rete::Symbol_Constant_String>("block");
    Rete::Symbol_Constant_String_Ptr_C m_clear_attr = std::make_shared<Rete::Symbol_Constant_String>("clear");
    Rete::Symbol_Constant_String_Ptr_C m_in_place_attr = std::make_shared<Rete::Symbol_Constant_String>("in-place");
    Rete::Symbol_Constant_String_Ptr_C m_on_top_attr = std::make_shared<Rete::Symbol_Constant_String>("on-top");
    Rete::Symbol_Constant_String_Ptr_C m_name_attr = std::make_shared<Rete::Symbol_Constant_String>("name");
    Rete::Symbol_Constant_String_Ptr_C m_a_value = std::make_shared<Rete::Symbol_Constant_String>("a");
    Rete::Symbol_Constant_String_Ptr_C m_b_value = std::make_shared<Rete::Symbol_Constant_String>("b");
    Rete::Symbol_Constant_String_Ptr_C m_c_value = std::make_shared<Rete::Symbol_Constant_String>("c");
    Rete::Symbol_Constant_String_Ptr_C m_table_value = std::make_shared<Rete::Symbol_Constant_String>("table");
    Rete::Symbol_Constant_String_Ptr_C m_true_value = std::make_shared<Rete::Symbol_Constant_String>("true");

    Rete::WME_Ptr_C m_wme_blink = std::make_shared<Rete::WME>(m_s_id, m_s_id, m_s_id);
    std::list<Rete::WME_Ptr_C> m_wmes_prev;

    std::array<tracked_ptr<const Action>, 9> m_action = {{new Move(1, 0),
                                                          new Move(1, 2),
                                                          new Move(1, 3),
                                                          new Move(2, 0),
                                                          new Move(2, 1),
                                                          new Move(2, 3),
                                                          new Move(3, 0),
                                                          new Move(3, 1),
                                                          new Move(3, 2)}};

    std::array<Rete::Symbol_Identifier_Ptr_C, 9> m_action_id = {{std::make_shared<Rete::Symbol_Identifier>(m_action[0]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[1]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[2]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[3]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[4]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[5]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[6]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[7]->to_string()),
                                                                 std::make_shared<Rete::Symbol_Identifier>(m_action[8]->to_string())}};

    std::array<Rete::Symbol_Constant_Int_Ptr_C, 9> m_action_index = {{std::make_shared<Rete::Symbol_Constant_Int>(0),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(1),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(2),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(3),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(4),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(5),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(6),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(7),
                                                                      std::make_shared<Rete::Symbol_Constant_Int>(8)}};
  };

}

#endif
