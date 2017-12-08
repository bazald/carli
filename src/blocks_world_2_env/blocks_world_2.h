#ifndef BLOCKS_WORLD_2_H
#define BLOCKS_WORLD_2_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>

#if !defined(_WINDOWS)
#define BLOCKS_WORLD_2_LINKAGE
#elif !defined(BLOCKS_WORLD_2_INTERNAL)
#define BLOCKS_WORLD_2_LINKAGE __declspec(dllimport)
#else
#define BLOCKS_WORLD_2_LINKAGE __declspec(dllexport)
#endif

namespace Blocks_World_2 {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  typedef int64_t block_id;

  /** Stupid features:
    * BLOCK ^column C
    * BLOCK ^height H
    * SRC ^right-of DEST
    */

  class BLOCKS_WORLD_2_LINKAGE Move : public Carli::Action {
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

  class BLOCKS_WORLD_2_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &);
    Environment & operator=(const Environment &);

  public:
    enum class Goal {EXACT, COLOR, STACK, UNSTACK, ON_A_B};
    enum class Reward {GUIDING, BLIND};

    Environment();

    struct Block {
      Block(const block_id &id_, const int32_t &color_) : id(id_), color(color_) {}

      bool operator==(const Block &rhs) const {
        return id == rhs.id && color == rhs.color;
      }

      bool operator<(const Block &rhs) const {
        return id < rhs.id || (id == rhs.id && color < rhs.color);
      }

      block_id id;
      int32_t color;
    };

    typedef std::vector<Block> Stack;
    typedef std::vector<Stack> Stacks;

    const Goal & get_goal() const {return m_goal;}
    const int64_t & get_num_blocks_min() const {return m_num_blocks_min;}
    const int64_t & get_num_blocks_max() const {return m_num_blocks_max;}
    const std::function<bool (const Environment::Block &lhs, const Environment::Block &rhs)> & get_match_test() const {return m_match_test;}
    const Stacks & get_blocks() const {return m_blocks;}
    const Stacks & get_target() const {return m_target;}
    const Block & get_table() const {return m_table;}

    bool success() const;

  private:
    void init_impl();
    Stacks random_Stacks(const std::vector<Block> &blocks);

    std::pair<reward_type, reward_type> transition_impl(const Carli::Action &action);
    int64_t matching_blocks() const;

    void print_impl(ostream &os) const;

    Goal m_goal = dynamic_cast<const Option_Itemized &>(Options::get_global()["bw2-goal"]).get_value() == "exact" ? Goal::EXACT
                : dynamic_cast<const Option_Itemized &>(Options::get_global()["bw2-goal"]).get_value() == "color" ? Goal::COLOR
                : dynamic_cast<const Option_Itemized &>(Options::get_global()["bw2-goal"]).get_value() == "stack" ? Goal::STACK
                : dynamic_cast<const Option_Itemized &>(Options::get_global()["bw2-goal"]).get_value() == "unstack" ? Goal::UNSTACK
                : Goal::ON_A_B;
    const Reward m_reward = dynamic_cast<const Option_Itemized &>(Options::get_global()["bw2-reward"]).get_value() == "guiding" ? Reward::GUIDING : Reward::BLIND;
    const int64_t m_num_blocks_min = get_Option_Ranged<int64_t>(Options::get_global(), "num-blocks-min");
    const int64_t m_num_blocks_max = get_Option_Ranged<int64_t>(Options::get_global(), "num-blocks-max");

    std::function<bool (const Environment::Block &lhs, const Environment::Block &rhs)> m_match_test;

    Zeni::Random m_random;
    Stacks m_blocks;
    Stacks m_target;
    Block m_table = Block(0, 0);
  };

  class BLOCKS_WORLD_2_LINKAGE Agent : public Carli::Agent {
    Agent(const Agent &);
    Agent & operator=(const Agent &);

  public:
    Agent(const std::shared_ptr<Carli::Environment> &env);
    ~Agent();

  private:
    void generate_rete();

    void generate_features();

    void update();

    Zeni::Random m_random;

    /*
     * Objects: workspace/goal
     *          stack
     *          block
     *          name
     *
     * Relations: stack ^matches goal-stack
     *            stack ^top block
     *            block ^matches-top stack
     */

    const Rete::Symbol_Constant_String_Ptr_C m_above_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("above"));
    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action"));
    const Rete::Symbol_Constant_String_Ptr_C m_action_in_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action-in"));
    const Rete::Symbol_Constant_String_Ptr_C m_action_out_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action-out"));
    const Rete::Symbol_Constant_String_Ptr_C m_block_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("block"));
    const Rete::Symbol_Constant_String_Ptr_C m_dest_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("dest"));
    const Rete::Symbol_Constant_String_Ptr_C m_color_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("color"));
    const Rete::Symbol_Constant_String_Ptr_C m_brightness_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("brightness")); ///< Distractor
    const Rete::Symbol_Constant_String_Ptr_C m_glowing_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("glowing")); ///< Distractor
    const Rete::Symbol_Constant_String_Ptr_C m_height_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("height")); ///< Distractor
    const Rete::Symbol_Constant_String_Ptr_C m_clear_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("clear")); ///< Legacy
    const Rete::Symbol_Constant_String_Ptr_C m_on_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("on")); ///< Legacy
    const Rete::Symbol_Constant_String_Ptr_C m_in_place_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("in-place")); ///< Legacy
    const Rete::Symbol_Constant_String_Ptr_C m_name_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("name"));
    const Rete::Symbol_Constant_String_Ptr_C m_blocks_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("blocks"));
    const Rete::Symbol_Constant_String_Ptr_C m_stacks_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("stacks"));
    const Rete::Symbol_Constant_String_Ptr_C m_discrepancy_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("discrepancy"));
    const Rete::Symbol_Constant_String_Ptr_C m_higher_than_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("higher-than"));
    const Rete::Symbol_Constant_String_Ptr_C m_target_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("goal"));
    const Rete::Symbol_Constant_String_Ptr_C m_goal_on_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("goal-on"));
    const Rete::Symbol_Constant_String_Ptr_C m_stack_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("stack"));
    const Rete::Symbol_Constant_String_Ptr_C m_matches_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("matches"));
    const Rete::Symbol_Constant_String_Ptr_C m_early_matches_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("early-matches")); ///< Defective
    const Rete::Symbol_Constant_String_Ptr_C m_late_matches_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("late-matches")); ///< Defective
    const Rete::Symbol_Constant_String_Ptr_C m_tallest_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("tallest"));
    const Rete::Symbol_Constant_String_Ptr_C m_top_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top"));
    const Rete::Symbol_Constant_String_Ptr_C m_matches_top_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("matches-top"));
    const Rete::Symbol_Constant_String_Ptr_C m_early_matches_top_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("early-matches-top")); ///< Defective
    const Rete::Symbol_Constant_String_Ptr_C m_late_matches_top_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("late-matches-top")); ///< Defective
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("true"));
    const Rete::Symbol_Constant_String_Ptr_C m_false_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("false"));
    const Rete::Symbol_Identifier_Ptr_C m_blocks_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("BLOCKS"));
    const Rete::Symbol_Identifier_Ptr_C m_stacks_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("STACKS"));
    const Rete::Symbol_Identifier_Ptr_C m_target_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("GOAL"));
    const Rete::Symbol_Identifier_Ptr_C m_table_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("TABLE"));
    const Rete::Symbol_Identifier_Ptr_C m_table_stack_id = m_table_id; //Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("|"));
    const Rete::Symbol_Constant_Int_Ptr_C m_table_name = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(0));

    std::map<block_id, Rete::Symbol_Identifier_Ptr_C> m_block_ids;
    std::map<block_id, Rete::Symbol_Constant_Int_Ptr_C> m_block_names;
    std::map<block_id, Rete::Symbol_Identifier_Ptr_C> m_stack_ids;
    std::map<block_id, Rete::Symbol_Identifier_Ptr_C> m_target_stack_ids;

    std::unordered_map<Rete::WME_Ptr_C, bool, Rete::hash_deref<Rete::WME>, Rete::compare_deref_eq> m_wmes;

    void clear_old_wmes() {
      auto wt = m_wmes.begin(), wend = m_wmes.end();
      while(wt != wend) {
        if(wt->second)
          wt++->second = false;
        else {
          {
            CPU_Accumulator cpu_accumulator(*this);
            remove_wme(wt->first);
          }
          wt = m_wmes.erase(wt);
        }
      }
    }

    void insert_new_wme(const Rete::WME_Ptr_C &wme) {
      const auto found = m_wmes.find(wme);
      if(found == m_wmes.end()) {
        m_wmes[wme] = true;
        CPU_Accumulator cpu_accumulator(*this);
        insert_wme(wme);
      }
      else
        found->second = true;
    }

//    double m_feature_generation_time = 0.0;
  };

}

#endif
