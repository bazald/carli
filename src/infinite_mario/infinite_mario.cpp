#include "infinite_mario.h"

#include "jni_layer.h"

namespace Mario {

  //void infinite_mario_ai(const State &prev, const State &current, Action &action) {
  //  action[BUTTON_LEFT] = 0;
  //  action[BUTTON_RIGHT] = 1;
  //  action[BUTTON_DOWN] = 0;

  ////   for(int i = 0; i != OBSERVATION_SIZE; ++i) {
  ////     for(int j = 0; j != OBSERVATION_SIZE; ++j) {
  ////       if(current.getCompleteObservation[i][j] == SCENE_OBJECT_RED_KOOPA_WINGED ||
  ////          current.getCompleteObservation[i][j] == SCENE_OBJECT_GREEN_KOOPA_WINGED ||
  ////          current.getCompleteObservation[i][j] == SCENE_OBJECT_SPIKY_WINGED
  ////       ) {
  ////         action[BUTTON_LEFT] = 1;
  ////         action[BUTTON_RIGHT] = 0;
  ////         action[BUTTON_DOWN] = 0;
  ////       }
  ////     }
  ////   }

  //  if(prev.action[BUTTON_JUMP])
  //    action[BUTTON_JUMP] = !current.isMarioOnGround;
  //  else
  //    action[BUTTON_JUMP] = current.mayMarioJump;

  //  if(current.getMarioMode == 2)
  //    action[BUTTON_SPEED] = !prev.action[BUTTON_SPEED];
  //  else if(prev.action[BUTTON_SPEED])
  //    action[BUTTON_SPEED] = !current.isMarioCarrying;
  //  else
  //    action[BUTTON_SPEED] = 1;
  //}

  static Agent & get_Agent(const std::shared_ptr<State> &current, const Action &action) {
    static Agent g_Agent(current);
    return g_Agent;
  }

  void infinite_mario_ai(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, Action &action) {
    Agent &agent = get_Agent(current, action);

    const double reward = agent.act();
  }
  
  Agent::Agent(const std::shared_ptr<State> &state)
   : Carli::Agent(state)
  {
    insert_wme(m_wme_blink);
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  template<typename SUBFEATURE, typename AXIS>
  void Agent::generate_rete_continuous(const Carli::Node_Unsplit_Ptr &node_unsplit,
                                       const std::function<action_ptrsc(const Rete::WME_Token &token)> &get_action,
                                       const AXIS &axis,
                                       const double &lower_bound,
                                       const double &upper_bound)
  {
    const double midpt = floor((lower_bound + upper_bound) / 2.0);
    const double values[][2] = {{lower_bound, midpt},
                                {midpt, upper_bound}};

    for(int i = 0; i != 2; ++i) {
      Node_Ranged::Lines lines;
//      lines.push_back(Node_Ranged::Line(std::make_pair(5, ), std::make_pair(5, 20)));
      auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                      Node_Ranged::Range(/*std::make_pair(0, 0), std::make_pair(5, 20)*/),
                                                      lines);
      auto feature = new SUBFEATURE(axis, values[i][0], values[i][1], 2, i != 0);
      nfr->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(axis, 2), feature->symbol_constant(), node_unsplit->action->parent());
      nfr->action = make_action_retraction([this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, nfr->q_value);
      }, [this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, nfr->q_value);
      }, predicate).get();
      node_unsplit->fringe_values.push_back(nfr);
    }
  }

  void Agent::generate_rete() {
    Rete::WME_Bindings state_bindings;

//    state_bindings.clear();
    auto filter_action = make_filter(Rete::WME(m_first_var, m_action_attr, m_third_var));
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 0)));
    auto filter_type_current = make_filter(Rete::WME(m_first_var, m_type_current_attr, m_third_var));
    auto join_type_current = make_join(state_bindings, filter_action, filter_type_current);
    state_bindings.clear();
    auto filter_type_next = make_filter(Rete::WME(m_first_var, m_type_next_attr, m_third_var));
    auto join_type_next = make_join(state_bindings, join_type_current, filter_type_next);
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 0)));
    auto filter_width = make_filter(Rete::WME(m_first_var, m_width_attr, m_third_var));
    auto join_width = make_join(state_bindings, join_type_next, filter_width);
    auto filter_height = make_filter(Rete::WME(m_first_var, m_height_attr, m_third_var));
    auto join_height = make_join(state_bindings, join_width, filter_height);
    auto filter_x = make_filter(Rete::WME(m_first_var, m_x_attr, m_third_var));
    auto join_x = make_join(state_bindings, join_height, filter_x);
    auto filter_y = make_filter(Rete::WME(m_first_var, m_y_attr, m_third_var));
    auto join_y = make_join(state_bindings, join_x, filter_y);
    auto filter_gaps_beneath = make_filter(Rete::WME(m_first_var, m_gaps_beneath_attr, m_third_var));
    auto join_gaps_beneath = make_join(state_bindings, join_y, filter_gaps_beneath);
    auto filter_gaps_created = make_filter(Rete::WME(m_first_var, m_gaps_created_attr, m_third_var));
    auto join_gaps_created = make_join(state_bindings, join_gaps_beneath, filter_gaps_created);
    auto filter_depth_to_gap = make_filter(Rete::WME(m_first_var, m_depth_to_gap_attr, m_third_var));
    auto join_depth_to_gap = make_join(state_bindings, join_gaps_created, filter_depth_to_gap);
    auto filter_clears = make_filter(Rete::WME(m_first_var, m_clears_attr, m_third_var));
    auto join_clears = make_join(state_bindings, join_depth_to_gap, filter_clears);
    auto filter_enables_clearing = make_filter(Rete::WME(m_first_var, m_enables_clearing_attr, m_third_var));
    auto join_enables_clearing = make_join(state_bindings, join_clears, filter_enables_clearing);
    auto filter_prohibits_clearing = make_filter(Rete::WME(m_first_var, m_prohibits_clearing_attr, m_third_var));
    auto join_prohibits_clearing = make_join(state_bindings, join_enables_clearing, filter_prohibits_clearing);
    auto filter_x_odd = make_filter(Rete::WME(m_first_var, m_x_odd_attr, m_third_var));
    auto join_x_odd = make_join(state_bindings, join_prohibits_clearing, filter_x_odd);
    auto &join_last = join_x_odd;

    auto filter_blink = make_filter(*m_wme_blink);

    auto get_action = [this](const Rete::WME_Token &token)->action_ptrsc {
      return std::make_shared<Place>(token);
    };

    auto node_unsplit = std::make_shared<Node_Unsplit>(*this, 1);
    {
      auto join_blink = make_existential_join(Rete::WME_Bindings(), join_last, filter_blink);

      node_unsplit->action = make_action_retraction([this,get_action,node_unsplit](const Rete::Rete_Action &rete_action, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        if(!this->specialize(rete_action, get_action, node_unsplit))
          this->insert_q_value_next(action, node_unsplit->q_value);
      }, [this,get_action,node_unsplit](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_unsplit->q_value);
      }, join_blink).get();
    }

    for(Type::Axis axis : {Type::CURRENT/*, Type::NEXT*/}) {
      for(auto super : {TETS_SQUARE, TETS_LINE, TETS_T, TETS_L, TETS_J, TETS_S, TETS_Z}) {
        for(uint8_t orientation = 0, oend = num_types(super); orientation != oend; ++orientation) {
          const auto type = super_to_type(super, orientation);
          auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
          auto feature = new Type(axis, type);
          node_fringe->feature = feature;
          auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(axis, 2), feature->symbol_constant(), node_unsplit->action->parent());
          node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
            const auto action = get_action(token);
            this->insert_q_value_next(action, node_fringe->q_value);
          }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
            const auto action = get_action(token);
            this->purge_q_value_next(action, node_fringe->q_value);
          }, predicate).get();
          node_unsplit->fringe_values.push_back(node_fringe);
        }
      }
    }

//    for(auto value : {true, false}) {
//      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
//      auto feature = new X_Odd(value);
//      node_fringe->feature = feature;
//      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(X_Odd::AXIS, 2), feature->symbol_constant(), node_unsplit->action->parent());
//      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe->q_value);
//      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe->q_value);
//      }, predicate).get();
//      node_unsplit->fringe_values.push_back(node_fringe);
//    }

    generate_rete_continuous<Size, Size::Axis>(node_unsplit, get_action, Size::WIDTH, 0.0f, 4.0f);
    generate_rete_continuous<Size, Size::Axis>(node_unsplit, get_action, Size::HEIGHT, 0.0f, 4.0f);
    generate_rete_continuous<Position, Position::Axis>(node_unsplit, get_action, Position::X, 0.0f, 10.0f);
    generate_rete_continuous<Position, Position::Axis>(node_unsplit, get_action, Position::Y, 0.0f, 20.0f);
    generate_rete_continuous<Gaps, Gaps::Axis>(node_unsplit, get_action, Gaps::BENEATH, 0.0f, 75.0f);
    generate_rete_continuous<Gaps, Gaps::Axis>(node_unsplit, get_action, Gaps::CREATED, 0.0f, 75.0f);
//    generate_rete_continuous<Gaps, Gaps::Axis>(node_unsplit, get_action, Gaps::DEPTH, 0.0f, 20.0f);
    generate_rete_continuous<Clears, Clears::Axis>(node_unsplit, get_action, Clears::CLEARS, 0.0f, 5.0f);
    generate_rete_continuous<Clears, Clears::Axis>(node_unsplit, get_action, Clears::ENABLES, 0.0f, 5.0f);
    generate_rete_continuous<Clears, Clears::Axis>(node_unsplit, get_action, Clears::PROHIBITS, 0.0f, 5.0f);
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    std::list<Rete::WME_Ptr_C> wmes_current;
    std::ostringstream oss;

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_input_attr, m_input_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_input_id, m_type_next_attr, std::make_shared<Rete::Symbol_Constant_Int>(env->get_next())));

    size_t index = 0;
    for(const auto &placement : env->get_placements()) {
      oss << "place-" << ++index;
      Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_input_id, m_action_attr, action_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_type_current_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.type)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_width_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.size.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.size.second)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_x_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.position.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_y_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.position.second)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_gaps_beneath_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.gaps_beneath)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_gaps_created_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.gaps_created)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_depth_to_gap_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.depth_to_gap)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_x_odd_attr, (placement.position.first & 1) ? m_true_value : m_false_value));

      int clears = 0;
      int enables = 0;
      int prohibits = 0;
      for(int i = 1; i != 5; ++i) {
        if(placement.outcome[i] == Environment::Outcome::OUTCOME_ACHIEVED)
          clears = i;
        else if(placement.outcome[i] == Environment::Outcome::OUTCOME_ENABLED)
          enables = i;
      }
      for(int i = 4; i != 0; --i)
        if(placement.outcome[i] == Environment::Outcome::OUTCOME_PROHIBITED)
          prohibits = i;

      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_clears_attr, std::make_shared<Rete::Symbol_Constant_Int>(clears)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_enables_clearing_attr, std::make_shared<Rete::Symbol_Constant_Int>(enables)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_prohibits_clearing_attr, std::make_shared<Rete::Symbol_Constant_Int>(prohibits)));
    }

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

    m_metastate = env->get_placements().empty() ? Metastate::FAILURE : Metastate::NON_TERMINAL;
  }

}

int main(int argc, char **argv) {
  try {
    Carli::Experiment experiment;

    experiment.take_args(argc, argv);

    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();

    experiment.standard_run([](){return std::make_shared<Tetris::Environment>();},
                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Tetris::Agent>(env);},
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
