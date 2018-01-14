#include "sliding_puzzle.h"

#include "carli/parser/rete_parser.h"
#include "carli/utility/shared_suffix_list.h"

#include <algorithm>
#include <queue>

namespace Sliding_Puzzle {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  static const int32_t g_num_colors = 3;

  static bool xor_string(const std::string &str) {
    char x = '\0';
    for(const char &c : str)
      x ^= c;
    return x & 1;
  }

  Environment::Environment() {
    init_impl();
  }

  bool Environment::success() const {
    return success(m_grid);
  }

  bool Environment::success(const Environment::Grid &grid) const {
    for(int64_t i = 1; i != int64_t(grid.size()); ++i) {
      if(m_grid[i - 1] != i)
        return false;
    }
    return true;
  }

  int64_t Environment::calculate_num_steps_to_goal() const {
    const bool approximate = true;

    typedef Zeni::Shared_Suffix_List<int64_t> Gridpart_SSL;

    std::unordered_set<Gridpart_SSL::list_pointer_type, Rete::hash_deref<Gridpart_SSL>, Rete::compare_deref_eq> all_gridparts;

    const auto Grid_to_Gridpart_SSL = [&all_gridparts](const Grid &grid)->Gridpart_SSL::list_pointer_type {
      Gridpart_SSL::list_pointer_type gridpart_ssl;
      for(auto st = grid.rbegin(); st != grid.rend(); ++st) {
        Gridpart_SSL::list_pointer_type insertion = std::make_shared<Gridpart_SSL>(*st, gridpart_ssl);
        auto inserted = all_gridparts.insert(insertion);
        gridpart_ssl = *inserted.first;
      }
      return gridpart_ssl;
    };
    const auto Gridpart_SSL_to_Grid = [&all_gridparts](const Gridpart_SSL::list_pointer_type &stack_ssl)->Grid {
      Grid stack;
      for(auto st = stack_ssl->begin(); st != stack_ssl->end(); ++st)
        stack.push_back(*st);
      return stack;
    };

    struct State : public std::enable_shared_from_this<State> {
      State(const Gridpart_SSL::list_pointer_type &grid_, const int64_t &num_steps_, const int64_t &heuristic_, const std::shared_ptr<State> &prev_state_ = nullptr) : prev_state(prev_state_), grid(grid_), num_steps(num_steps_), heuristic(heuristic_) {}

      bool operator<(const State &rhs) const {
        return num_steps + heuristic < rhs.num_steps + rhs.heuristic ||
              (num_steps + heuristic == rhs.num_steps + rhs.heuristic &&
                (num_steps < rhs.num_steps ||
                (num_steps == rhs.num_steps &&
                  (heuristic < rhs.heuristic ||
                  (heuristic == rhs.heuristic &&
                    grid < rhs.grid)))));
      }

      std::shared_ptr<State> prev_state;
      Gridpart_SSL::list_pointer_type grid;
      int64_t num_steps;
      int64_t heuristic;
    };
    std::set<std::shared_ptr<State>, Rete::compare_deref_lt> astar;

    Grid target = m_grid;
    for(int64_t i = 1; i != int64_t(m_grid.size()); ++i)
      target[i - 1] = i;
    *target.rbegin() = 0;
    const auto target_ssl = Grid_to_Gridpart_SSL(target);

    //for(auto tile : m_grid)
    //  std::cerr << ' ' << tile;
    //std::cerr << std::endl;
    //for(auto tile : Gridpart_SSL_to_Grid(Grid_to_Gridpart_SSL(m_grid)))
    //  std::cerr << ' ' << tile;
    //std::cerr << std::endl;
    //exit(42);

    const std::function<int64_t (const Grid &grid)> heuristic = approximate ?
    std::function<int64_t(const Grid &grid)>([this](const Grid &grid)->int64_t {
      const std::pair<int64_t, int64_t> rps = remaining_problem_size(grid);
      std::unordered_set<int64_t> fringe;
      for(int64_t i = m_grid_w - rps.first; i != m_grid_w; ++i)
        fringe.insert((m_grid_h - rps.second) * m_grid_w + i + 1);
      for(int64_t j = m_grid_h - rps.second + 1; j != m_grid_h; ++j)
        fringe.insert(j * m_grid_w + (m_grid_w - rps.first) + 1);
      fringe.erase(m_grid_w * m_grid_h);

      std::unordered_map<int64_t, std::pair<int64_t, int64_t>> target_pos;
      for(int64_t i : fringe)
        target_pos[i] = std::make_pair((i - 1) % m_grid_w, (i - 1) / m_grid_w);

      std::unordered_map<int64_t, std::pair<int64_t, int64_t>> grid_pos;
      for(int64_t j = 0; j != m_grid_h; ++j) {
        for(int64_t i = 0; i != m_grid_w; ++i) {
          if(fringe.find(grid[j * m_grid_w + i]) != fringe.end())
            grid_pos[grid[j * m_grid_w + i]] = std::make_pair(i, j);
        }
      }

      int64_t manhattan = 0;
      for(int64_t i : fringe) {
        manhattan += std::abs(grid_pos[i].first - target_pos[i].first) + std::abs(grid_pos[i].second - target_pos[i].second);
      }

      int64_t inversions = 0;
      {
        const int64_t i = m_grid_w - rps.first;
        std::set<int64_t> numbers;
        for(int64_t j = m_grid_h - rps.second; j != m_grid_h; ++j) {
          if(!grid[j * m_grid_w + i] || (grid[j * m_grid_w + i] - 1) % m_grid_w != i)
            continue;
          const auto ub = numbers.upper_bound(grid[j * m_grid_w + i]);
          inversions += std::distance(ub, numbers.end());
          numbers.insert(grid[j * m_grid_w + i]);
        }
      }
      {
        const int64_t j = m_grid_h - rps.second;
        std::set<int64_t> numbers;
        for(int64_t i = m_grid_w - rps.first; i != m_grid_w; ++i) {
          if(!grid[j * m_grid_w + i] || (grid[j * m_grid_w + i] - 1) / m_grid_w != j)
            continue;
          const auto ub = numbers.upper_bound(grid[j * m_grid_w + i]);
          inversions += std::distance(ub, numbers.end());
          numbers.insert(grid[j * m_grid_w + i]);
        }
      }

      return manhattan + 2 * inversions;
    }) :
    std::function<int64_t(const Grid &grid)>([this](const Grid &grid)->int64_t {
      //return 0;

      std::unordered_map<int64_t, std::pair<int64_t, int64_t>> target_pos;
      for(int64_t i = 1; i != int64_t(grid.size()); ++i)
        target_pos[i] = std::make_pair((i - 1) % m_grid_w, (i - 1) / m_grid_w);

      std::unordered_map<int64_t, std::pair<int64_t, int64_t>> grid_pos;
      for(int64_t j = 0; j != m_grid_h; ++j) {
        for(int64_t i = 0; i != m_grid_w; ++i) {
          if(grid[j * m_grid_w + i])
            grid_pos[grid[j * m_grid_w + i]] = std::make_pair(i, j);
        }
      }

      int64_t manhattan = 0;
      for(int64_t i = 1; i != int64_t(m_grid.size()); ++i)
        manhattan += std::abs(grid_pos[i].first - target_pos[i].first) + std::abs(grid_pos[i].second - target_pos[i].second);

      int64_t inversions = 0;
      for(int64_t i = 0; i != m_grid_w; ++i) {
        std::set<int64_t> numbers;
        for(int64_t j = 0; j != m_grid_h; ++j) {
          if(!grid[j * m_grid_w + i] || (grid[j * m_grid_w + i] - 1) % m_grid_w != i)
            continue;
          const auto ub = numbers.upper_bound(grid[j * m_grid_w + i]);
          inversions += std::distance(ub, numbers.end());
          numbers.insert(grid[j * m_grid_w + i]);
        }
      }
      for(int64_t j = 0; j != m_grid_h; ++j) {
        std::set<int64_t> numbers;
        for(int64_t i = 0; i != m_grid_w; ++i) {
          if(!grid[j * m_grid_w + i] || (grid[j * m_grid_w + i] - 1) / m_grid_w != j)
            continue;
          const auto ub = numbers.upper_bound(grid[j * m_grid_w + i]);
          inversions += std::distance(ub, numbers.end());
          numbers.insert(grid[j * m_grid_w + i]);
        }
      }

      return manhattan + 2 * inversions;
    });

    std::unordered_set<Gridpart_SSL::list_pointer_type, Rete::hash_deref<Gridpart_SSL>, Rete::compare_deref_eq> states_evaluated;

    std::pair<int64_t, int64_t> rps_min(m_grid_w, m_grid_h);

    {
      auto grid_ssl = Grid_to_Gridpart_SSL(m_grid);
      states_evaluated.insert(grid_ssl);
      astar.insert(std::make_shared<State>(grid_ssl, 0, heuristic(m_grid)));
      if(approximate)
        rps_min = remaining_problem_size(m_grid);

      std::cerr << "Initial " << rps_min.first << 'x' << rps_min.second << ':' << std::endl;
      for(int64_t j = 0; j != m_grid_h; ++j) {
        std::cerr << ' ';
        for(int64_t i = 0; i != m_grid_w; ++i) {
          std::cerr << ' ' << m_grid[j * m_grid_w + i];
        }
        std::cerr << std::endl;
      }
    }

    int64_t num_steps = 0;
    while(!astar.empty()) {
      const auto state = *astar.begin();
      astar.erase(astar.begin());

      //std::cerr << astar.size() << ' ' << state->num_steps << ' ' << state->heuristic << std::endl;

      if(state->grid == target_ssl) {
        std::stack<std::shared_ptr<State>> states;
        for(auto ss = state; ss; ss = ss->prev_state)
          states.push(ss);
        int64_t step = 0;
        while(!states.empty()) {
          auto ss = states.top();
          states.pop();

          auto grid = Gridpart_SSL_to_Grid(ss->grid);
          const std::pair<int64_t, int64_t> rps = remaining_problem_size(grid);
          std::cerr << "Step " << step++ << " -- " << ss->num_steps << "+" << ss->heuristic << " -- " << rps.first << 'x' << rps.second << std::endl;
          for(int64_t j = 0; j != m_grid_h; ++j) {
            std::cerr << ' ';
            for(int64_t i = 0; i != m_grid_w; ++i) {
              std::cerr << ' ' << grid[j * m_grid_w + i];
            }
            std::cerr << std::endl;
          }
        }

        num_steps = state->num_steps;
        std::cerr << "Num steps = " << num_steps << std::endl;
        break;
      }

      const Grid grid = Gridpart_SSL_to_Grid(state->grid);

      const int64_t blank_tile = int64_t(std::distance(grid.begin(), std::find(grid.begin(), grid.end(), 0)));
      const int64_t blank_x = blank_tile % m_grid_w;
      const int64_t blank_y = blank_tile / m_grid_w;

      const std::pair<int64_t, int64_t> rps = remaining_problem_size(grid);

      if(approximate) {
        if(rps.first > rps_min.first || rps.second > rps_min.second)
          continue;
      }

      if(blank_y + 1 != m_grid_h) {
        Grid next = grid;
        std::swap(next[blank_tile], next[(blank_y + 1) * m_grid_w + blank_x]);
        bool ins = true;
        if(approximate) {
          const auto rps_next = remaining_problem_size(next);
          if((rps_next.first <  rps_min.first && rps_next.second <= rps_min.second) ||
             (rps_next.first <= rps_min.first && rps_next.second <  rps_min.second))
          {
            //std::cerr << "Shrink " << rps_next.first << 'x' << rps_next.second << ':' << std::endl;
            //for(int64_t j = 0; j != m_grid_h; ++j) {
            //  std::cerr << ' ';
            //  for(int64_t i = 0; i != m_grid_w; ++i) {
            //    std::cerr << ' ' << next[j * m_grid_w + i];
            //  }
            //  std::cerr << std::endl;
            //}
            rps_min = rps_next;
          }
          else if(rps_next.first > rps_min.first || rps_next.second > rps_min.second)
            ins = false;
        }
        if(ins) {
          auto next_ssl = Grid_to_Gridpart_SSL(next);
          if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
            states_evaluated.insert(next_ssl);
            astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
          }
        }
      }
      if(blank_y + rps.second > m_grid_h) {
        Grid next = grid;
        std::swap(next[blank_tile], next[(blank_y - 1) * m_grid_w + blank_x]);
        bool ins = true;
        if(approximate) {
          const auto rps_next = remaining_problem_size(next);
          if((rps_next.first <  rps_min.first && rps_next.second <= rps_min.second) ||
             (rps_next.first <= rps_min.first && rps_next.second <  rps_min.second))
          {
            //std::cerr << "Shrink " << rps_next.first << 'x' << rps_next.second << ':' << std::endl;
            //for(int64_t j = 0; j != m_grid_h; ++j) {
            //  std::cerr << ' ';
            //  for(int64_t i = 0; i != m_grid_w; ++i) {
            //    std::cerr << ' ' << next[j * m_grid_w + i];
            //  }
            //  std::cerr << std::endl;
            //}
            rps_min = rps_next;
          }
          else if(rps_next.first > rps_min.first || rps_next.second > rps_min.second)
            ins = false;
        }
        if(ins) {
          auto next_ssl = Grid_to_Gridpart_SSL(next);
          if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
            states_evaluated.insert(next_ssl);
            astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
          }
        }
      }
      if(blank_x + 1 != m_grid_w) {
        Grid next = grid;
        std::swap(next[blank_tile], next[blank_y * m_grid_w + blank_x + 1]);
        bool ins = true;
        if(approximate) {
          const auto rps_next = remaining_problem_size(next);
          if((rps_next.first <  rps_min.first && rps_next.second <= rps_min.second) ||
             (rps_next.first <= rps_min.first && rps_next.second <  rps_min.second))
          {
            //std::cerr << "Shrink " << rps_next.first << 'x' << rps_next.second << ':' << std::endl;
            //for(int64_t j = 0; j != m_grid_h; ++j) {
            //  std::cerr << ' ';
            //  for(int64_t i = 0; i != m_grid_w; ++i) {
            //    std::cerr << ' ' << next[j * m_grid_w + i];
            //  }
            //  std::cerr << std::endl;
            //}
            rps_min = rps_next;
          }
          else if(rps_next.first > rps_min.first || rps_next.second > rps_min.second)
            ins = false;
        }
        if(ins) {
          auto next_ssl = Grid_to_Gridpart_SSL(next);
          if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
            states_evaluated.insert(next_ssl);
            astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
          }
        }
      }
      if(blank_x + rps.first > m_grid_w) {
        Grid next = grid;
        std::swap(next[blank_tile], next[blank_y * m_grid_w + blank_x - 1]);
        bool ins = true;
        if(approximate) {
          const auto rps_next = remaining_problem_size(next);
          if((rps_next.first <  rps_min.first && rps_next.second <= rps_min.second) ||
             (rps_next.first <= rps_min.first && rps_next.second <  rps_min.second))
          {
            //std::cerr << "Shrink " << rps_next.first << 'x' << rps_next.second << ':' << std::endl;
            //for(int64_t j = 0; j != m_grid_h; ++j) {
            //  std::cerr << ' ';
            //  for(int64_t i = 0; i != m_grid_w; ++i) {
            //    std::cerr << ' ' << next[j * m_grid_w + i];
            //  }
            //  std::cerr << std::endl;
            //}
            rps_min = rps_next;
          }
          else if(rps_next.first > rps_min.first || rps_next.second > rps_min.second)
            ins = false;
        }
        if(ins) {
          auto next_ssl = Grid_to_Gridpart_SSL(next);
          if(states_evaluated.find(next_ssl) == states_evaluated.end()) {
            states_evaluated.insert(next_ssl);
            astar.insert(std::make_shared<State>(next_ssl, state->num_steps + 1, heuristic(next), state));
          }
        }
      }
    }

    return num_steps;
  }

  std::pair<int64_t, int64_t> Environment::remaining_problem_size(const Grid &grid) const {
    int64_t x_in_place = 0;
    int64_t y_in_place = 0;
    bool x_out_place = false;
    bool y_out_place = false;

    for(;;) {
      if(!y_out_place) {
        int64_t i;
        for(i = 0; i != m_grid_w; ++i) {
          if(grid[y_in_place * m_grid_w + i] != y_in_place * m_grid_w + i + 1)
            break;
        }
        if(i != m_grid_w || !solveable(grid, x_in_place, y_in_place + 1)) {
          y_out_place = true;
          if(x_out_place)
            break;
        }
        else
          ++y_in_place;
      }

      if(!x_out_place) {
        int64_t j;
        for(j = 0; j != m_grid_h; ++j) {
          if(grid[j * m_grid_w + x_in_place] != j * m_grid_w + x_in_place + 1)
            break;
        }
        if(j != m_grid_h || !solveable(grid, x_in_place + 1, y_in_place)) {
          x_out_place = true;
          if(y_out_place)
            break;
        }
        else
          ++x_in_place;
      }
    }

    return std::make_pair(m_grid_w - x_in_place, m_grid_h - y_in_place);
  }

  bool Environment::solveable(const Grid &grid, const int64_t &initial_x, const int64_t &initial_y) const {
    int64_t inversions = 0;

    std::set<int64_t> numbers;
    for(int64_t j = initial_y; j != m_grid_h; ++j) {
      for(int64_t i = initial_x; i != m_grid_w; ++i) {
        if(!grid[j * m_grid_w + i]) {
          if(((m_grid_w - initial_x) & 1) == 0 && initial_y != m_grid_h - 1) /// Must count row number of blank if width is even, unless height is 0
            inversions += j + 1;
          continue;
        }
        const auto ub = numbers.upper_bound(grid[j * m_grid_w + i]);
        inversions += std::distance(ub, numbers.end());
        numbers.insert(grid[j * m_grid_w + i]);
      }
    }

    if(initial_x == m_grid_w - 1 || initial_y == m_grid_h - 1)
      return inversions == 0; /// Special case unary scenario
    else if(((m_grid_w - initial_x) & 1) == 0)
      return (inversions & 1) == (m_grid_h & 1); /// Counting row number of blank if width is even
    else
      return (inversions & 1) == 0; /// Counting inversions only if width is odd
  }

  void Environment::init_impl() {
    //assert(m_num_blocks_min > 2);
    //assert(m_num_blocks_max >= m_num_blocks_min);

    //int64_t num_blocks = 0;
    //{
    //  num_blocks = m_num_blocks_min + (m_num_blocks_min != m_num_blocks_max ? m_random.rand_lte(int32_t(m_num_blocks_max - m_num_blocks_min)) : 0);
    //}

    m_grid_w = 4;
    m_grid_h = 4;

    m_grid.resize(m_grid_w * m_grid_h);

    for(int64_t i = 1; i != int64_t(m_grid.size()); ++i)
      m_grid[i - 1] = i;
    *m_grid.rbegin() = 0;

    do {
      std::shuffle(m_grid.begin(), m_grid.end(), m_random);
    } while(!solveable(m_grid));

    if(m_evaluate_optimality) {
      static bool skip_first = true;
      if(skip_first)
        skip_first = false;
      else
        m_num_steps_to_goal = calculate_num_steps_to_goal();
    }
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action) {
    const Move &move = debuggable_cast<const Move &>(action);

    const int64_t blank_tile = int64_t(std::distance(m_grid.begin(), std::find(m_grid.begin(), m_grid.end(), 0)));
    const int64_t blank_x = blank_tile % m_grid_w;
    const int64_t blank_y = blank_tile / m_grid_w;

    switch(move.direction) {
    case Move::UP:
      assert(blank_y + 1 != m_grid_h);
      std::swap(m_grid[blank_tile], m_grid[(blank_y + 1) * m_grid_w + blank_x]);
      break;

    case Move::DOWN:
      assert(blank_y);
      std::swap(m_grid[blank_tile], m_grid[(blank_y - 1) * m_grid_w + blank_x]);
      break;

    case Move::LEFT:
      assert(blank_x + 1 != m_grid_w);
      std::swap(m_grid[blank_tile], m_grid[blank_y * m_grid_w + blank_x + 1]);
      break;

    case Move::RIGHT:
      assert(blank_x);
      std::swap(m_grid[blank_tile], m_grid[blank_y * m_grid_w + blank_x - 1]);
      break;

    default:
      abort();
    }

    return std::make_pair(-1.0, -1.0);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Sliding Puzzle:" << endl;
    for(int64_t j = 0; j != m_grid_h; ++j) {
      for(int64_t i = 0; i != m_grid_w; ++i) {
        os << ' ' << m_grid[j * m_grid_w + i];
      }
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env_)
   : Carli::Agent(env_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);})
  {
    generate_rete();
    generate_features();
  }

  Agent::~Agent() {
//    std::cerr << "Feature Generation Time: " << m_feature_generation_time << std::endl;

    destroy();
  }

  void Agent::generate_rete() {
    std::string rules_in = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
    if(rules_in == "default")
      rules_in = "rules/sliding-puzzle.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
//    using dseconds = std::chrono::duration<double, std::ratio<1,1>>;
//    const auto start_time = std::chrono::high_resolution_clock::now();

    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &grid = env->get_grid();
    const auto &grid_w = env->get_grid_w();
    const auto &grid_h = env->get_grid_h();

    const int64_t blank_tile = int64_t(std::distance(grid.begin(), std::find(grid.begin(), grid.end(), 0)));
    const int64_t blank_x = blank_tile % grid_w;
    const int64_t blank_y = blank_tile / grid_w;

    for(int64_t tile = 0; tile != int64_t(env->get_grid().size()); ++tile) {
      if(!m_tile_names[tile])
        m_tile_names[tile] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(tile));
    }

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      Rete::Agenda::Locker locker(agenda);
      clear_old_wmes();
    }

    Rete::Agenda::Locker locker(agenda);

    if(blank_y + 1 != grid_h) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_up_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_up_id, m_move_direction_attr, m_move_direction_up));
    }
    if(blank_y) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_down_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_down_id, m_move_direction_attr, m_move_direction_down));
    }
    if(blank_x + 1 != grid_w) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_left_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_left_id, m_move_direction_attr, m_move_direction_left));
    }
    if(blank_x) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_right_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_right_id, m_move_direction_attr, m_move_direction_right));
    }

    clear_old_wmes();
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
  }

}
