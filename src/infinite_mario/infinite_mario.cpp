#include "infinite_mario.h"

#include "jni_layer.h"

namespace Mario {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  //void infinite_mario_ai(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, Action &action) {
  //  action[BUTTON_LEFT] = 0;
  //  action[BUTTON_RIGHT] = 1;
  //  action[BUTTON_DOWN] = 0;

  ////   for(int j = 0; j != OBSERVATION_HEIGHT; ++j) {
  ////     for(int i = 0; i != OBSERVATION_WIDTH; ++i) {
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

  static Agent & get_Agent(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current) {
    static Agent g_Agent(prev, current);
    return g_Agent;
  }

  static bool g_infinite_mario_ai_initialized = false;
  static size_t g_steps_passed = 0;

  void infinite_mario_ai(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, Action &action) {
    Agent &agent = get_Agent(prev, current);

    if(g_infinite_mario_ai_initialized)
      agent.act_part_2(prev, current, false);
    else if(++g_steps_passed >= 24) {
      g_infinite_mario_ai_initialized = true;
      agent.init();
    }

    if(g_infinite_mario_ai_initialized)
      agent.act_part_1(action);
  }

  void infinite_mario_reinit(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current) {
    Agent &agent = get_Agent(prev, current);

    if(g_infinite_mario_ai_initialized)
      agent.act_part_2(prev, current, true);

    g_infinite_mario_ai_initialized = false;
    g_steps_passed = 0;
  }

  bool tile_can_jump_into(const Tile &tile) {
    switch(tile) {
    case TILE_BRICK:
    case TILE_QUESTION:
      return true;
    default:
      return false;
    }
  }

  bool tile_can_jump_through(const Tile &tile) {
    switch(tile) {
    case TILE_IRRELEVANT:
    case TILE_HALF_BORDER:
      return true;
    default:
      return false;
    }
  }

  bool tile_can_land_on(const Tile &tile) {
    switch(tile) {
    case TILE_SOMETHING:
    case TILE_BORDER:
    case TILE_HALF_BORDER:
    case TILE_BRICK:
    case TILE_POT_OR_CANNON:
    case TILE_QUESTION:
      return true;
    default:
      return false;
    }
  }

  bool tile_can_pass_through(const Tile &tile) {
    switch(tile) {
    case TILE_IRRELEVANT:
    case TILE_HALF_BORDER:
      return true;
    default:
      return false;
    }
  }

  bool object_dangerous(const Object &object) {
    switch(object) {
    case OBJECT_GOOMBA:
    case OBJECT_GOOMBA_WINGED:
    case OBJECT_RED_KOOPA:
    case OBJECT_RED_KOOPA_WINGED:
    case OBJECT_GREEN_KOOPA:
    case OBJECT_GREEN_KOOPA_WINGED:
    case OBJECT_BULLET_BILL:
    case OBJECT_SPIKY:
    case OBJECT_SPIKY_WINGED:
    case OBJECT_ENEMY_FLOWER:
    case OBJECT_SHELL:
      return true;
    default:
      return false;
    }
  }

  bool object_flies(const Object &object) {
    switch(object) {
    case OBJECT_GOOMBA_WINGED:
    case OBJECT_RED_KOOPA_WINGED:
    case OBJECT_GREEN_KOOPA_WINGED:
    case OBJECT_BULLET_BILL:
    case OBJECT_SPIKY_WINGED:
    case OBJECT_ENEMY_FLOWER:
      return true;
    default:
      return false;
    }
  }

  bool object_powerup(const Object &object) {
    switch(object) {
    case OBJECT_MUSHROOM:
    case OBJECT_FIRE_FLOWER:
      return true;
    default:
      return false;
    }
  }

  bool object_killable_by_fireball(const Object &object) {
    switch(object) {
    case OBJECT_GOOMBA:
    case OBJECT_GOOMBA_WINGED:
    case OBJECT_RED_KOOPA:
    case OBJECT_RED_KOOPA_WINGED:
    case OBJECT_GREEN_KOOPA:
    case OBJECT_GREEN_KOOPA_WINGED:
    case OBJECT_ENEMY_FLOWER:
      return true;
    default:
      return false;
    }
  }

  bool object_killable_by_jump(const Object &object) {
    switch(object) {
    case OBJECT_GOOMBA:
    case OBJECT_GOOMBA_WINGED:
    case OBJECT_RED_KOOPA:
    case OBJECT_RED_KOOPA_WINGED:
    case OBJECT_GREEN_KOOPA:
    case OBJECT_GREEN_KOOPA_WINGED:
    case OBJECT_BULLET_BILL:
      return true;
    default:
      return false;
    }
  }

  void Agent::act_part_1(Action &action) {
    m_current = m_next;
    m_current_q_value = m_next_q_values[m_next];
    m_current_q_value.sort([](const tracked_ptr<Q_Value> &lhs, const tracked_ptr<Q_Value> &rhs)->bool{return lhs->depth < rhs->depth;});

    assert(m_current);

    action = debuggable_cast<const Button_Presses &>(*m_next).action;

#if defined(_WINDOWS) && defined(NDEBUG)
    //system("cls");
#endif
    /** Render scene **/
    //std::cerr << "Observation:" << std::endl;
    //for(int j = 0; j != OBSERVATION_HEIGHT; ++j) {
    //  for(int i = 0; i != OBSERVATION_WIDTH; ++i) {
    //    if(m_current_state->getEnemiesObservation[j][i])
    //      std::cerr << m_current_state->getEnemiesObservation[j][i];
    //    else
    //      std::cerr << m_current_state->getLevelSceneObservation[j][i].tile;
    //  }
    //  std::cerr << std::endl;
    //}
    //std::cerr << std::endl;

    /** Render pits **/
    //std::cerr << "Observation:" << std::endl;
    //for(int j = 0; j != OBSERVATION_HEIGHT; ++j) {
    //  for(int i = 0; i != OBSERVATION_WIDTH; ++i) {
    //    if(m_current_state->getLevelSceneObservation[j][i].detail.pit)
    //      std::cerr << ' ';
    //    else if(m_current_state->getLevelSceneObservation[j][i].detail.above_pit)
    //      std::cerr << '.';
    //    else
    //      std::cerr << '#';
    //  }
    //  std::cerr << std::endl;
    //}
    //std::cerr << std::endl;
  }

  void Agent::act_part_2(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, const bool &terminal) {
    double delta_x = current->getMarioFloatPos.first - prev->getMarioFloatPos.first;
    if(prev->action[BUTTON_RIGHT])
      delta_x = std::max(delta_x, 0.0);
    else if(prev->action[BUTTON_LEFT])
      delta_x = std::min(delta_x, 0.0);

    //const bool continued_high_jump = prev->isMarioHighJumping && prev->action[BUTTON_JUMP];
    //const bool stopped_high_jump = prev->isMarioHighJumping && !prev->action[BUTTON_JUMP];
    //const bool legal_jump = prev->mayMarioJump && prev->action[BUTTON_JUMP];
    //const bool illegal_jump = !prev->mayMarioJump && prev->action[BUTTON_JUMP];
    //const reward_type reward = (delta_x > 0 ? 1 : 2) * delta_x;

    //const reward_type reward_jumping = 0; // legal_jump ? 30 : continued_high_jump ? 30 : stopped_high_jump ? -50 : 0;
    const reward_type reward =
      m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2 - 1].detail.pit
        ? (m_current_state->getMarioFloatPos.second < m_prev_state->getMarioFloatPos.second ? 10.0 : -10000.0)
        : 100.0 * delta_x - 110.0;

    std::cerr << "Reward = " << reward << std::endl;

#ifndef NO_COLLAPSE_DETECTION_HACK
    if(reward > 0.0) {
      if(++m_positive_rewards_in_a_row > 30)
        m_experienced_n_positive_rewards_in_a_row = true;
    }
    else
      m_positive_rewards_in_a_row = 0;
#endif

    update();

    if(terminal)
      td_update(m_current_q_value, reward, Q_Value_List());
    else {
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
        Carli::Action_Ptr_C next = m_exploration_policy();

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

    //m_total_reward += reward;
    //++m_step_count;
  }

  Agent::Agent(const std::shared_ptr<State> &prev_, const std::shared_ptr<State> &current_)
   : Carli::Agent(current_), m_current_state(current_), m_prev_state(prev_)
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
                                       const AXIS &axis,
                                       const double &lower_bound,
                                       const double &upper_bound)
  {
    const double midpt = floor((lower_bound + upper_bound) / 2.0);
    const double values[][2] = {{lower_bound, midpt},
                                {midpt, upper_bound}};
    for(int i = 0; i != 2; ++i) {
      auto feature = new SUBFEATURE(axis, values[i][0], values[i][1], 2, i != 0);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(axis, 2), feature->symbol_constant(), node_unsplit->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("infinite-mario*rl-action*f"), node_unsplit, feature); //, Node_Ranged::Range(/*std::make_pair(0, 0), std::make_pair(5, 20)*/), lines);
    }
  }

  void Agent::generate_rete() {
    Rete::WME_Bindings state_bindings;

    auto filter_x = make_filter(Rete::WME(m_first_var, m_x_attr, m_third_var));
    auto filter_y = make_filter(Rete::WME(m_first_var, m_y_attr, m_third_var));
    auto filter_x_dot = make_filter(Rete::WME(m_first_var, m_x_dot_attr, m_third_var));
    auto filter_y_dot = make_filter(Rete::WME(m_first_var, m_y_dot_attr, m_third_var));
    auto filter_mode = make_filter(Rete::WME(m_first_var, m_mode_attr, m_third_var));
    auto filter_on_ground = make_filter(Rete::WME(m_first_var, m_on_ground_attr, m_third_var));
    auto filter_may_jump = make_filter(Rete::WME(m_first_var, m_may_jump_attr, m_third_var));
    auto filter_is_carrying = make_filter(Rete::WME(m_first_var, m_is_carrying_attr, m_third_var));
    auto filter_is_high_jumping = make_filter(Rete::WME(m_first_var, m_is_high_jumping_attr, m_third_var));
    auto filter_is_above_pit = make_filter(Rete::WME(m_first_var, m_is_above_pit_attr, m_third_var));
    auto filter_is_in_pit = make_filter(Rete::WME(m_first_var, m_is_in_pit_attr, m_third_var));
    auto filter_pit_right = make_filter(Rete::WME(m_first_var, m_pit_right_attr, m_third_var));
    auto filter_obstacle_right = make_filter(Rete::WME(m_first_var, m_obstacle_right_attr, m_third_var));
    auto filter_dpad = make_filter(Rete::WME(m_first_var, m_dpad_attr, m_third_var));
    auto filter_jump = make_filter(Rete::WME(m_first_var, m_jump_attr, m_third_var));
    auto filter_speed = make_filter(Rete::WME(m_first_var, m_speed_attr, m_third_var));
    auto filter_type = make_filter(Rete::WME(m_first_var, m_type_attr, m_third_var));
    auto filter_right_pit_dist = make_filter(Rete::WME(m_first_var, m_right_pit_dist_attr, m_third_var));
    auto filter_right_pit_width = make_filter(Rete::WME(m_first_var, m_right_pit_width_attr, m_third_var));
    auto filter_right_jump_dist = make_filter(Rete::WME(m_first_var, m_right_jump_dist_attr, m_third_var));
    auto filter_right_jump_height = make_filter(Rete::WME(m_first_var, m_right_jump_height_attr, m_third_var));

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

    //auto filter_enemy = make_filter(Rete::WME(m_first_var, m_enemy_attr, m_third_var));
    //auto join_enemy_type = make_join(state_bindings, filter_enemy, filter_type);
    //auto join_enemy_x = make_join(state_bindings, join_enemy_type, filter_x);
    //auto join_enemy_y = make_join(state_bindings, join_enemy_x, filter_y);

    state_bindings.clear();
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));

    auto join_state_x_y = make_join(state_bindings, filter_x, filter_y);
    auto join_state_x_dot = make_join(state_bindings, join_state_x_y, filter_x_dot);
    auto join_state_y_dot = make_join(state_bindings, join_state_x_dot, filter_y_dot);
    auto join_state_mode = make_join(state_bindings, join_state_y_dot, filter_mode);
    auto join_state_on_ground = make_join(state_bindings, join_state_mode, filter_on_ground);
    auto join_state_may_jump = make_join(state_bindings, join_state_on_ground, filter_may_jump);
    auto join_state_is_carrying = make_join(state_bindings, join_state_may_jump, filter_is_carrying);
    auto join_state_is_high_jumping = make_join(state_bindings, join_state_is_carrying, filter_is_high_jumping);
    auto join_state_is_above_pit = make_join(state_bindings, join_state_is_high_jumping, filter_is_above_pit);
    auto join_state_is_in_pit = make_join(state_bindings, join_state_is_above_pit, filter_is_in_pit);
    auto join_state_pit_right = make_join(state_bindings, join_state_is_in_pit, filter_pit_right);
    auto join_state_obstacle_right = make_join(state_bindings, join_state_pit_right, filter_obstacle_right);
    auto join_state_right_pit_dist = make_join(state_bindings, join_state_obstacle_right, filter_right_pit_dist);
    auto join_state_right_pit_width = make_join(state_bindings, join_state_right_pit_dist, filter_right_pit_width);
    auto join_state_right_jump_dist = make_join(state_bindings, join_state_right_pit_width, filter_right_jump_dist);
    auto join_state_right_jump_height = make_join(state_bindings, join_state_right_jump_dist, filter_right_jump_height);

    auto join_complete_state = make_join(state_bindings, join_state_right_jump_height, join_button_presses_in_speed);

    auto join_last = make_join(state_bindings, join_complete_state, join_button_presses_out_speed);
    //auto &join_enemy_last = join_enemy_y;

    auto filter_blink = make_filter(*m_wme_blink);

    auto get_action = [this](const Rete::WME_Token &token)->Carli::Action_Ptr_C {
      return std::make_shared<Button_Presses>(token);
    };

    Carli::Node_Unsplit_Ptr root_action_data;
    {
      auto join_blink = make_existential_join(Rete::WME_Bindings(), join_last, filter_blink);

      auto root_action = make_standard_action(join_blink, "infinite-mario*rl-action*general");
      root_action_data = std::make_shared<Node_Unsplit>(*this, root_action, get_action, 1, nullptr);
      root_action->data = root_action_data;
    }

    /*** State ***/

    for(const auto flag : {/*Feature_Flag::ON_GROUND, Feature_Flag::MAY_JUMP, Feature_Flag::IS_CARRYING,*/
                           /*Feature_Flag::IS_HIGH_JUMPING,*/ Feature_Flag::IS_ABOVE_PIT, Feature_Flag::IS_IN_PIT,
                           /*Feature_Flag::PIT_RIGHT,*/ Feature_Flag::OBSTACLE_RIGHT}) {
      for(const auto value : {false, true}) {
        auto feature = new Feature_Flag(flag, value);
        auto predicate = make_predicate_vc(feature->predicate(), feature->wme_token_index(), feature->symbol_constant(), join_last);
        make_standard_fringe(predicate, next_rule_name("infinite-mario*rl-action*f"), root_action_data, feature);
      }
    }

    //generate_rete_continuous<Feature_Position, Feature_Position::Axis>(root_action_data, Feature_Position::X, 0.0f, 4000.0f);
    //generate_rete_continuous<Feature_Position, Feature_Position::Axis>(root_action_data, Feature_Position::Y, 0.0f, 352.0f);

    generate_rete_continuous<Feature_Numeric, Feature_Numeric::Axis>(root_action_data, Feature_Numeric::RIGHT_PIT_DIST, 0.0f, 12.0f);
    //generate_rete_continuous<Feature_Numeric, Feature_Numeric::Axis>(root_action_data, Feature_Numeric::RIGHT_PIT_WIDTH, 0.0f, 12.0f);
    //generate_rete_continuous<Feature_Numeric, Feature_Numeric::Axis>(root_action_data, Feature_Numeric::RIGHT_JUMP_DIST, 0.0f, 12.0f);
    //generate_rete_continuous<Feature_Numeric, Feature_Numeric::Axis>(root_action_data, Feature_Numeric::RIGHT_JUMP_HEIGHT, 0.0f, 12.0f);

    /*** Output Buttons ***/

    for(const auto dpad : {BUTTON_NONE, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_DOWN}) {
      auto feature = new Feature_Button(Feature_Button::OUT_DPAD, dpad);
      auto predicate = make_predicate_vc(feature->predicate(), feature->wme_token_index(), feature->symbol_constant(), join_last);
      make_standard_fringe(predicate, next_rule_name("infinite-mario*rl-action*f"), root_action_data, feature);
    }

    for(const auto button : {Feature_Button::OUT_JUMP, Feature_Button::OUT_SPEED}) {
      for(const auto down : {false, true}) {
        auto feature = new Feature_Button(button, down);
        auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(button, 2), feature->symbol_constant(), join_last);
        make_standard_fringe(predicate, next_rule_name("infinite-mario*rl-action*f"), root_action_data, feature);
      }
    }
  }

  void Agent::generate_features() {
    std::list<Rete::WME_Ptr_C> wmes_current;
    std::ostringstream oss;

    const std::pair<double, double> velocity(m_current_state->getMarioFloatPos.first - m_prev_state->getMarioFloatPos.first,
                                             m_current_state->getMarioFloatPos.second - m_prev_state->getMarioFloatPos.second);

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_x_attr, std::make_shared<Rete::Symbol_Constant_Float>(m_current_state->getMarioFloatPos.first)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_y_attr, std::make_shared<Rete::Symbol_Constant_Float>(m_current_state->getMarioFloatPos.second)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_x_dot_attr, std::make_shared<Rete::Symbol_Constant_Float>(velocity.first)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_y_dot_attr, std::make_shared<Rete::Symbol_Constant_Float>(velocity.second)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_mode_attr, std::make_shared<Rete::Symbol_Constant_Int>(m_current_state->getMarioMode)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_on_ground_attr, m_current_state->isMarioOnGround ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_may_jump_attr, m_current_state->mayMarioJump ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_is_carrying_attr, m_current_state->isMarioCarrying ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_is_high_jumping_attr, m_current_state->isMarioHighJumping ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_is_above_pit_attr, m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2 - 1].detail.above_pit ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_is_in_pit_attr, m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2 - 1].detail.pit ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_pit_right_attr, m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2].detail.pit ||
                                                                                 m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2 + 1].detail.pit ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_obstacle_right_attr, !tile_can_pass_through(m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2].tile) ? m_true_value : m_false_value));

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_button_presses_in_attr, m_button_presses_in_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_button_presses_in_id, m_dpad_attr,
      std::make_shared<Rete::Symbol_Constant_Int>(m_current_state->action[BUTTON_DOWN] ? BUTTON_DOWN :
      m_current_state->action[BUTTON_LEFT] ^ m_current_state->action[BUTTON_RIGHT] ? (m_current_state->action[BUTTON_LEFT] ? BUTTON_LEFT : BUTTON_RIGHT) :
      BUTTON_NONE)));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_button_presses_in_id, m_jump_attr, m_current_state->action[BUTTON_JUMP] ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_button_presses_in_id, m_speed_attr, m_current_state->action[BUTTON_SPEED] ? m_true_value : m_false_value));

    {
      int dist = 0;
      int width = 0;

      while(dist != OBSERVATION_WIDTH / 2) {
        if(!m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2 + dist].detail.above_pit) {
          ++dist;
          continue;
        }

        width = 1;
        while(dist + width != OBSERVATION_WIDTH / 2) {
          if(!m_current_state->getLevelSceneObservation[OBSERVATION_HEIGHT / 2][OBSERVATION_WIDTH / 2 + dist + width].detail.above_pit)
            break;
          ++width;
        }
        break;
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_right_pit_dist_attr, std::make_shared<Rete::Symbol_Constant_Float>(dist)));
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_right_pit_width_attr, std::make_shared<Rete::Symbol_Constant_Float>(width)));
    }

    {
      int dist = 0;
      int j = OBSERVATION_HEIGHT / 2;

      while(dist != OBSERVATION_WIDTH / 2) {
        if(tile_can_pass_through(m_current_state->getLevelSceneObservation[j][OBSERVATION_WIDTH / 2 + dist].tile)) {
          ++dist;
          continue;
        }

        while(j != -1) {
          if(tile_can_pass_through(m_current_state->getLevelSceneObservation[j][OBSERVATION_WIDTH / 2 + dist].tile))
            break;
          --j;
        }
        break;
      }

      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_right_jump_dist_attr, std::make_shared<Rete::Symbol_Constant_Float>(dist)));
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_right_jump_height_attr, std::make_shared<Rete::Symbol_Constant_Float>(OBSERVATION_HEIGHT / 2 - j)));
    }

    for(int i = 0; i != 16; ++i) {
      if(i & 0x2 && !m_current_state->mayMarioJump && !m_current_state->isMarioHighJumping)
        continue; ///< Cannot jump, not high jumping
      if(!(i & 0x2) && m_current_state->isMarioHighJumping)
        continue; ///< Force high jumping
      oss << "O" << i + 1;
      Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_button_presses_out_attr, action_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_dpad_attr, std::make_shared<Rete::Symbol_Constant_Int>(
        (i & 0xC) == 0xC ? BUTTON_DOWN : i & 0x4 ? BUTTON_LEFT : i & 0x8 ? BUTTON_RIGHT : BUTTON_NONE)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_jump_attr, i & 0x2 ? m_true_value : m_false_value));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_speed_attr, i & 0x1 ? m_true_value : m_false_value));
    }

    int i = 0;
    for(const auto &enemy : m_current_state->getEnemiesFloatPos) {
      oss << "E" << ++i;
      Rete::Symbol_Identifier_Ptr_C enemy_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_enemy_attr, enemy_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(enemy_id, m_type_attr, std::make_shared<Rete::Symbol_Constant_Int>(enemy.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(enemy_id, m_x_attr, std::make_shared<Rete::Symbol_Constant_Float>(enemy.second.first - m_current_state->getMarioFloatPos.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(enemy_id, m_y_attr, std::make_shared<Rete::Symbol_Constant_Float>(enemy.second.second - m_current_state->getMarioFloatPos.second)));
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
