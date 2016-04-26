#ifndef BLOCKS_WORLD_H
#define BLOCKS_WORLD_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>

#if !defined(_WINDOWS)
#define BLOCKS_WORLD_LINKAGE
#elif !defined(BLOCKS_WORLD_INTERNAL)
#define BLOCKS_WORLD_LINKAGE __declspec(dllimport)
#else
#define BLOCKS_WORLD_LINKAGE __declspec(dllexport)
#endif

namespace Blocks_World {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  typedef int64_t block_id;

  /** Stupid features:
    * BLOCK ^column C
    * BLOCK ^height H
    * SRC ^right-of DEST
    */

  class BLOCKS_WORLD_LINKAGE Move : public Carli::Action {
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

    Move(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : block(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("block-name")->second]).value),
     dest(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("dest-name")->second]).value)
    {
    }

    Move * clone() const {
      return new Move(block, dest);
    }

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Move &>(rhs));
    }

    int64_t compare(const Move &rhs) const {
      return block != rhs.block ? block - rhs.block : dest - rhs.dest;
    }

    void print_impl(ostream &os) const {
      os << "move(" << block << ',' << dest << ')';
    }

    block_id block;
    block_id dest;
  };

  class BLOCKS_WORLD_LINKAGE Environment : public Carli::Environment {
  public:
    Environment();

    typedef std::list<block_id> Stack;
    typedef std::list<Stack> Stacks;

    const Stacks & get_blocks() const {return m_blocks;}
    const Stacks & get_goal() const {return m_goal;}

    int64_t num_blocks() const;

  private:
    void init_impl();

    std::pair<reward_type, reward_type> transition_impl(const Carli::Action &action);

    void print_impl(ostream &os) const;

    Stacks m_blocks;
    Stacks m_goal;
  };

  class BLOCKS_WORLD_LINKAGE Agent : public Carli::Agent {
  public:
    Agent(const std::shared_ptr<Carli::Environment> &env);
    ~Agent();

  private:
    void generate_rete();

    void generate_features();

    void update();

    Zeni::Random m_random;

    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action"));
    const Rete::Symbol_Constant_String_Ptr_C m_dest_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("dest"));
    const Rete::Symbol_Constant_String_Ptr_C m_block_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("block"));
    const Rete::Symbol_Constant_String_Ptr_C m_brightness_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("brightness"));
    const Rete::Symbol_Constant_String_Ptr_C m_clear_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("clear"));
    const Rete::Symbol_Constant_String_Ptr_C m_glowing_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("glowing"));
    const Rete::Symbol_Constant_String_Ptr_C m_height_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("height"));
    const Rete::Symbol_Constant_String_Ptr_C m_in_place_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("in-place"));
    const Rete::Symbol_Constant_String_Ptr_C m_name_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("name"));
//    const Rete::Symbol_Constant_String_Ptr_C m_on_top_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("on-top"));
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("true"));

    /// http://msdn.microsoft.com/en-us/library/dn793970.aspx
    const std::array<Rete::Symbol_Identifier_Ptr_C, 7> m_block_ids;
    const std::array<Rete::Symbol_Constant_Int_Ptr_C, 7> m_block_names;

    std::list<Rete::WME_Ptr_C> m_wmes_prev;
  };

}

#endif
