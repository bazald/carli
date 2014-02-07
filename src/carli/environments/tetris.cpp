#include "tetris.h"

namespace Tetris {

  Environment::Environment() {
    init_impl();
  }

  Environment Environment::operator=(const Environment &rhs) {
    ::Environment::operator=(rhs);

    m_random_init = rhs.m_random_init;
    m_random_selection = rhs.m_random_selection;
    m_grid = rhs.m_grid;
    m_current = rhs.m_current;
    m_next = rhs.m_next;
    m_placements = rhs.m_placements;
    m_lookahead = rhs.m_lookahead;

    return *this;
  }

  void Environment::init_impl() {
    m_random_selection = Zeni::Random(2147483647 * m_random_init.rand());

    memset(&m_grid, 0, sizeof(m_grid));
    for(auto &line : m_grid)
      line.second = line.first.size();

    m_next = Tetromino_Type(m_random_selection.rand_lt(7));
    m_current = Tetromino_Type(m_random_selection.rand_lt(7));

    generate_placements();
  }

  Environment::reward_type Environment::transition_impl(const Action &action) {
    const Place &place = debuggable_cast<const Place &>(action);

    const auto tet = generate_Tetromino(m_current, place.orientation);
    place_Tetromino(tet, place.position);

    m_current = m_next;
    m_next = Tetromino_Type(m_random_selection.rand_lt(7));

    const double score = score_line[clear_lines(place.position)];

    generate_placements();

    if(m_placements.empty())
      return score_failure;
    else
      return score;
  }

  void Environment::place_Tetromino(const Environment::Tetromino &tet, const std::pair<size_t, size_t> &position) {
    for(size_t j = 0; j != tet.height; ++j) {
      for(size_t i = 0; i != tet.width; ++i) {
        if(tet[j][i]) {
          m_grid[position.second - j].first[position.first + i] = true;
          --m_grid[position.second - j].second;
        }
      }
    }
  }

  void Environment::print_impl(ostream &os) const {
    os << "Board:" << std::endl;
    for(int j = 19; j != -1; --j) {
      os << "  ";
      for(int i = 0; i != 10; ++i)
        os << (m_grid[j].first[i] ? 'O' : '.');
      os << std::endl;
    }

    os << "Current:" << std::endl;
    const auto current = generate_Tetromino(m_current);
    for(int j = 0; j != current.height; ++j) {
      os << "  ";
      for(int i = 0; i != current.width; ++i)
        os << (current[j][i] ? 'O' : ' ');
      os << std::endl;
    }

    os << "Next:" << std::endl;
    const auto next = generate_Tetromino(m_next);
    for(int j = 0; j != next.height; ++j) {
      os << "  ";
      for(int i = 0; i != next.width; ++i)
        os << (next[j][i] ? 'O' : ' ');
      os << std::endl;
    }
  }

  Environment::Tetromino Environment::generate_Tetromino(const Tetromino_Type &type, const int &orientation) {
    Environment::Tetromino tet;
    memset(&tet, 0, sizeof(tet));

    switch(type) {
    case TET_LINE:
      if(orientation & 1) {
        tet[0][0] = tet[0][1] = tet[0][2] = tet[0][3] = true;
        tet.width = 4;
        tet.height = 1;
      }
      else {
        tet[0][0] = tet[1][0] = tet[2][0] = tet[3][0] = true;
        tet.width = 1;
        tet.height = 4;
      }
      break;
    case TET_SQUARE:
      tet[0][0] = tet[0][1] = tet[1][0] = tet[1][1] = true;
      tet.width = 2;
      tet.height = 2;
      break;
    case TET_T:
      if(orientation & 2) {
        if(orientation & 1) {
          tet[0][1] = tet[1][1] = tet[1][2] = tet[1][0] = true;
          tet.width = 3;
          tet.height = 2;
        }
        else {
          tet[1][0] = tet[1][1] = tet[2][1] = tet[0][1] = true;
          tet.width = 2;
          tet.height = 3;
        }
      }
      else {
        if(orientation & 1) {
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][1] = true;
          tet.width = 3;
          tet.height = 2;
        }
        else {
          tet[0][0] = tet[1][0] = tet[2][0] = tet[1][1] = true;
          tet.width = 2;
          tet.height = 3;
        }
      }
      break;
    case TET_S:
      if(orientation & 1) {
        tet[0][0] = tet[1][0] = tet[1][1] = tet[2][1] = true;
        tet.width = 2;
        tet.height = 3;
      }
      else {
        tet[1][0] = tet[1][1] = tet[0][1] = tet[0][2] = true;
        tet.width = 3;
        tet.height = 2;
      }
      break;
    case TET_Z:
      if(orientation & 1) {
        tet[0][0] = tet[0][1] = tet[1][1] = tet[1][2] = true;
        tet.width = 3;
        tet.height = 2;
      }
      else {
        tet[2][0] = tet[1][0] = tet[1][1] = tet[0][1] = true;
        tet.width = 2;
        tet.height = 3;
      }
      break;
    case TET_L:
      if(orientation & 2) {
        if(orientation & 1) {
          tet[0][2] = tet[1][2] = tet[1][1] = tet[1][0] = true;
          tet.width = 3;
          tet.height = 2;
        }
        else {
          tet[0][0] = tet[0][1] = tet[1][1] = tet[2][1] = true;
          tet.width = 2;
          tet.height = 3;
        }
      }
      else {
        if(orientation & 1) {
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][0] = true;
          tet.width = 3;
          tet.height = 2;
        }
        else {
          tet[0][0] = tet[1][0] = tet[2][0] = tet[2][1] = true;
          tet.width = 2;
          tet.height = 3;
        }
      }
      break;
    case TET_J:
      if(orientation & 2) {
        if(orientation & 1) {
          tet[0][0] = tet[1][2] = tet[1][1] = tet[1][0] = true;
          tet.width = 3;
          tet.height = 2;
        }
        else {
          tet[2][0] = tet[0][1] = tet[1][1] = tet[2][1] = true;
          tet.width = 2;
          tet.height = 3;
        }
      }
      else {
        if(orientation & 1) {
          tet[0][0] = tet[0][1] = tet[0][2] = tet[1][2] = true;
          tet.width = 3;
          tet.height = 2;
        }
        else {
          tet[0][0] = tet[1][0] = tet[2][0] = tet[0][1] = true;
          tet.width = 2;
          tet.height = 3;
        }
      }
      break;
    default:
      abort();
    }

    return tet;
  }

  uint8_t Environment::clear_lines(const std::pair<size_t, size_t> &position) {
    uint8_t lines_cleared = 0.0;

    for(size_t j = position.second > 4 ? position.second - 4 : 0, jend = j + 4; j != jend; ) {
      if(m_grid[j].second)
        ++j;
      else {
        memmove(&m_grid[j], &m_grid[j + 1], (19 - j) * sizeof(m_grid[0]));
        memset(&m_grid[19].first, 0, sizeof(m_grid[19].first));
        m_grid[19].second = m_grid[19].first.size();
        ++lines_cleared;
        --jend;
      }
    }

    return lines_cleared;
  }

  size_t Environment::gaps_beneath(const Tetromino &tet, const std::pair<size_t, size_t> &position) const {
    size_t gaps = 0;

    for(int i = 0; i != tet.width; ++i) {
      const int barrier = position.second < 4 ? position.second + 1 : 4;

      size_t sum = 0;
      for(int j = 0; j != barrier; ++j) {
        if(tet[j][i])
          sum = 0;
        else if(!m_grid[position.second - j].first[position.first + i])
          ++sum;
      }

      gaps += sum;

      for(int j = position.second - barrier; j > -1; --j) {
        if(!m_grid[j].first[position.first + i])
          ++gaps;
      }
    }

    return gaps;
  }

  size_t Environment::gaps_created(const Tetromino &tet, const std::pair<size_t, size_t> &position) const {
    size_t gaps = 0;

    for(int i = 0; i != tet.width; ++i) {
      const int barrier = position.second < 4 ? position.second + 1 : 4;

      size_t sum = 0;
      bool done = false;
      for(int j = 0; j != barrier; ++j) {
        if(tet[j][i])
          sum = 0;
        else if(m_grid[position.second - j].first[position.first + i]) {
          done = true;
          break;
        }
        else
          ++sum;
      }

      gaps += sum;
      if(done)
        continue;

      for(int j = position.second - barrier; j > -1; --j) {
        if(m_grid[j].first[position.first + i])
          break;
        else
          ++gaps;
      }
    }

    return gaps;
  }

  Environment::Outcome Environment::outcome(const uint8_t &lines_cleared, const Tetromino &tet, const std::pair<size_t, size_t> &position) const {
    assert(lines_cleared && lines_cleared < 5);

    Environment next(*this);
    next.m_lookahead = true;
    next.place_Tetromino(tet, position);
    if(next.clear_lines(position) >= lines_cleared)
      return OUTCOME_ACHIEVED;

    if(!m_lookahead) {
      std::vector<Tetromino_Type> types(1, TET_LINE);
      if(lines_cleared < 4) {
        types.push_back(TET_T);
        types.push_back(TET_S);
        types.push_back(TET_Z);
        types.push_back(TET_L);
        types.push_back(TET_J);
        if(lines_cleared < 3)
          types.push_back(TET_SQUARE);
      }

      for(const auto t : types) {
        Environment next2(next);
        next2.m_current = t;
        next2.generate_placements();
        for(auto nn : next2.m_placements) {
          if(nn.outcome[lines_cleared] == OUTCOME_ACHIEVED)
            return OUTCOME_ENABLED;
        }
      }

      next = *this;
      next.m_lookahead = true;
      for(const auto t : types) {
        next.m_current = t;
        next.generate_placements();
        for(auto nn : next.m_placements) {
          if(nn.outcome[lines_cleared] == OUTCOME_ACHIEVED)
            return OUTCOME_PROHIBITED;
        }
      }
    }

    return OUTCOME_NULL;
  }

  void Environment::generate_placements() {
    const uint8_t orientations = orientations_Tetromino(m_current);

    m_placements.clear();

    for(int orientation = 0; orientation != orientations; ++orientation) {
      const auto tet = generate_Tetromino(m_current, orientation);

      for(size_t i = 0, iend = 11 - tet.width; i != iend; ++i) {
        size_t j = tet.height - 1;

        for(size_t x = 0; x != tet.width; ++x) {
          const size_t ymax = tet[3][x] ? 4 : tet[2][x] ? 3 : tet[1][x] ? 2 : 1;

          for(size_t y = 19; y < 20; --y) {
            if(m_grid[y].first[i + x]) {
              j = std::max(j, y + ymax);
              break;
            }
          }
        }

        if(j < 20) {
          const auto position = std::make_pair(i, j);
          m_placements.emplace_back(orientation,
                                    std::make_pair(tet.width, tet.height),
                                    position,
                                    gaps_beneath(tet, position),
                                    gaps_created(tet, position),
                                    outcome(1, tet, position),
                                    outcome(2, tet, position),
                                    outcome(3, tet, position),
                                    outcome(4, tet, position));
        }
      }
    }
  }

  uint8_t Environment::orientations_Tetromino(const Tetromino_Type &type) {
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

  template<typename SUBFEATURE, typename AXIS>
  void Agent::generate_rete_continuous(const Node_Unsplit_Ptr &node_unsplit,
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
    auto filter_gaps_beneath = make_filter(Rete::WME(m_first_var, m_gaps_beneath_attr, m_third_var));
    auto join_gaps_beneath = make_join(state_bindings, join_y, filter_gaps_beneath);
    auto filter_gaps_created = make_filter(Rete::WME(m_first_var, m_gaps_created_attr, m_third_var));
    auto join_gaps_created = make_join(state_bindings, join_gaps_beneath, filter_gaps_created);
    auto &join_last = join_gaps_created;

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

    for(Type::Axis axis : {Type::CURRENT, Type::NEXT}) {
      for(size_t type = 0; type != TET_TYPES; ++type) {
        auto node_fringe = std::make_shared<Node_Fringe>(*this, 2);
        auto feature = new Type(axis, Tetromino_Type(type));
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

    generate_rete_continuous<Size, Size::Axis>(node_unsplit, get_action, Size::WIDTH, 0.0f, 4.0f);
    generate_rete_continuous<Size, Size::Axis>(node_unsplit, get_action, Size::HEIGHT, 0.0f, 4.0f);
    generate_rete_continuous<Position, Position::Axis>(node_unsplit, get_action, Position::X, 0.0f, 10.0f);
    generate_rete_continuous<Position, Position::Axis>(node_unsplit, get_action, Position::Y, 0.0f, 20.0f);
    generate_rete_continuous<Gaps, Gaps::Axis>(node_unsplit, get_action, Gaps::BENEATH, 0.0f, 75.0f);
    generate_rete_continuous<Gaps, Gaps::Axis>(node_unsplit, get_action, Gaps::CREATED, 0.0f, 75.0f);
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
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_gaps_beneath_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.gaps_beneath)));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_gaps_created_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.gaps_created)));
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
