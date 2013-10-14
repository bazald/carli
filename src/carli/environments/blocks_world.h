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

  class Clear;
  class In_Place;
  class Name;

  class Feature;
  class Feature : public Feature_Present {
  public:
    enum Which {BLOCK = 0, DEST = 1};

    Feature(const bool &present_)
     : Feature_Present(present_)
    {
    }

    virtual Feature * clone() const = 0;

    int compare_axis(const ::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int compare_axis(const Feature &rhs) const = 0;
    virtual int compare_axis(const Clear &rhs) const = 0;
    virtual int compare_axis(const In_Place &rhs) const = 0;
    virtual int compare_axis(const Name &rhs) const = 0;

    virtual Rete::WME_Token_Index wme_token_index() const = 0;
  };

  class Clear : public Feature {
  public:
    Clear(const Which &block_, const bool &present_)
     : Feature(present_),
     block(block_)
    {
    }

    Clear * clone() const {
      return new Clear(block, this->present);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Clear &rhs) const {
      return block - rhs.block;
    }
    int compare_axis(const In_Place &) const {
      return -1;
    }
    int compare_axis(const Name &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(1 + block, 2);
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "clear(" << block << ')';
    }

    Which block;
  };

  class In_Place : public Feature {
  public:
    In_Place(const Which &block_, const bool &present_)
     : Feature(present_),
     block(block_)
    {
    }

    In_Place * clone() const {
      return new In_Place(block, this->present);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Clear &) const {
      return 1;
    }
    int compare_axis(const In_Place &rhs) const {
      return block - rhs.block;
    }
    int compare_axis(const Name &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(1 + block, 2);
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "in-place(" << block << ')';
    }

    Which block;
  };

  class Name : public Feature {
  public:
    Name(const Which &block_, const block_id &name_)
     : Feature(true),
     block(block_),
     name(name_)
    {
    }

    Name * clone() const {
      return new Name(block, name);
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Clear &) const {
      return 1;
    }
    int compare_axis(const In_Place &) const {
      return 1;
    }
    int compare_axis(const Name &rhs) const {
      return block - rhs.block;
    }

    int compare_value(const ::Feature &rhs) const {
      return name - debuggable_cast<const Name &>(rhs).name;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(3 + block, 2);
    }

    void print(ostream &os) const {
      if(!present)
        os << '!';
      os << "name(" << block << ',' << name << ')';
    }

    Which block;
    block_id name;
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

    Move(const Rete::WME_Token &token)
     : block(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(1, 2)]).value),
     dest(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(2, 2)]).value)
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

    Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    Rete::Symbol_Identifier_Ptr_C m_input_id = std::make_shared<Rete::Symbol_Identifier>("I1");
    Rete::Symbol_Constant_String_Ptr_C m_input_attr = std::make_shared<Rete::Symbol_Constant_String>("input");
    Rete::Symbol_Constant_String_Ptr_C m_action_attr = std::make_shared<Rete::Symbol_Constant_String>("action");
    Rete::Symbol_Constant_String_Ptr_C m_index_attr = std::make_shared<Rete::Symbol_Constant_String>("index");
    Rete::Symbol_Constant_String_Ptr_C m_dest_attr = std::make_shared<Rete::Symbol_Constant_String>("dest");
    Rete::Symbol_Constant_String_Ptr_C m_block_attr = std::make_shared<Rete::Symbol_Constant_String>("block");
    Rete::Symbol_Constant_String_Ptr_C m_clear_attr = std::make_shared<Rete::Symbol_Constant_String>("clear");
    Rete::Symbol_Constant_String_Ptr_C m_in_place_attr = std::make_shared<Rete::Symbol_Constant_String>("in-place");
//    Rete::Symbol_Constant_String_Ptr_C m_on_top_attr = std::make_shared<Rete::Symbol_Constant_String>("on-top");
    Rete::Symbol_Constant_String_Ptr_C m_name_attr = std::make_shared<Rete::Symbol_Constant_String>("name");
    Rete::Symbol_Constant_String_Ptr_C m_true_value = std::make_shared<Rete::Symbol_Constant_String>("true");

    std::array<Rete::Symbol_Identifier_Ptr_C, 4> m_block_ids = {{std::make_shared<Rete::Symbol_Identifier>("TABLE"),
                                                                 std::make_shared<Rete::Symbol_Identifier>("A"),
                                                                 std::make_shared<Rete::Symbol_Identifier>("B"),
                                                                 std::make_shared<Rete::Symbol_Identifier>("C")}};
    std::array<Rete::Symbol_Constant_Int_Ptr_C, 4> m_block_names = {{std::make_shared<Rete::Symbol_Constant_Int>(0),
                                                                     std::make_shared<Rete::Symbol_Constant_Int>(1),
                                                                     std::make_shared<Rete::Symbol_Constant_Int>(2),
                                                                     std::make_shared<Rete::Symbol_Constant_Int>(3)}};

    std::list<Rete::WME_Ptr_C> m_wmes_prev;
  };

}

#endif
