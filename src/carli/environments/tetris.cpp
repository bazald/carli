#include "tetris.h"

namespace Tetris {

  Environment::Environment() {
    init_impl();
  }

  void Environment::init_impl() {
    m_random_selection = Zeni::Random(2147483647 * m_random_init.rand());

    memset(&m_grid, 0, sizeof(m_grid));

    m_next = Tetromino_Type(m_random_selection.rand_lt(7));
    m_current = Tetromino_Type(m_random_selection.rand_lt(7));

    generate_placements();
  }

  Environment::reward_type Environment::transition_impl(const Action &action) {
    const Place &place = debuggable_cast<const Place &>(action);

    const auto tet = generate_Tetronmino(m_current, place.orientation);
    for(size_t j = 0; j != 4; ++j) {
      for(size_t i = 0; i != 4; ++i) {
        if(tet[j][i])
          m_grid[j + place.position.second][i + place.position.first] = true;
      }
    }
    m_current = m_next;
    m_next = Tetromino_Type(m_random_selection.rand_lt(7));

    const double score = clear_lines();
    generate_placements();
    return score;
  }

  void Environment::print_impl(ostream &os) const {
  }

  Environment::Tetromino Environment::generate_Tetronmino(const Tetromino_Type &type, const int &orientation) {
    Environment::Tetromino tet;
    memset(&tet, 0, sizeof(tet));

    switch(type) {
    case TET_LINE:
      if(orientation & 1)
        tet[0][0] = tet[0][1] = tet[0][2] = tet[0][3] = true;
      else
        tet[0][0] = tet[1][0] = tet[2][0] = tet[3][0] = true;
      break;
    case TET_SQUARE:
      tet[0][0] = tet[0][1] = tet[1][0] = tet[1][1] = true;
      break;
    case TET_T:
      if(orientation & 2) {
        if(orientation & 1)
          tet[0][1] = tet[1][1] = tet[1][2] = tet[1][0] = true;
        else
          tet[1][0] = tet[1][1] = tet[2][1] = tet[0][1] = true;
      }
      else {
        if(orientation & 1)
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][1] = true;
        else
          tet[0][0] = tet[1][0] = tet[2][0] = tet[1][1] = true;
      }
      break;
    case TET_S:
      if(orientation & 1)
        tet[0][0] = tet[1][0] = tet[1][1] = tet[2][1] = true;
      else
        tet[1][0] = tet[1][1] = tet[0][1] = tet[0][2] = true;
      break;
    case TET_Z:
      if(orientation & 1)
        tet[0][0] = tet[0][1] = tet[1][1] = tet[1][2] = true;
      else
        tet[2][0] = tet[1][0] = tet[1][1] = tet[0][1] = true;
      break;
    case TET_L:
      if(orientation & 2) {
        if(orientation & 1)
          tet[0][2] = tet[1][2] = tet[1][1] = tet[1][0] = true;
        else
          tet[0][0] = tet[0][1] = tet[1][1] = tet[2][1] = true;
      }
      else {
        if(orientation & 1)
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][0] = true;
        else
          tet[0][0] = tet[1][0] = tet[2][0] = tet[2][1] = true;
      }
      break;
    case TET_J:
      if(orientation & 2) {
        if(orientation & 1)
          tet[0][0] = tet[1][2] = tet[1][1] = tet[1][0] = true;
        else
          tet[2][0] = tet[0][1] = tet[1][1] = tet[2][1] = true;
      }
      else {
        if(orientation & 1)
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][2] = true;
        else
          tet[0][0] = tet[1][0] = tet[2][0] = tet[0][1] = true;
      }
      break;
    default:
      abort();
    }

    return tet;
  }

  double Environment::clear_lines() {
    double score = 0.0;

    for(size_t j = 19; j < 20; ) {
      bool cleared = true;
      for(int i = 0; i != 10; ++i) {
        if(!m_grid[j][i]) {
          cleared = false;
          break;
        }
      }

      if(!cleared) {
        --j;
        continue;
      }

      memmove(&m_grid[1], &m_grid[0], sizeof(m_grid[0]) * j);
      memset(&m_grid[0], 0, sizeof(m_grid[0]));
    }

    return score;
  }

  Environment::Placement Environment::test_placement(const Environment::Tetromino &tet, const std::pair<size_t, size_t> &position) {
    const size_t width = width_Tetronmino(tet);
    const size_t height = height_Tetronmino(tet);

    if(position.first + width > 10 || position.second + height > 20)
      return PLACE_ILLEGAL;

    bool grounded = position.second + height == 20;

    for(int j = 0; j != 4; ++j) {
      for(int i = 0; i != 4; ++i) {
        if(tet[j][i]) {
          if(m_grid[position.second + j][position.first + i])
            return PLACE_ILLEGAL;
          else if(!grounded && m_grid[position.second + j + 1][position.first + i])
            grounded = true;
        }
      }
    }

    return grounded ? PLACE_GROUNDED : PLACE_UNGROUNDED;
  }

  size_t Environment::width_Tetronmino(const Environment::Tetromino &tet) {
    for(size_t i = 0; i != 4; ++i) {
      for(size_t j = 0; ; ++j) {
        if(j == 4)
          return i;
        else {
          if(tet[j][i])
            break;
        }
      }
    }

    return 4;
  }

  size_t Environment::height_Tetronmino(const Environment::Tetromino &tet) {
    for(size_t j = 0; j != 4; ++j) {
      for(size_t i = 0; ; ++i) {
        if(i == 4)
          return j;
        else {
          if(tet[j][i])
            break;
        }
      }
    }

    return 4;
  }

  void Environment::generate_placements() {
    m_placements.clear();

    for(int orientation = 0; orientation != 4; ++orientation) {
      const auto tet = generate_Tetronmino(m_current, orientation);
      const size_t width = width_Tetronmino(tet);

      for(size_t i = 0, iend = 11 - width; i != iend; ++i) {
        for(size_t j = 0; ; ++j) {
          if(j == 20 || test_placement(tet, std::make_pair(i, j)) == PLACE_ILLEGAL) {
            if(j)
              m_placements.emplace_back(std::make_pair(i, j - 1), orientation);
            break;
          }
        }
      }
    }
  }

  Agent::Agent(const std::shared_ptr< ::Environment> &env)
   : ::Agent(env)
  {
    insert_wme(m_wme_blink);
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
//    Rete::WME_Bindings state_bindings;
//
//    auto filter_action = make_filter(Rete::WME(m_first_var, m_action_attr, m_third_var));
////    state_bindings.clear();
//    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 0)));
//    auto filter_block = make_filter(Rete::WME(m_first_var, m_block_attr, m_third_var));
//    auto join_action_block = make_join(state_bindings, filter_action, filter_block);
//    auto filter_dest = make_filter(Rete::WME(m_first_var, m_dest_attr, m_third_var));
//    auto join_action_dest = make_join(state_bindings, join_action_block, filter_dest);
//
//    auto filter_name = make_filter(Rete::WME(m_first_var, m_name_attr, m_third_var));
//    state_bindings.clear();
//    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(1, 2), Rete::WME_Token_Index(0, 0)));
//    auto join_block_name = make_join(state_bindings, join_action_dest, filter_name);
//    state_bindings.clear();
//    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(2, 2), Rete::WME_Token_Index(0, 0)));
//    auto join_dest_name = make_join(state_bindings, join_block_name, filter_name);
//
//    auto filter_blink = make_filter(*m_wme_blink);
//    auto filter_clear = make_filter(Rete::WME(m_first_var, m_clear_attr, m_third_var));
//    auto filter_in_place = make_filter(Rete::WME(m_first_var, m_in_place_attr, m_third_var));
//
//    auto get_action = [this](const Rete::WME_Token &token)->action_ptrsc {
//      return std::make_shared<Move>(token);
//    };
//
//    auto node_unsplit = std::make_shared<Node_Unsplit>(*this, 1);
//    {
//      auto join_blink = make_existential_join(Rete::WME_Bindings(), join_dest_name, filter_blink);
//
//      node_unsplit->action = make_action_retraction([this,get_action,node_unsplit](const Rete::Rete_Action &rete_action, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        if(!this->specialize(rete_action, get_action, node_unsplit))
//          this->insert_q_value_next(action, node_unsplit->q_value);
//      }, [this,get_action,node_unsplit](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_unsplit->q_value);
//      }, join_blink).get();
//    }
//
//    std::vector<Feature::Which> blocks = {{Feature::BLOCK, Feature::DEST}};
//    const bool disable_distractors = true;
//
//    if(disable_distractors)
//      blocks = {Feature::BLOCK};
//
//    for(const auto &block : blocks) {
//      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
//      auto feature = new Clear(block, true);
//      node_fringe->feature = feature;
//      state_bindings.clear();
//      state_bindings.insert(Rete::WME_Binding(feature->wme_token_index(), Rete::WME_Token_Index(0, 0)));
//      auto join_block_clear = make_existential_join(state_bindings, join_dest_name, filter_clear);
//      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe->q_value);
//      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe->q_value);
//      }, join_block_clear).get();
//      node_unsplit->fringe_values.push_back(node_fringe);
//
//      auto node_fringe_neg = std::make_shared<Node_Fringe>(*this, 2);
//      node_fringe_neg->feature = new Clear(block, false);
//      auto neg = make_negation_join(state_bindings, join_dest_name, filter_clear);
//      node_fringe_neg->action = make_action_retraction([this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe_neg->q_value);
//      }, [this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe_neg->q_value);
//      }, neg).get();
//      node_unsplit->fringe_values.push_back(node_fringe_neg);
//    }
//
//    if(disable_distractors)
//      blocks = {Feature::DEST};
//
//    for(const auto &block : blocks) {
//      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
//      auto feature = new In_Place(block, true);
//      node_fringe->feature = feature;
//      state_bindings.clear();
//      state_bindings.insert(Rete::WME_Binding(feature->wme_token_index(), Rete::WME_Token_Index(0, 0)));
//      auto join_block_in_place = make_existential_join(state_bindings, join_dest_name, filter_in_place);
//      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe->q_value);
//      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe->q_value);
//      }, join_block_in_place).get();
//      node_unsplit->fringe_values.push_back(node_fringe);
//
//      auto node_fringe_neg = std::make_shared<Node_Fringe>(*this, 2);
//      node_fringe_neg->feature = new In_Place(block, false);
//      auto neg = make_negation_join(state_bindings, join_dest_name, filter_in_place);
//      node_fringe_neg->action = make_action_retraction([this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe_neg->q_value);
//      }, [this,get_action,node_fringe_neg](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe_neg->q_value);
//      }, neg).get();
//      node_unsplit->fringe_values.push_back(node_fringe_neg);
//    }
//
//    for(size_t block = 1; block != m_block_ids.size(); ++block) {
//      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
//      auto feature = new Name(Feature::BLOCK, m_block_names[block]->value);
//      node_fringe->feature = feature;
//      auto name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, feature->wme_token_index(), m_block_names[block], join_dest_name);
//      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe->q_value);
//      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe->q_value);
//      }, name_is).get();
//      node_unsplit->fringe_values.push_back(node_fringe);
//    }
//
//    for(size_t block = 0; block != m_block_ids.size(); ++block) {
//      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
//      auto feature = new Name(Feature::DEST, m_block_names[block]->value);
//      node_fringe->feature = feature;
//      auto name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, feature->wme_token_index(), m_block_names[block], join_dest_name);
//      node_fringe->action = make_action_retraction([this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->insert_q_value_next(action, node_fringe->q_value);
//      }, [this,get_action,node_fringe](const Rete::Rete_Action &, const Rete::WME_Token &token) {
//        const auto action = get_action(token);
//        this->purge_q_value_next(action, node_fringe->q_value);
//      }, name_is).get();
//      node_unsplit->fringe_values.push_back(node_fringe);
//    }
//
////    state_bindings.clear();
////    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(2, 2), Rete::WME_Token_Index(0, 0)));
////    auto join_block_clear = make_existential_join(state_bindings, join_dest_name, filter_clear);
////
////    state_bindings.clear();
////    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(3, 2), Rete::WME_Token_Index(0, 0)));
////    auto join_dest_in_place = make_existential_join(state_bindings, join_block_clear, filter_in_place);
////
////    for(int i = 1; i != 4; ++i) {
////      auto block_name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(4, 2), get_block_name(i), join_dest_in_place);
////
////      for(int j = 0; j != 4; ++j) {
////        auto dest_name_is = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(5, 2), get_block_name(j), block_name_is);
////        auto join_blink = make_existential_join(Rete::WME_Bindings(), dest_name_is, filter_blink);
////
////        auto rl = std::make_shared<Node_Split>(*this, new Q_Value(0.0, Q_Value::Type::SPLIT, 1));
////        rl->action = make_action_retraction([this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
////          const auto action = get_action(token);
////          this->insert_q_value_next(action, rl->q_value);
////        }, [this,get_action,rl](const Rete::Rete_Action &, const Rete::WME_Token &token) {
////          const auto action = get_action(token);
////          this->purge_q_value_next(action, rl->q_value);
////        }, join_blink).get();
////      }
////    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    std::list<Rete::WME_Ptr_C> wmes_current;
    std::ostringstream oss;

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_input_attr, m_input_id));

    size_t index = 0;
    for(const auto &placement : env->get_placements()) {
      oss << "place-" << ++index;
      Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_input_id, m_action_attr, action_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_x_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.first.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_y_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.first.second)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_orientation_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.second)));
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
