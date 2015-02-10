#include "blocks_world_2.h"

#include "carli/parser/rete_parser.h"

namespace Blocks_World_2 {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  Environment::Environment() {
    init_impl();
  }

  void Environment::init_impl() {
    m_blocks = random_Stacks(m_num_blocks);
    do {
      m_goal = random_Stacks(m_num_blocks);
    } while(m_blocks == m_goal);
  }

  Environment::Stacks Environment::random_Stacks(const int64_t &num_blocks) {
    assert(num_blocks > 0);

    std::vector<int> blocks;
    blocks.reserve(num_blocks);
    for(int i = 1; i <= num_blocks; ++i)
      blocks.push_back(i);
    std::random_shuffle(blocks.begin(), blocks.end());

    Stacks stacks;
    for(const auto &block : blocks) {
      const size_t stack_index = m_random.rand_lte(int32_t(stacks.size()));
      if(stack_index >= stacks.size())
        stacks.push_back(Stack(1, block));
      else
        stacks[stack_index].push_back(block);
    }
    std::sort(stacks.begin(), stacks.end());

    return stacks;
  }

  Agent::reward_type Environment::transition_impl(const Carli::Action &action) {
    const Move &move = debuggable_cast<const Move &>(action);

    Stacks::iterator src = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.rbegin() == move.block;
    });
    Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.rbegin() == move.dest;
    });

    if(src == m_blocks.end())
      return -10.0; ///< throw std::runtime_error("Attempt to move Block from under another Block.");

    src->pop_back();

    if(dest != m_blocks.end()) {
      dest->push_back(move.block);
      if(src->empty())
        m_blocks.erase(src);
    }
    else {
      assert(move.dest == 0);
      assert(src->size() != 0);
      m_blocks.push_back(Stack(1, move.block));
    }

    std::sort(m_blocks.begin(), m_blocks.end());

    return -1.0;
  }

  void Environment::print_impl(ostream &os) const {
    os << "Blocks World (Table is Left):" << endl;
    for(const Stack &stack : m_blocks) {
      for(const block_id &id : stack)
        os << ' ' << char('@' + id);
      os << endl;
    }
    os << "Goal:" << endl;
    for(const Stack &stack : m_goal) {
      for(const block_id &id : stack)
        os << ' ' << char('@' + id);
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);})
  {
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
    std::string rules_in = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
    if(rules_in == "default")
      rules_in = "rules/blocks-world-2.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();
    std::list<Rete::WME_Ptr_C> wmes_current;

    std::ostringstream oss;
    for(const auto &stack : blocks) {
      int64_t height = 0;
      for(const auto &block : stack) {
        const auto block_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + block))));

        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(++height)));

        const double brightness = m_random.frand_lte();
        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_brightness_attr, std::make_shared<Rete::Symbol_Constant_Float>(brightness)));
        if(brightness > 0.5)
          wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_glowing_attr, m_true_value));
      }

      for(const auto &dest_stack : blocks) {
        if(stack == dest_stack)
          continue;
        oss << "move-" << char('@' + *stack.rbegin()) << '-' << char('@' + *dest_stack.rbegin());
        Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *stack.rbegin()))))));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *dest_stack.rbegin()))))));
      }

      if(stack.size() > 1) {
        oss << "move-" << char('@' + *stack.rbegin()) << "-TABLE";
        Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *stack.rbegin()))))));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_table_id));
      }
    }

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_blocks_attr, m_blocks_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_goal_attr, m_goal_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_id, m_name_attr, m_table_name));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_blocks_id, m_stack_attr, m_table_stack_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_goal_id, m_stack_attr, m_table_stack_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_top_attr, m_table_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_table_stack_id, m_matches_attr, m_table_stack_id));

    for(const auto &stack : blocks) {
      std::string stack_str = "|";
      for(const auto &block : stack)
        stack_str += char('@' + block);

      Rete::Symbol_Identifier_Ptr_C stack_id(new Rete::Symbol_Identifier(stack_str));

      wmes_current.push_back(std::make_shared<Rete::WME>(m_blocks_id, m_stack_attr, stack_id));
      for(const auto &block : stack) {
        const Rete::Symbol_Identifier_Ptr_C block_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + block))));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_blocks_id, m_block_attr, block_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(block_id, m_name_attr, Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(block))));
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *stack.rbegin())))), m_clear_attr, m_true_value));

      wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_top_attr, Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *stack.rbegin()))))));
      ///wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_matches_attr, m_table_stack_id));

      for(const auto &goal_stack : goal) {
        if(goal_stack.size() < stack.size())
          continue;

        const auto match = std::mismatch(stack.begin(), stack.end(), goal_stack.begin());

        if(match.first == stack.end()) {
          std::string goal_str = "|";
          for(const auto &block : goal_stack)
            goal_str += char('@' + block);

          Rete::Symbol_Identifier_Ptr_C goal_id(new Rete::Symbol_Identifier(goal_str));

          wmes_current.push_back(std::make_shared<Rete::WME>(stack_id, m_matches_attr, goal_id));

          if(match.second != goal_stack.end())
            wmes_current.push_back(std::make_shared<Rete::WME>(Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *match.second)))), m_matches_top_attr, stack_id));

          break;
        }
      }
    }

    int64_t discrepancy = 0;
    for(const auto &goal_stack : goal) {
      discrepancy += goal_stack.size();
      for(const auto &stack : blocks) {
        const auto match = std::mismatch(goal_stack.begin(), goal_stack.end(), stack.begin());
        if(match.first != goal_stack.begin()) {
          discrepancy -= match.first - goal_stack.begin();
          for(auto bt = goal_stack.begin(); bt != match.first; ++bt)
            wmes_current.push_back(std::make_shared<Rete::WME>(Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *bt)))), m_in_place_attr, m_true_value));
          break;
        }
      }
      wmes_current.push_back(std::make_shared<Rete::WME>(Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *goal_stack.begin())))), m_matches_top_attr, m_table_stack_id));
    }
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_discrepancy_attr, Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(discrepancy))));

    for(const auto &goal_stack : goal) {
      std::string goal_stack_str = "|";
      for(const auto &block : goal_stack)
        goal_stack_str += char('@' + block);

      Rete::Symbol_Identifier_Ptr_C goal_stack_id(new Rete::Symbol_Identifier(goal_stack_str));

      wmes_current.push_back(std::make_shared<Rete::WME>(m_goal_id, m_stack_attr, goal_stack_id));
      if(std::find(blocks.begin(), blocks.end(), goal_stack) == blocks.end())
        wmes_current.push_back(std::make_shared<Rete::WME>(goal_stack_id, m_top_attr, Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *goal_stack.rbegin()))))));
      wmes_current.push_back(std::make_shared<Rete::WME>(Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(std::string(1, char('@' + *goal_stack.begin())))), m_matches_top_attr, m_table_stack_id));
    }

    for(auto wt = m_wmes_prev.begin(), wend = m_wmes_prev.end(); wt != wend; ) {
      const auto found = std::find_if(wmes_current.begin(), wmes_current.end(), [wt](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == **wt;});
      if(found == wmes_current.end()) {
        remove_wme(*wt);
        m_wmes_prev.erase(wt++);
      }
      else {
        wmes_current.erase(found);
        ++wt;
      }
    }

    for(auto &wme : wmes_current) {
      const auto found = std::find_if(m_wmes_prev.begin(), m_wmes_prev.end(), [wme](const Rete::WME_Ptr_C &wme_)->bool{return *wme_ == *wme;});
      if(found == m_wmes_prev.end()) {
        m_wmes_prev.push_back(wme);
        insert_wme(wme);
      }
    }
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();

    for(const auto &gstack : goal) {
      if(std::find(blocks.begin(), blocks.end(), gstack) == blocks.end())
        return;
    }

    m_metastate = Metastate::SUCCESS;
  }

}
