#include "infinite_mario.h"

#include "jni_layer.h"
#include "carli/parser/rete_parser.h"

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
      generate_all_features();

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
          if(sum_value(nullptr, m_current_q_value, nullptr) < sum_value(nullptr, m_next_q_values[next], nullptr))
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
   : Carli::Agent(current_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Button_Presses>(variables, token);}),
   m_current_state(current_),
   m_prev_state(prev_)
  {
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
    std::string rules = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
    if(rules == "default")
      rules = "../../rules/infinite-mario.carli";
    if(rete_parse_file(*this, rules))
      abort();
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
    //auto env = dynamic_pointer_cast<const Environment>(get_env());

    //m_metastate = env->get_placements().empty() ? Metastate::FAILURE : Metastate::NON_TERMINAL;
  }

}
