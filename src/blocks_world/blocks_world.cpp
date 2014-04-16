#include "blocks_world.h"

#include "carli/experiment.h"

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
    os << "Blocks World:" << endl;
    for(const Stack &stack : m_blocks) {
      for(const block_id &id : stack)
        os << ' ' << id;
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env)
  {
    insert_wme(m_wme_blink);
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
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

    auto filter_blink = make_filter(*m_wme_blink);
    auto filter_clear = make_filter(Rete::WME(m_first_var, m_clear_attr, m_third_var));
    auto filter_in_place = make_filter(Rete::WME(m_first_var, m_in_place_attr, m_third_var));

    auto get_action = [this](const Rete::WME_Token &token)->action_ptrsc {
      return std::make_shared<Move>(token);
    };

    auto node_unsplit = std::make_shared<Node_Unsplit>(*this, 1);
    {
      auto join_blink = make_existential_join(Rete::WME_Bindings(), join_dest_name, filter_blink);

      node_unsplit->action = make_action_retraction([this,get_action,node_unsplit](const Rete::Rete_Action &rete_action, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        if(!this->specialize(rete_action, token, get_action, node_unsplit))
          this->insert_q_value_next(action, node_unsplit->q_value);
      }, [this,get_action,node_unsplit](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_unsplit->q_value);
      }, join_blink).get();
    }

    std::vector<Feature::Which> blocks = {{Feature::BLOCK, Feature::DEST}};
    const bool disable_distractors = true;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif
    if(disable_distractors)
#ifdef _MSC_VER
#pragma warning(pop)
#endif
      blocks = {Feature::BLOCK};

    for(const auto &block : blocks) {
      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
      auto feature = new Clear(block, true);
      node_fringe->feature = feature;
      state_bindings.clear();
      state_bindings.insert(Rete::WME_Binding(feature->wme_token_index(), Rete::WME_Token_Index(0, 0)));
      auto join_block_clear = make_existential_join(state_bindings, join_dest_name, filter_clear);
      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe->q_value);
      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe->q_value);
      }, join_block_clear).get();
      node_unsplit->fringe_values.push_back(node_fringe);

      auto node_fringe_neg = std::make_shared<Node_Fringe>(*this, 2);
      node_fringe_neg->feature = new Clear(block, false);
      auto neg = make_negation_join(state_bindings, join_dest_name, filter_clear);
      node_fringe_neg->action = make_action_retraction([this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe_neg->q_value);
      }, [this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe_neg->q_value);
      }, neg).get();
      node_unsplit->fringe_values.push_back(node_fringe_neg);
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif
    if(disable_distractors)
      blocks = {Feature::DEST};
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    for(const auto &block : blocks) {
      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
      auto feature = new In_Place(block, true);
      node_fringe->feature = feature;
      state_bindings.clear();
      state_bindings.insert(Rete::WME_Binding(feature->wme_token_index(), Rete::WME_Token_Index(0, 0)));
      auto join_block_in_place = make_existential_join(state_bindings, join_dest_name, filter_in_place);
      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe->q_value);
      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe->q_value);
      }, join_block_in_place).get();
      node_unsplit->fringe_values.push_back(node_fringe);

      auto node_fringe_neg = std::make_shared<Node_Fringe>(*this, 2);
      node_fringe_neg->feature = new In_Place(block, false);
      auto neg = make_negation_join(state_bindings, join_dest_name, filter_in_place);
      node_fringe_neg->action = make_action_retraction([this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe_neg->q_value);
      }, [this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe_neg->q_value);
      }, neg).get();
      node_unsplit->fringe_values.push_back(node_fringe_neg);
    }

    for(size_t block = 1; block != m_block_ids.size(); ++block) {
      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
      auto feature = new Name(Feature::BLOCK, m_block_names[block]->value);
      node_fringe->feature = feature;
      auto name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, feature->wme_token_index(), m_block_names[block], join_dest_name);
      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe->q_value);
      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe->q_value);
      }, name_is).get();
      node_unsplit->fringe_values.push_back(node_fringe);
    }

    for(size_t block = 0; block != m_block_ids.size(); ++block) {
      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
      auto feature = new Name(Feature::DEST, m_block_names[block]->value);
      node_fringe->feature = feature;
      auto name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, feature->wme_token_index(), m_block_names[block], join_dest_name);
      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe->q_value);
      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe->q_value);
      }, name_is).get();
      node_unsplit->fringe_values.push_back(node_fringe);
    }

//    state_bindings.clear();
//    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(2, 2), Rete::WME_Token_Index(0, 0)));
//    auto join_block_clear = make_existential_join(state_bindings, join_dest_name, filter_clear);
//
//    state_bindings.clear();
//    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(3, 2), Rete::WME_Token_Index(0, 0)));
//    auto join_dest_in_place = make_existential_join(state_bindings, join_block_clear, filter_in_place);
//
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
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &blocks = env->get_blocks();
    const auto &goal = env->get_goal();
    std::list<Rete::WME_Ptr_C> wmes_current;

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_input_attr, m_input_id));
    std::ostringstream oss;
    for(size_t block = 1; block != m_block_ids.size(); ++block) {
      for(size_t dest = 0; dest != m_block_ids.size(); ++dest) {
        if(block == dest)
          continue;
        oss << "move-" << block << '-' << dest;
        Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
        oss.str("");
        wmes_current.push_back(std::make_shared<Rete::WME>(m_input_id, m_action_attr, action_id));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_block_attr, m_block_ids[block]));
        wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dest_attr, m_block_ids[dest]));
      }
    }

    for(auto bt = blocks.begin(), bend = blocks.end(); bt != bend; ++bt) {
      for(auto st = bt->begin(), send = bt->end(); st != send; ++st) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_block_attr, m_block_ids[*st]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[*st], m_name_attr, m_block_names[*st]));
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[*bt->begin()], m_clear_attr, m_true_value));

//      for(auto st = bt->begin(), stn = ++bt->begin(), send = bt->end(); stn != send; st = stn, ++stn)
//        wmes_current.push_back(std::make_shared<Rete::WME>(get_block_id(*st), m_on_top_attr, get_block_id(*stn)));

      auto base = bt->rbegin();
      const auto stack = std::find_if(goal.begin(), goal.end(), [&base](const Environment::Stack &stack)->bool{return std::find(stack.begin(), stack.end(), *base) != stack.end();});
      assert(stack != goal.end());
      for(auto base_goal = stack->rbegin(); base != bt->rend() && base_goal != stack->rend() && *base == *base_goal; ++base, ++base_goal)
        wmes_current.push_back(std::make_shared<Rete::WME>(m_block_ids[*base], m_in_place_attr, m_true_value));
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

int main(int argc, char **argv) {
  try {
    Carli::Experiment experiment;

    experiment.take_args(argc, argv);

    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();

    experiment.standard_run([](){return std::make_shared<Blocks_World::Environment>();},
                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Blocks_World::Agent>(env);},
                            [&output](const std::shared_ptr<Carli::Agent> &){}
                           );

    return 0;
  }
  catch(std::exception &ex) {
    std::cerr << "Exiting with exception: " << ex.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Exiting with unknown exception." << std::endl;
  }

  return -1;
}
