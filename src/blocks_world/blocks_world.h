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

  class Clear;
  class In_Place;
  class Name;
  class Height;

  class Feature;
  class BLOCKS_WORLD_LINKAGE Feature : public Carli::Feature {
  public:
    enum Which {BLOCK = 0, DEST = 1};
    
    Feature(const Rete::WME_Token_Index &axis_, const Rete::WME_Bindings &bindings_) : Carli::Feature(axis_, bindings_) {}

    virtual Feature * clone() const = 0;

  protected:
    static Rete::WME_Bindings make_bindings() {
      Rete::WME_Bindings bindings_;
      bindings_.insert(std::make_pair(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 2)));
      return bindings_;
    }
  };

  class BLOCKS_WORLD_LINKAGE Clear : public Carli::Feature_Enumerated<Feature> {
  public:
    Clear(const Which &block_, const bool &present_)
     : Feature_Enumerated<Feature>(Rete::WME_Token_Index(1 + block_, 2), make_bindings(), present_),
     block(block_)
    {
    }

    Clear * clone() const {
      return new Clear(block, this->value != 0);
    }

    bool matches(const Rete::WME_Token &) const override {return true;}

    void print(ostream &os) const {
      os << "clear(" << block << ':' << value << ')';
    }

    Which block;
  };

  class BLOCKS_WORLD_LINKAGE In_Place : public Carli::Feature_Enumerated<Feature> {
  public:
    In_Place(const Which &block_, const bool &present_)
     : Feature_Enumerated<Feature>(Rete::WME_Token_Index(1 + block_, 2), make_bindings(), present_),
     block(block_)
    {
    }

    In_Place * clone() const {
      return new In_Place(block, this->value != 0);
    }

    bool matches(const Rete::WME_Token &) const override {return true;}

    void print(ostream &os) const {
      os << "in-place(" << block << ':' << value << ')';
    }

    Which block;
  };

  class BLOCKS_WORLD_LINKAGE Name : public Carli::Feature_Enumerated<Feature> {
  public:
    Name(const Which &block_, const block_id &name_)
     : Feature_Enumerated<Feature>(Rete::WME_Token_Index(3 + block_, 2), make_bindings(), true),
     block(block_),
     name(name_)
    {
    }

    Name * clone() const {
      return new Name(block, name);
    }

    int64_t compare_value(const Carli::Feature &rhs) const {
      return name - debuggable_cast<const Name &>(rhs).name;
    }

    bool matches(const Rete::WME_Token &) const override {return true;}

    void print(ostream &os) const {
      os << "name(" << block << ',' << name << ':' << value << ')';
    }

    Which block;
    block_id name;
  };

  class BLOCKS_WORLD_LINKAGE Height : public Carli::Feature_Ranged<Feature> {
  public:
    Height(const Which &block_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_)
     : Carli::Feature_Ranged<Feature>(Rete::WME_Token_Index(5 + block_, 2), make_bindings(), bound_lower_, bound_upper_, depth_, upper_, true),
     block(block_)
    {
    }

    Height * clone() const {
      return new Height(axis.first == 5 ? BLOCK : DEST, bound_lower, bound_upper, depth, upper);
    }

    bool matches(const Rete::WME_Token &) const override {return true;}

    void print(ostream &os) const {
      os << "height(" << Which(axis.first - 5) << ',' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }

    Which block;
  };

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

    Move(const Rete::WME_Token &token)
     : block(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(3, 2)]).value),
     dest(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(4, 2)]).value)
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

    reward_type transition_impl(const Carli::Action &action);

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

    const Rete::Symbol_Variable_Ptr_C m_first_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::First));
    const Rete::Symbol_Variable_Ptr_C m_third_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::Third));

    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action"));
    const Rete::Symbol_Constant_String_Ptr_C m_dest_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("dest"));
    const Rete::Symbol_Constant_String_Ptr_C m_block_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("block"));
    const Rete::Symbol_Constant_String_Ptr_C m_clear_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("clear"));
    const Rete::Symbol_Constant_String_Ptr_C m_in_place_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("in-place"));
//    const Rete::Symbol_Constant_String_Ptr_C m_on_top_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("on-top"));
    const Rete::Symbol_Constant_String_Ptr_C m_height_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("height"));
    const Rete::Symbol_Constant_String_Ptr_C m_name_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("name"));
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("true"));

    /// std::array mysteriously results in crashes when compiled with Visual Studio 12.0.21005.1 REL
    const std::vector<Rete::Symbol_Identifier_Ptr_C> m_block_ids = {{Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("TABLE")),
                                                                     Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("A")),
                                                                     Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("B")),
                                                                     Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("C"))}};
    const std::array<Rete::Symbol_Constant_Int_Ptr_C, 4> m_block_names = {{Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(0)),
                                                                           Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1)),
                                                                           Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2)),
                                                                           Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3))}};

    std::list<Rete::WME_Ptr_C> m_wmes_prev;
  };

}

#endif
