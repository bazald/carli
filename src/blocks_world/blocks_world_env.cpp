#include "blocks_world.h"

#include "../carli/parser/rete_parser.h"

namespace Blocks_World {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  Environment::Environment() {
    init_impl();

    {
      m_goal.push_front(Stack());
      Stack &stack = *m_goal.begin();
      stack.push_front(3);
      stack.push_front(2);
      stack.push_front(1);
    }
  }

  int64_t Environment::num_blocks() const {
    int64_t nb = 0;
    for(const auto &blocks : m_blocks)
      nb += blocks.size();
    return nb;
  }

  void Environment::init_impl() {
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

  Agent::reward_type Environment::transition_impl(const Carli::Action &action) {
    const Move &move = debuggable_cast<const Move &>(action);

    Stacks::iterator src = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.begin() == move.block;
    });
    Stacks::iterator dest = std::find_if(m_blocks.begin(), m_blocks.end(), [&move](Stack &stack)->bool {
      return !stack.empty() && *stack.begin() == move.dest;
    });

    if(src == m_blocks.end())
      return -10.0; ///< throw std::runtime_error("Attempt to move Block from under another Block.");
    if(dest == m_blocks.end())
      dest = m_blocks.insert(m_blocks.begin(), Stack());

    src->pop_front();
    dest->push_front(move.block);

    if(src->empty())
      m_blocks.erase(src);

    return -1.0;
  }

  void Environment::print_impl(ostream &os) const {
    os << "Blocks World (Table is Left):" << endl;
    for(const Stack &stack : m_blocks) {
      for(const block_id &id : stack)
        os << ' ' << id;
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(token);})
  {
    insert_wme(m_wme_blink);
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
#if 1
    rete_parse_file(*this, "rules/blocks-world.carli");
#else
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    Rete::WME_Bindings state_bindings;

    auto filter_action = make_filter(Rete::WME(m_first_var, m_action_attr, m_third_var));
//    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 0)));
    auto filter_block = make_filter(Rete::WME(m_first_var, m_block_attr, m_third_var));
    auto join_action_block = make_join(state_bindings, filter_action, filter_block);
    auto filter_dest = make_filter(Rete::WME(m_first_var, m_dest_attr, m_third_var));
    auto join_action_dest = make_join(state_bindings, join_action_block, filter_dest);

    auto filter_name = make_filter(Rete::WME(m_first_var, m_name_attr, m_third_var));
    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(1, 2), Rete::WME_Token_Index(0, 0)));
    auto join_block_name = make_join(state_bindings, join_action_dest, filter_name);
    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(2, 2), Rete::WME_Token_Index(0, 0)));
    auto join_dest_name = make_join(state_bindings, join_block_name, filter_name);

    auto filter_height = make_filter(Rete::WME(m_first_var, m_height_attr, m_third_var));
    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(1, 2), Rete::WME_Token_Index(0, 0)));
    auto join_block_height = make_join(state_bindings, join_dest_name, filter_height);
    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(2, 2), Rete::WME_Token_Index(0, 0)));
    auto join_dest_height = make_join(state_bindings, join_block_height, filter_height);

    auto &join_last = join_dest_height;

    auto filter_blink = make_filter(*m_wme_blink);
    auto filter_clear = make_filter(Rete::WME(m_first_var, m_clear_attr, m_third_var));
    auto filter_in_place = make_filter(Rete::WME(m_first_var, m_in_place_attr, m_third_var));

    Carli::Node_Unsplit_Ptr root_action_data;
    {
      auto join_blink = make_existential_join(Rete::WME_Bindings(), join_last, filter_blink);

      auto root_action = make_standard_action(join_blink, next_rule_name("blocks-world*rl-action*u"), false);
      root_action_data = std::make_shared<Node_Unsplit>(*this, root_action, 1, nullptr);
      root_action->data = root_action_data;
    }

    const bool enable_distractors = get_Option_Ranged<bool>(Options::get_global(), "enable-distractors");

    std::vector<Feature::Which> blocks = {{Feature::BLOCK, Feature::DEST}};

    if(!enable_distractors)
      blocks = {Feature::BLOCK};
    for(const auto &block : blocks) {
      auto feature = new Clear(block, true);
      state_bindings.clear();
      state_bindings.insert(Rete::WME_Binding(feature->axis, Rete::WME_Token_Index(0, 0)));
      auto join_block_clear = make_existential_join(state_bindings, join_last, filter_clear);
      make_standard_fringe(join_block_clear, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);

      feature = new Clear(block, false);
      auto neg = make_negation_join(state_bindings, join_last, filter_clear);
      make_standard_fringe(neg, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);
    }

    if(!enable_distractors)
      blocks = {Feature::DEST};
    for(const auto &block : blocks) {
      auto feature = new In_Place(block, true);
      state_bindings.clear();
      state_bindings.insert(Rete::WME_Binding(feature->axis, Rete::WME_Token_Index(0, 0)));
      auto join_block_in_place = make_existential_join(state_bindings, join_last, filter_in_place);
      make_standard_fringe(join_block_in_place, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);

      feature = new In_Place(block, false);
      auto neg = make_negation_join(state_bindings, join_last, filter_in_place);
      make_standard_fringe(neg, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);
    }

    for(size_t block = 1; block != m_block_ids.size(); ++block) {
      auto feature = new Name(Feature::BLOCK, m_block_names[block]->value);
      auto name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, feature->axis, m_block_names[block], join_last);
      make_standard_fringe(name_is, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);
    }

    for(size_t block = 0; block != m_block_ids.size(); ++block) {
      auto feature = new Name(Feature::DEST, m_block_names[block]->value);
      auto name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, feature->axis, m_block_names[block], join_last);
      make_standard_fringe(name_is, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);
    }

    if(enable_distractors) {
      const int64_t num_blocks = env->num_blocks();
      for(const auto &block : blocks) {
        auto feature = new Height(block, 1.0, double(num_blocks / 2) + 2, 2, false);
        auto predicate = make_predicate_vc(feature->predicate(), feature->axis, feature->symbol_constant(), join_last);
        make_standard_fringe(predicate, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);

        feature = new Height(block, double(num_blocks / 2) + 2, double(num_blocks), 2, true);
        predicate = make_predicate_vc(feature->predicate(), feature->axis, feature->symbol_constant(), join_last);
        make_standard_fringe(predicate, next_rule_name("blocks-world*rl-action*f"), false, root_action_data, feature);
      }
    }

//    for(int i = 1; i != 4; ++i) {
//      auto block_name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(4, 2), get_block_name(i), join_dest_in_place);
//
//      for(int j = 0; j != 4; ++j) {
//        auto dest_name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(5, 2), get_block_name(j), block_name_is);
//        auto join_blink = make_existential_join(Rete::WME_Bindings(), dest_name_is, filter_blink);
//
//        auto rl = std::make_shared<Node_Split>(*this, new Q_Value(0.0, Q_Value::Type::SPLIT, 1));
//        rl->action = make_action_retraction([this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//          const auto action = get_action(token);
//          this->insert_q_value_next(action, rl->q_value);
//        }, [this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//          const auto action = get_action(token);
//          this->purge_q_value_next(action, rl->q_value);
//        }, join_blink).get();
//      }
//    }
#endif
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();
    std::list<Rete::WME_Ptr_C> wmes_current;

    std::ostringstream oss;
    for(size_t block = 1; block != m_block_ids.size(); ++block) {
      for(size_t dest = 0; dest != m_block_ids.size(); ++dest) {
        if(block == dest)
          continue;
        oss << "move-" << block << '-' << dest;
        Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[block]));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[dest]));
      }
    }

    for(auto bt = blocks.begin(), bend = blocks.end(); bt != bend; ++bt) {
      int64_t height = 0;
      for(auto st = bt->begin(), send = bt->end(); st != send; ++st) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, m_block_ids[size_t(*st)]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*st)], m_name_attr, m_block_names[size_t(*st)]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*st)], m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(++height)));
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*bt->begin())], m_clear_attr, m_true_value));

//      for(auto st = bt->begin(), stn = ++bt->begin(), send = bt->end(); stn != send; st = stn, ++stn)
//        wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*st), m_on_top_attr, get_block_id(*stn)));

      auto base = bt->rbegin();
      const auto stack = std::find_if(goal.begin(), goal.end(), [&base](const Environment::Stack &stack)->bool{return std::find(stack.begin(), stack.end(), *base) != stack.end();});
      assert(stack != goal.end());
      for(auto base_goal = stack->rbegin(); base != bt->rend() && base_goal != stack->rend() && *base == *base_goal; ++base, ++base_goal)
        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[size_t(*base)], m_in_place_attr, m_true_value));
    }

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, m_block_ids[0]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_name_attr, m_block_names[0]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_clear_attr, m_true_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[0], m_in_place_attr, m_true_value));

    remove_wme(m_wme_blink);

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

    insert_wme(m_wme_blink);
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->get_blocks() == env->get_goal() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

}
