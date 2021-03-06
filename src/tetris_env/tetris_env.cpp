#include "tetris.h"

#include "carli/parser/rete_parser.h"

#define GENERATE_ONLY_LINES

namespace Tetris {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  Environment::Environment()
    : score_line({{0.0, 10.0, 20.0, 30.0, 40.0}}) ///< No risk-reward tradeoff
    //: score_line({{0.0, 10.0, 20.0, 40.0, 80.0}}) ///< Risk-reward tradeoff
  {
    init_impl();
  }

  Environment::Environment(const Environment &rhs)
   : Carli::Environment(rhs),
   score_line(rhs.score_line),
   m_random_init(rhs.m_random_init),
   m_random_selection(rhs.m_random_selection),
   m_grid(rhs.m_grid),
   m_current(rhs.m_current),
   m_next(rhs.m_next),
   //m_placements(rhs.m_placements),
   m_lookahead(rhs.m_lookahead)
  {
  }

  Environment & Environment::operator=(const Environment &rhs) {
    Carli::Environment::operator=(rhs);

    score_line = rhs.score_line;
    m_random_init = rhs.m_random_init;
    m_random_selection = rhs.m_random_selection;
    m_grid = rhs.m_grid;
    m_current = rhs.m_current;
    m_next = rhs.m_next;
    //m_placements = rhs.m_placements;
    m_lookahead = rhs.m_lookahead;

    return *this;
  }

  void Environment::init_impl() {
    m_random_selection = Zeni::Random(2147483647 * m_random_init.rand());

    memset(&m_grid, 0, sizeof(m_grid));
    for(auto &line : m_grid)
      line.second = int16_t(line.first.size());

#ifdef GENERATE_ONLY_LINES
    m_current = TETS_LINE;
    m_next = TETS_LINE;
#else
    m_current = Tetromino_Supertype(m_random_selection.rand_lt(7) + 1);
    m_next = Tetromino_Supertype(m_random_selection.rand_lt(7) + 1);
#endif
//    m_current = m_random_selection.rand_lt(2) ? TETS_LINE : TETS_SQUARE;
//    m_next = m_random_selection.rand_lt(2) ? TETS_LINE : TETS_SQUARE;
//    m_current = TETS_T;
//    m_next = TETS_T;

    generate_placements();
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action) {
    const Place &place = debuggable_cast<const Place &>(action);

    const auto tet = generate_Tetromino(place.type);
    place_Tetromino(tet, place.position);

    m_current = m_next;
#ifndef GENERATE_ONLY_LINES
    m_next = Tetromino_Supertype(m_random_selection.rand_lt(7) + 1);
#endif
//    m_next = m_random_selection.rand_lt(2) ? TETS_LINE : TETS_SQUARE;

    const double score = score_line[clear_lines(place.position)];

    generate_placements();

    if(m_placements.empty())
      return std::make_pair(score_failure, score_failure);
    else
      return std::make_pair(score, score);
  }

  void Environment::place_Tetromino(const Environment::Tetromino &tet, const std::pair<int16_t, int16_t> &position) {
    for(auto tt = tet.begin(); tt != tet.end(); ++tt) {
      auto &line = m_grid[position.second - tt->second];
      assert(!line.first[position.first + tt->first]);
      line.first[position.first + tt->first] = true;
      --line.second;
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
    auto tet = generate_Tetromino(super_to_type(m_current, 0));
    int index = 0;
    for(int j = 0; index != 4 && j != tet.height; ++j) {
      os << "  ";
      for(int i = 0; index != 4 && tet[index].second == j && i != tet.width; ++i) {
        if(tet[index].first == i) {
          os << 'O';
          ++index;
        }
        else
          os << ' ';
      }
      os << std::endl;
    }
    assert(index == 4);

    os << "Next:" << std::endl;
    tet = generate_Tetromino(super_to_type(m_next, 0));
    index = 0;
    for(int j = 0; index != 4 && j != tet.height; ++j) {
      os << "  ";
      for(int i = 0; index != 4 && tet[index].second == j && i != tet.width; ++i) {
        if(tet[index].first == i) {
          os << 'O';
          ++index;
        }
        else
          os << ' ';
      }
      os << std::endl;
    }
    assert(index == 4);
  }

  Environment::Tetromino Environment::generate_Tetromino(const Tetromino_Type &type) {
    Environment::Tetromino tet;
    memset(&tet, 0, sizeof(tet));

    switch(type) {
    case TET_SQUARE:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet.width = 2;
      tet.height = 2;
      break;

    case TET_LINE_DOWN:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(0, 2);
      tet[3] = std::make_pair<uint8_t, uint8_t>(0, 3);
      tet.width = 1;
      tet.height = 4;
      break;

    case TET_LINE_RIGHT:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(2, 0);
      tet[3] = std::make_pair<uint8_t, uint8_t>(3, 0);
      tet.width = 4;
      tet.height = 1;
      break;

    case TET_T:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(2, 0);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_T_90:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(0, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_T_180:
      tet[0] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(2, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_T_270:
      tet[0] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_L:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(0, 2);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_L_90:
      tet[0] = std::make_pair<uint8_t, uint8_t>(2, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(2, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_L_180:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_L_270:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(2, 0);
      tet[3] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_J:
      tet[0] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(0, 2);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_J_90:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(2, 0);
      tet[3] = std::make_pair<uint8_t, uint8_t>(2, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_J_180:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(0, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_J_270:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(2, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_S:
      tet[0] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(2, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_S_90:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(1, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    case TET_Z:
      tet[0] = std::make_pair<uint8_t, uint8_t>(0, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(2, 1);
      tet.width = 3;
      tet.height = 2;
      break;

    case TET_Z_90:
      tet[0] = std::make_pair<uint8_t, uint8_t>(1, 0);
      tet[1] = std::make_pair<uint8_t, uint8_t>(0, 1);
      tet[2] = std::make_pair<uint8_t, uint8_t>(1, 1);
      tet[3] = std::make_pair<uint8_t, uint8_t>(0, 2);
      tet.width = 2;
      tet.height = 3;
      break;

    default:
      abort();
    }

    return tet;
  }

  uint8_t Environment::clear_lines(const std::pair<int16_t, int16_t> &position) {
    uint8_t lines_cleared = 0;

    for(int16_t j = position.second > 3 ? position.second - 3 : 0, jend = j + 4; j != jend; ) {
      if(m_grid[j].second)
        ++j;
      else {
        memmove(&m_grid[j], &m_grid[j + 1], (19 - j) * sizeof(m_grid[0]));
        memset(&m_grid[19].first, 0, sizeof(m_grid[19].first));
        m_grid[19].second = int16_t(m_grid[19].first.size());
        ++lines_cleared;
        --jend;
      }
    }

#ifndef NDEBUG
    for(const auto &line : m_grid)
      assert(line.second);
#endif

    return lines_cleared;
  }

  int16_t Environment::gaps_beneath(const Tetromino &tet, const std::pair<int16_t, int16_t> &position) const {
    int16_t gaps = 0;

    for(int i = 0; i != tet.width; ++i) {
      int16_t j = position.second;
      for(auto tt = tet.begin(); tt != tet.end(); ++tt)
        if(tt->first == i)
          j = position.second - tt->second - 1;

      for(; j > -1; --j) {
        if(!m_grid[j].first[position.first + i])
          ++gaps;
      }
    }

    return gaps;
  }

  int16_t Environment::gaps_created(const Tetromino &tet, const std::pair<int16_t, int16_t> &position) const {
    int16_t gaps = 0;

    for(int i = 0; i != tet.width; ++i) {
      int16_t j = position.second;
      for(auto tt = tet.begin(); tt != tet.end(); ++tt)
        if(tt->first == i)
          j = position.second - tt->second - 1;

      for(; j > -1; --j) {
        if(m_grid[j].first[position.first + i])
          break;
        else
          ++gaps;
      }
    }

    return gaps;
  }

  int16_t Environment::depth_to_highest_gap() const {
    int16_t max_gap = 0u;

    for(int16_t i = 0, iend = int16_t(m_grid[0].first.size()); i != iend; ++i) {
      int16_t blocked = 0u;

      for(int16_t j = int16_t(m_grid.size()) - 1; j > -1; --j) {
        if(m_grid[j].first[i])
          ++blocked;
        else
          break;
      }

      max_gap = std::max(max_gap, blocked);
    }

    return max_gap;
  }

  Environment::Outcome Environment::outcome(const uint8_t &lines_cleared, const Tetromino &tet, const std::pair<int16_t, int16_t> &position) const {
    assert(lines_cleared && lines_cleared < 5);

    Environment next(*this);
    next.m_lookahead = true;
    next.place_Tetromino(tet, position);
    if(next.clear_lines(position) >= lines_cleared)
      return Outcome::OUTCOME_ACHIEVED;

    if(!m_lookahead) {
      std::vector<Tetromino_Supertype> types;
      types.push_back(TETS_LINE);
      if(lines_cleared < 4) {
        types.push_back(TETS_T);
        types.push_back(TETS_L);
        types.push_back(TETS_J);
        types.push_back(TETS_S);
        types.push_back(TETS_Z);
        if(lines_cleared < 3)
          types.push_back(TETS_SQUARE);
      }

      for(const auto t : types) {
        next.m_current = t;
        next.generate_placements();
        for(auto nn : next.m_placements) {
          if(nn.outcome[lines_cleared] == Outcome::OUTCOME_ACHIEVED)
            return Outcome::OUTCOME_ENABLED;
        }
      }

      next = *this;
      next.m_lookahead = true;
      for(const auto t : types) {
        next.m_current = t;
        next.generate_placements();
        for(auto nn : next.m_placements) {
          if(nn.outcome[lines_cleared] == Outcome::OUTCOME_ACHIEVED)
            return Outcome::OUTCOME_PROHIBITED;
        }
      }
    }

    return Outcome::OUTCOME_NULL;
  }

  void Environment::generate_placements() {
    const uint8_t orientations = num_types(m_current);

    m_placements.clear();

    for(uint8_t orientation = 0; orientation != orientations; ++orientation) {
      const auto type = super_to_type(m_current, orientation);
      const auto tet = generate_Tetromino(type);

      for(int16_t i = 0, iend = 11 - tet.width; i != iend; ++i) {
        int16_t j = tet.height - 1;

        for(int16_t x = 0; x != tet.width; ++x) {
          const int16_t ymax = (tet[3].first == x ? tet[3].second :
                               tet[2].first == x ? tet[2].second :
                               tet[1].first == x ? tet[1].second :
                                                   tet[0].second) + 1;

          for(int16_t y = int16_t(m_grid.size()) - 1; y > -1; --y) {
            if(m_grid[y].first[i + x]) {
              j = std::max(j, int16_t(y + ymax));
              break;
            }
          }
        }

        if(j < int16_t(m_grid.size())) {
          assert(j + 1 - tet.height > -1);
          assert(!m_grid[j - tet[0].second].first[i + tet[0].first]);
          assert(!m_grid[j - tet[1].second].first[i + tet[1].first]);
          assert(!m_grid[j - tet[2].second].first[i + tet[2].first]);
          assert(!m_grid[j - tet[3].second].first[i + tet[3].first]);

          const auto position = std::make_pair(i, j);
          m_placements.emplace_back(type,
                                    std::make_pair(tet.width, tet.height),
                                    position,
                                    gaps_beneath(tet, position),
                                    gaps_created(tet, position),
                                    depth_to_highest_gap(),
                                    outcome(1, tet, position),
                                    outcome(2, tet, position),
                                    outcome(3, tet, position),
                                    outcome(4, tet, position));
        }
      }
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Place>(variables, token);})
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
      rules_in = "rules/tetris-ycc.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    std::list<Rete::WME_Ptr_C> wmes_current;
    std::ostringstream oss;

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_type_next_attr, std::make_shared<Rete::Symbol_Constant_Int>(env->get_next())));

    size_t index = 0;
    for(const auto &placement : env->get_placements()) {
      oss << "place-" << ++index;
      Rete::Symbol_Identifier_Ptr_C action_id = std::make_shared<Rete::Symbol_Identifier>(oss.str());
      oss.str("");
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, action_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(action_id, m_type_attr, std::make_shared<Rete::Symbol_Constant_Int>(placement.type)));
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

    Rete::Agenda::Locker locker(agenda);
    CPU_Accumulator cpu_accumulator(*this);

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      for(auto wt = m_wmes_prev.begin(), wend = m_wmes_prev.end(); wt != wend; ++wt)
        remove_wme(*wt);
      m_wmes_prev.clear();
    }
    else {
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

    m_metastate = env->get_placements().empty() ? Metastate::FAILURE : Metastate::NON_TERMINAL;
  }

}
