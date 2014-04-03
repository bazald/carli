#include "infinite_mario.h"

#include "jni_layer.h"

namespace Mario {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Fringe_Ranged;
  using Carli::Node_Ranged;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  //void infinite_mario_ai(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, Action &action) {
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

  //  if(prev->action[BUTTON_JUMP])
  //    action[BUTTON_JUMP] = !current->isMarioOnGround;
  //  else
  //    action[BUTTON_JUMP] = current->mayMarioJump;

  //  if(current->getMarioMode == 2)
  //    action[BUTTON_SPEED] = !prev->action[BUTTON_SPEED];
  //  else if(prev->action[BUTTON_SPEED])
  //    action[BUTTON_SPEED] = !current->isMarioCarrying;
  //  else
  //    action[BUTTON_SPEED] = 1;
  //}

  static Agent & get_Agent(const std::shared_ptr<State> &current_) {
    static Agent g_Agent(current_);
    return g_Agent;
  }

  static bool infinite_mario_ai_initialized = false;

  void infinite_mario_ai(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, Action &action) {
    Agent &agent = get_Agent(current);

    if(infinite_mario_ai_initialized)
      agent.act_part_2(prev, current);
    else {
      infinite_mario_ai_initialized = true;
      agent.init();
    }

    agent.act_part_1(action);
  }
  
  void Agent::act_part_1(Action &action) {
    m_current = m_next;
    m_current_q_value = m_next_q_values[m_next];
    m_current_q_value.sort([](const tracked_ptr<Q_Value> &lhs, const tracked_ptr<Q_Value> &rhs)->bool{return lhs->depth < rhs->depth;});

    assert(m_current);

    action = debuggable_cast<const Button_Presses &>(*m_next).action;
  }

  void Agent::act_part_2(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current) {
    const reward_type reward = current->getMarioFloatPos.first - prev->getMarioFloatPos.first > 0 ? 1 : -100;

    update();

    if(m_metastate == Metastate::NON_TERMINAL) {
      generate_features();
      clean_features();

      m_next = m_target_policy();
  #ifdef DEBUG_OUTPUT
  //      for(auto &next_q : m_next_q_values)
  //        std::cerr << "   " << *next_q.first << " is an option." << std::endl;
      std::cerr << "   " << *m_next << " is next." << std::endl;
  #endif
      auto &value_best = m_next_q_values[m_next];
      td_update(m_current_q_value, reward, value_best);

      if(!is_on_policy()) {
        action_ptrsc next = m_exploration_policy();

        if(*m_next != *next) {
          if(sum_value(nullptr, m_current_q_value) < sum_value(nullptr, m_next_q_values[next]))
            clear_eligibility_trace();
          m_next = next;
        }

  #ifdef DEBUG_OUTPUT
        std::cerr << "   " << *m_next << " is next." << std::endl;
  #endif
      }
    }
    else {
      td_update(m_current_q_value, reward, Q_Value_List());
    }

    //m_total_reward += reward;
    //++m_step_count;
  }

  Agent::Agent(const std::shared_ptr<State> &current_)
   : Carli::Agent(current_), m_current_state(current_)
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

    auto filter_x = make_filter(Rete::WME(m_first_var, m_x_attr, m_third_var));
    auto filter_y = make_filter(Rete::WME(m_first_var, m_y_attr, m_third_var));
    auto filter_mode = make_filter(Rete::WME(m_first_var, m_mode_attr, m_third_var));
    auto filter_on_ground = make_filter(Rete::WME(m_first_var, m_on_ground_attr, m_third_var));
    auto filter_may_jump = make_filter(Rete::WME(m_first_var, m_may_jump_attr, m_third_var));
    auto filter_is_carrying = make_filter(Rete::WME(m_first_var, m_is_carrying_attr, m_third_var));
    auto filter_dpad = make_filter(Rete::WME(m_first_var, m_dpad_attr, m_third_var));
    auto filter_jump = make_filter(Rete::WME(m_first_var, m_jump_attr, m_third_var));
    auto filter_speed = make_filter(Rete::WME(m_first_var, m_speed_attr, m_third_var));

    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 0)));

    auto filter_button_presses_in = make_filter(Rete::WME(m_first_var, m_button_presses_in_attr, m_third_var));
    auto join_button_presses_in_dpad = make_join(state_bindings, filter_button_presses_in, filter_dpad);
    auto join_button_presses_in_jump = make_join(state_bindings, join_button_presses_in_dpad, filter_jump);
    auto join_button_presses_in_speed = make_join(state_bindings, join_button_presses_in_jump, filter_speed);

    auto filter_button_presses_out = make_filter(Rete::WME(m_first_var, m_button_presses_out_attr, m_third_var));
    auto join_button_presses_out_dpad = make_join(state_bindings, filter_button_presses_out, filter_dpad);
    auto join_button_presses_out_jump = make_join(state_bindings, join_button_presses_out_dpad, filter_jump);
    auto join_button_presses_out_speed = make_join(state_bindings, join_button_presses_out_jump, filter_speed);

    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));

    auto join_state_x_y = make_join(state_bindings, filter_x, filter_y);
    auto join_state_mode = make_join(state_bindings, join_state_x_y, filter_mode);
    auto join_state_on_ground = make_join(state_bindings, join_state_mode, filter_on_ground);
    auto join_state_may_jump = make_join(state_bindings, join_state_on_ground, filter_may_jump);
    auto join_state_is_carrying = make_join(state_bindings, join_state_may_jump, filter_is_carrying);

    auto join_complete_state = make_join(state_bindings, join_state_is_carrying, join_button_presses_in_speed);

    auto join_last = make_join(state_bindings, join_complete_state, join_button_presses_out_speed);

    auto filter_blink = make_filter(*m_wme_blink);

    auto get_action = [this](const Rete::WME_Token &token)->action_ptrsc {
      return std::make_shared<Button_Presses>(token);
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

    //for(Type::Axis axis : {Type::CURRENT/*, Type::NEXT*/}) {
    //  for(auto super : {TETS_SQUARE, TETS_LINE, TETS_T, TETS_L, TETS_J, TETS_S, TETS_Z}) {
    //    for(uint8_t orientation = 0, oend = num_types(super); orientation != oend; ++orientation) {
    //      const auto type = super_to_type(super, orientation);
    //      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
    //      auto feature = new Type(axis, type);
    //      node_fringe->feature = feature;
    //      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(axis, 2), feature->symbol_constant(), node_unsplit->action->parent());
    //      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
    //        const auto action = get_action(token);
    //        this->insert_q_value_next(action, node_fringe->q_value);
    //      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
    //        const auto action = get_action(token);
    //        this->purge_q_value_next(action, node_fringe->q_value);
    //      }, predicate).get();
    //      node_unsplit->fringe_values.push_back(node_fringe);
    //    }
    //  }
    //}

    /*** State ***/

    //generate_rete_continuous<Feature_Position, Feature_Position::Axis>(node_unsplit, get_action, Feature_Position::X, 0.0f, 4000.0f);
    //generate_rete_continuous<Feature_Position, Feature_Position::Axis>(node_unsplit, get_action, Feature_Position::Y, 0.0f, 200.0f);

    /*** Output Buttons ***/

    for(const auto dpad : {BUTTON_NONE, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_DOWN}) {
      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
      auto feature = new Feature_Button(Feature_Button::OUT_DPAD, dpad);
      node_fringe->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Feature_Button::OUT_DPAD, 2), feature->symbol_constant(), node_unsplit->action->parent());
      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, node_fringe->q_value);
      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, node_fringe->q_value);
      }, predicate).get();
      node_unsplit->fringe_values.push_back(node_fringe);
    }

    for(const auto button : {Feature_Button::OUT_JUMP, Feature_Button::OUT_SPEED}) {
      for(const auto down : {false, true}) {
        auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
        auto feature = new Feature_Button(button, down);
        node_fringe->feature = feature;
        auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(button, 2), feature->symbol_constant(), node_unsplit->action->parent());
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

  void Agent::generate_features() {
    std::list<Rete::WME_Ptr_C> wmes_current;
    std::ostringstream oss;

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_x_attr, std::make_shared<Rete::Symbol_Constant_Float>(m_current_state->getMarioFloatPos.first)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_y_attr, std::make_shared<Rete::Symbol_Constant_Float>(m_current_state->getMarioFloatPos.second)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_mode_attr, std::make_shared<Rete::Symbol_Constant_Int>(m_current_state->getMarioMode)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_on_ground_attr, m_current_state->isMarioOnGround ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_may_jump_attr, m_current_state->mayMarioJump ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_is_carrying_attr, m_current_state->isMarioCarrying ? m_true_value : m_false_value));

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_button_presses_in_attr, m_button_presses_in_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_button_presses_in_id, m_dpad_attr,
      std::make_shared<Rete::Symbol_Constant_Int>(m_current_state->action[BUTTON_DOWN] ? BUTTON_DOWN :
      m_current_state->action[BUTTON_LEFT] ^ m_current_state->action[BUTTON_RIGHT] ? (m_current_state->action[BUTTON_LEFT] ? BUTTON_LEFT : BUTTON_RIGHT) :
      BUTTON_NONE)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_button_presses_in_id, m_jump_attr, m_current_state->action[BUTTON_JUMP] ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_button_presses_in_id, m_speed_attr, m_current_state->action[BUTTON_SPEED] ? m_true_value : m_false_value));

    for(int i = 0; i != 16; ++i) {
      oss << "O" << i + 1;
      Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_button_presses_out_attr, action_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dpad_attr, std::make_shared<Rete::Symbol_Constant_Int>(
        (i & 0xC) == 0xC ? BUTTON_DOWN : i & 0x4 ? BUTTON_LEFT : i & 0x8 ? BUTTON_RIGHT : BUTTON_NONE)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_jump_attr, i & 0x2 ? m_true_value : m_false_value));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_speed_attr, i & 0x1 ? m_true_value : m_false_value));
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

#ifndef NDEBUG
    //static volatile bool test = true;
    //while(test) continue;
#endif

    insert_wme(m_wme_blink);
  }

  void Agent::update() {
    //auto env = dynamic_pointer_cast<const Environment>(get_env());

    //m_metastate = env->get_placements().empty() ? Metastate::FAILURE : Metastate::NON_TERMINAL;
  }

}

//int main(int argc, char **argv) {
//  try {
//    Carli::Experiment experiment;
//
//    experiment.take_args(argc, argv);
//
//    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();
//
//    experiment.standard_run([](){return std::make_shared<Tetris::Environment>();},
//                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Tetris::Agent>(env);},
//                            [&output](const std::shared_ptr<Carli::Agent> &){}
//                           );
//
//    return 0;
//  }
//  catch(std::exception &ex) {
//    std::cerr << "Exiting with exception: " << ex.what() << std::endl;
//  }
//  catch(...) {
//    std::cerr << "Exiting with unknown exception." << std::endl;
//  }
//
//  return -1;
//}
