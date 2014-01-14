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
          m_grid[place.position.second - j][place.position.first + i] = true;
      }
    }
    m_current = m_next;
    m_next = Tetromino_Type(m_random_selection.rand_lt(7));

    const double score = clear_lines();

    generate_placements();

    if(m_placements.empty())
      return score_failure;
    else
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
    size_t lines_cleared = 0.0;

    for(size_t j = 0; j < 20; ) {
      bool cleared = true;
      for(int i = 0; i != 10; ++i) {
        if(!m_grid[j][i]) {
          cleared = false;
          break;
        }
      }

      if(!cleared) {
        ++j;
        continue;
      }

      memmove(&m_grid[j], &m_grid[j + 1], (19 - j) * sizeof(m_grid[0]));
      memset(&m_grid[19], 0, sizeof(m_grid[0]));
      ++lines_cleared;
    }

    return score_line[lines_cleared];
  }

  Environment::Result Environment::test_placement(const Environment::Tetromino &tet, const std::pair<size_t, size_t> &position) {
    const uint8_t width = width_Tetronmino(tet);
    const uint8_t height = height_Tetronmino(tet);

    if(position.first + width > 10 || int(position.second - height) < -1)
      return PLACE_ILLEGAL;

    bool grounded = int(position.second - height) == -1;

    for(int j = 0; j != 4; ++j) {
      for(int i = 0; i != 4; ++i) {
        if(tet[j][i]) {
          if(m_grid[position.second - j][position.first + i])
            return PLACE_ILLEGAL;
          else if(!grounded && m_grid[position.second - j + 1][position.first + i])
            grounded = true;
        }
      }
    }

    return grounded ? PLACE_GROUNDED : PLACE_UNGROUNDED;
  }

  uint8_t Environment::width_Tetronmino(const Environment::Tetromino &tet) {
    for(uint8_t i = 0; i != 4; ++i) {
      for(uint8_t j = 0; ; ++j) {
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

  uint8_t Environment::height_Tetronmino(const Environment::Tetromino &tet) {
    for(uint8_t j = 0; j != 4; ++j) {
      for(uint8_t i = 0; ; ++i) {
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
    const uint8_t orientations = orientations_Tetronmino(m_current);

    m_placements.clear();

    for(int orientation = 0; orientation != orientations; ++orientation) {
      const auto tet = generate_Tetronmino(m_current, orientation);
      const size_t width = width_Tetronmino(tet);
      const size_t height = height_Tetronmino(tet);

      for(size_t i = 0, iend = 11 - width; i != iend; ++i) {
        for(size_t j = 19; int(j) > -1; --j) {
          if(test_placement(tet, std::make_pair(i, j)) == PLACE_ILLEGAL) {
            if(j < 19)
              m_placements.emplace_back(orientation, std::make_pair(width, height), std::make_pair(i, j + 1));
            break;
          }
        }
      }
    }
  }

  uint8_t Environment::orientations_Tetronmino(const Tetromino_Type &type) {
    switch(type) {
    case TET_SQUARE:
      return 1;
    case TET_LINE:
    case TET_S:
    case TET_Z:
      return 2;
    case TET_T:
    case TET_L:
    case TET_J:
      return 4;
    default:
      abort();
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
    Rete::WME_Bindings state_bindings;

//    state_bindings.clear();
    auto filter_action = make_filter(Rete::WME(m_first_var, m_action_attr, m_third_var));
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 2), Rete::WME_Token_Index(0, 0)));
    auto filter_type_current = make_filter(Rete::WME(m_first_var, m_type_current_attr, m_third_var));
    auto join_type_current = make_join(state_bindings, filter_action, filter_type_current);
    auto filter_type_next = make_filter(Rete::WME(m_first_var, m_type_next_attr, m_third_var));
    auto join_type_next = make_join(state_bindings, join_type_current, filter_type_next);
    auto filter_orientation = make_filter(Rete::WME(m_first_var, m_orientation_attr, m_third_var));
    auto join_orientation = make_join(state_bindings, join_type_next, filter_orientation);
    auto filter_width = make_filter(Rete::WME(m_first_var, m_width_attr, m_third_var));
    auto join_width = make_join(state_bindings, join_orientation, filter_width);
    auto filter_height = make_filter(Rete::WME(m_first_var, m_height_attr, m_third_var));
    auto join_height = make_join(state_bindings, join_width, filter_height);
    auto filter_x = make_filter(Rete::WME(m_first_var, m_x_attr, m_third_var));
    auto join_x = make_join(state_bindings, join_height, filter_x);
    auto filter_y = make_filter(Rete::WME(m_first_var, m_y_attr, m_third_var));
    auto join_y = make_join(state_bindings, join_x, filter_y);
    auto &join_last = join_y;

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

    {
      Node_Ranged::Lines lines;
      lines.push_back(Node_Ranged::Line(std::make_pair(5, 0), std::make_pair(5, 20)));
      auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                      Node_Ranged::Range(std::make_pair(0, 0), std::make_pair(5, 20)),
                                                      lines);
      auto feature = new Position(Position::X, 0, 5, 2, false);
      nfr->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::X, 2), feature->symbol_constant(), node_unsplit->action->parent());
      nfr->action = make_action_retraction([this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, nfr->q_value);
      }, [this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, nfr->q_value);
      }, predicate).get();
      node_unsplit->fringe_values.push_back(nfr);
    }

    {
      auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                      Node_Ranged::Range(std::make_pair(5, 0), std::make_pair(10, 20)),
                                                      Node_Ranged::Lines());
      auto feature = new Position(Position::X, 5, 10, 2, true);
      nfr->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::X, 2), feature->symbol_constant(), node_unsplit->action->parent());
      nfr->action = make_action_retraction([this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, nfr->q_value);
      }, [this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, nfr->q_value);
      }, predicate).get();
      node_unsplit->fringe_values.push_back(nfr);
    }

    {
      Node_Ranged::Lines lines;
      lines.push_back(Node_Ranged::Line(std::make_pair(0, 10), std::make_pair(10, 10)));
      auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                      Node_Ranged::Range(std::make_pair(0, 0), std::make_pair(10, 10)),
                                                      lines);
      auto feature = new Position(Position::Y, 0, 10, 2, false);
      nfr->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::Y, 2), feature->symbol_constant(), node_unsplit->action->parent());
      nfr->action = make_action_retraction([this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, nfr->q_value);
      }, [this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, nfr->q_value);
      }, predicate).get();
      node_unsplit->fringe_values.push_back(nfr);
    }

    {
      auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                      Node_Ranged::Range(std::make_pair(0, 10), std::make_pair(10, 20)),
                                                      Node_Ranged::Lines());
      auto feature = new Position(Position::Y, 10, 20, 2, true);
      nfr->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::Y, 2), feature->symbol_constant(), node_unsplit->action->parent());
      nfr->action = make_action_retraction([this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->insert_q_value_next(action, nfr->q_value);
      }, [this,get_action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &token) {
        const auto action = get_action(token);
        this->purge_q_value_next(action, nfr->q_value);
      }, predicate).get();
      node_unsplit->fringe_values.push_back(nfr);
    }

    for(size_t orientation = 0; orientation != 4; ++orientation) {
      auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
      auto feature = new Orientation(orientation);
      node_fringe->feature = feature;
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Orientation::ORIENTATION, 2), feature->symbol_constant(), node_unsplit->action->parent());
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
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_type_current_attr, std::make_shared<Rete::Symbol_Constant_Int>(env->get_current())));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_type_next_attr, std::make_shared<Rete::Symbol_Constant_Int>(env->get_next())));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_orientation_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.orientation)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_width_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.size.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_height_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.size.second)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_x_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.position.first)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_y_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.position.second)));
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
