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

  int64_t Environment::target_index(const int64_t &tile) const {
    if(tile)
      return tile - 1;
    else
      return m_grid_w * m_grid_h - 1;
  }

  int64_t Environment::grid_index(const Grid &grid, const int64_t &tile) const {
    return int64_t(std::distance(grid.begin(), std::find(grid.begin(), grid.end(), tile)));
  }

  std::pair<int64_t, int64_t> Environment::target_pos(const int64_t &tile) const {
    const int64_t index = target_index(tile);
    return std::make_pair(index % m_grid_w, index / m_grid_w);
  }

  std::pair<int64_t, int64_t> Environment::grid_pos(const Grid &grid, const int64_t &tile) const {
    const int64_t index = grid_index(grid, tile);
    return std::make_pair(index % m_grid_w, index / m_grid_w);
  }

  int64_t Environment::inversions_column(const Grid &grid, const int64_t &column, const int64_t &starting_row) const {
    int64_t inversions = 0;
    std::set<int64_t> numbers;
    for(int64_t j = starting_row; j != m_grid_h; ++j) {
      if(!grid[j * m_grid_w + column] || (grid[j * m_grid_w + column] - 1) % m_grid_w != column)
        continue;
      const auto ub = numbers.upper_bound(grid[j * m_grid_w + column]);
      inversions += std::distance(ub, numbers.end());
      numbers.insert(grid[j * m_grid_w + column]);
    }
    return inversions;
  }

  int64_t Environment::inversions_row(const Grid &grid, const int64_t &row, const int64_t &starting_column) const {
    int64_t inversions = 0;
    std::set<int64_t> numbers;
    for(int64_t i = starting_column; i != m_grid_w; ++i) {
      if(!grid[row * m_grid_w + i] || (grid[row * m_grid_w + i] - 1) / m_grid_w != row)
        continue;
      const auto ub = numbers.upper_bound(grid[row * m_grid_w + i]);
      inversions += std::distance(ub, numbers.end());
      numbers.insert(grid[row * m_grid_w + i]);
    }
    return inversions;
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

  bool Environment::success(const Environment::Grid &grid) const {
    for(int64_t i = 1; i <= m_grid_w; ++i) {
      if(i == m_grid_w)
        if(grid[2 * m_grid_w - 1] == i)
          break;
      if(grid[i - 1] != i)
        return false;
    }
    //for(int64_t i = 1; i != m_grid_w * m_grid_h; ++i) {
    //  if(grid[i - 1] != i)
    //    return false;
    //}
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

      const int64_t manhattan = manhattan_distance(grid, fringe);
      const int64_t inversions = inversions_column(grid, m_grid_w - rps.first, m_grid_h - rps.second) +
                                 inversions_row(grid, m_grid_h - rps.second, m_grid_w - rps.first);

      return manhattan + 2 * inversions;
    }) :
    std::function<int64_t(const Grid &grid)>([this](const Grid &grid)->int64_t {
      //return 0;

      std::vector<int64_t> numbers(grid.size() - 1);
      for(int64_t i = 1; i != int64_t(grid.size()); ++i)
        numbers[i - 1] = i;

      const int64_t manhattan = manhattan_distance(grid, numbers);

      int64_t inversions = 0;
      for(int64_t i = 0; i != m_grid_w; ++i)
        inversions += inversions_column(grid, i);
      for(int64_t j = 0; j != m_grid_h; ++j)
        inversions += inversions_row(grid, j);

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
        //std::stack<std::shared_ptr<State>> states;
        //for(auto ss = state; ss; ss = ss->prev_state)
        //  states.push(ss);
        //int64_t step = 0;
        //while(!states.empty()) {
        //  auto ss = states.top();
        //  states.pop();

        //  auto grid = Gridpart_SSL_to_Grid(ss->grid);
        //  const std::pair<int64_t, int64_t> rps = remaining_problem_size(grid);
        //  std::cerr << "Step " << step++ << " -- " << ss->num_steps << "+" << ss->heuristic << " -- " << rps.first << 'x' << rps.second << std::endl;
        //  for(int64_t j = 0; j != m_grid_h; ++j) {
        //    std::cerr << ' ';
        //    for(int64_t i = 0; i != m_grid_w; ++i) {
        //      std::cerr << ' ' << grid[j * m_grid_w + i];
        //    }
        //    std::cerr << std::endl;
        //  }
        //}
        //std::cerr << "Num steps = " << num_steps << std::endl;

        num_steps = state->num_steps;
        break;
      }

      const Grid grid = Gridpart_SSL_to_Grid(state->grid);
      const auto blank = grid_pos(grid, 0);
      const auto rps = remaining_problem_size(grid);

      if(approximate) {
        if(rps.first > rps_min.first || rps.second > rps_min.second)
          continue;
      }

      if(blank.second + 1 != m_grid_h) {
        Grid next = grid;
        std::swap(next[blank.second * m_grid_w + blank.first], next[(blank.second + 1) * m_grid_w + blank.first]);
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
      if(blank.second + rps.second > m_grid_h) {
        Grid next = grid;
        std::swap(next[blank.second * m_grid_w + blank.first], next[(blank.second - 1) * m_grid_w + blank.first]);
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
      if(blank.first + 1 != m_grid_w) {
        Grid next = grid;
        std::swap(next[blank.second * m_grid_w + blank.first], next[blank.second * m_grid_w + blank.first + 1]);
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
      if(blank.first + rps.first > m_grid_w) {
        Grid next = grid;
        std::swap(next[blank.second * m_grid_w + blank.first], next[blank.second * m_grid_w + blank.first - 1]);
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

    const auto blank = grid_pos(m_grid, 0);

    switch(move.direction) {
    case Move::UP:
      assert(blank.second + 1 != m_grid_h);
      std::swap(m_grid[blank.second * m_grid_w + blank.first], m_grid[(blank.second + 1) * m_grid_w + blank.first]);
      break;

    case Move::DOWN:
      assert(blank.second);
      std::swap(m_grid[blank.second * m_grid_w + blank.first], m_grid[(blank.second - 1) * m_grid_w + blank.first]);
      break;

    case Move::LEFT:
      assert(blank.first + 1 != m_grid_w);
      std::swap(m_grid[blank.second * m_grid_w + blank.first], m_grid[blank.second * m_grid_w + blank.first + 1]);
      break;

    case Move::RIGHT:
      assert(blank.first);
      std::swap(m_grid[blank.second * m_grid_w + blank.first], m_grid[blank.second * m_grid_w + blank.first - 1]);
      break;

    default:
      abort();
    }

    return std::make_pair(success() ? 100.0 : -1.0, -1.0);
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
    : Carli::Agent(env_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token); })
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

    const auto blank = env->grid_pos(grid, 0);
    const auto rps = env->remaining_problem_size(grid);

    std::set<int64_t> fringe_top;
    for(int64_t i = grid_w - rps.first; i != grid_w; ++i)
      fringe_top.insert((grid_h - rps.second) * grid_w + i + 1);
    fringe_top.erase(grid_w * grid_h);

    std::set<int64_t> fringe_left;
    for(int64_t j = grid_h - rps.second; j != grid_h; ++j)
      fringe_left.insert(j * grid_w + (grid_w - rps.first) + 1);
    fringe_left.erase(grid_w * grid_h);

    const auto top_left_ccw_dist =
      [this,&env,&grid_w,&grid_h]
      (const Environment::Grid &grid, const std::pair<int64_t, int64_t> &rps)->int64_t
    {
      const int64_t top_left = (grid_h - rps.second + 1) * grid_w - rps.first + 1;
      const auto pos = env->grid_pos(grid, top_left);

      if(pos.second == grid_h - rps.second)
        return pos.first - (grid_w - rps.first);
      else
        return rps.first + env->distance(pos, env->grid_pos(grid, (grid_h - rps.second + 2) * grid_w));
    };

    const auto top_right_cw_dist =
      [this,&env,&grid_w,&grid_h]
      (const Environment::Grid &grid, const std::pair<int64_t, int64_t> &rps)->int64_t
    {
      const int64_t top_right = (grid_h - rps.second + 1) * grid_w;
      const auto pos = env->grid_pos(grid, top_right);

      if(pos.second == grid_h - rps.second)
        return grid_w - pos.first;
      else
        return rps.first + env->distance(pos, env->grid_pos(grid, (grid_h - rps.second + 2) * grid_w - rps.first + 1));
    };

    const auto left_top_cw_dist =
      [this,&env,&grid_w,&grid_h]
      (const Environment::Grid &grid, const std::pair<int64_t, int64_t> &rps)->int64_t
    {
      const int64_t left_top = (grid_h - rps.second + 1) * grid_w - rps.first + 1;
      const auto pos = env->grid_pos(grid, left_top);

      if(pos.first == grid_w - rps.first)
        return pos.second - (grid_h - rps.second);
      else
        return rps.second + env->distance(pos, env->grid_pos(grid, grid_h * grid_w - rps.first + 2));
    };

    const auto left_bottom_ccw_dist =
      [this,&env,&grid_w,&grid_h]
      (const Environment::Grid &grid, const std::pair<int64_t, int64_t> &rps)->int64_t
    {
      const int64_t left_bottom = grid_h * grid_w - rps.first + 1;
      const auto pos = env->grid_pos(grid, left_bottom);

      if(pos.first == grid_w - rps.first)
        return grid_h - pos.second;
      else
        return rps.second + env->distance(pos, env->grid_pos(grid, (grid_h - rps.second + 1) * grid_w - rps.first + 2));
    };

    const int64_t top_snake_manhattan = snake_manhattan(*env, grid, fringe_top);
    const int64_t left_snake_manhattan = snake_manhattan(*env, grid, fringe_left);

    const int64_t top_snake_dtb = snake_dist_to_blank(*env, grid, fringe_top);
    const int64_t left_snake_dtb = snake_dist_to_blank(*env, grid, fringe_left);

    const int64_t top_left_ccw = top_left_ccw_dist(grid, rps);
    const int64_t top_right_cw = top_right_cw_dist(grid, rps);
    const int64_t left_top_cw = left_top_cw_dist(grid, rps);
    const int64_t left_bottom_ccw = left_bottom_ccw_dist(grid, rps);

    //int64_t moves_to_blank_tlccw = 0;
    //if(top_left_ccw && top_left_ccw <= rps.first)
    //  moves_to_blank_tlccw = moves_to_blank(*env, grid, std::make_pair(grid_w - top_left_ccw, grid_h - rps.second), fringe_top);
    //else if(top_left_ccw > rps.first) {
    //  const auto pos = env->grid_pos(grid, *fringe_top.begin());
    //  if(pos.first + 1 != grid_w)
    //    moves_to_blank_tlccw = moves_to_blank(*env, grid, std::make_pair(pos.first + 1, pos.second), fringe_top);
    //  if(pos.second != grid_h - rps.second)
    //    moves_to_blank_tlccw = moves_to_blank(*env, grid, std::make_pair(pos.first, pos.second - 1), fringe_top);
    //}

    //int64_t moves_to_blank_trcc = 0;
    //int64_t moves_to_blank_ltcw = 0;
    //int64_t moves_to_blank_lbccw = 0;

    const std::tuple<int64_t, int64_t, int64_t> lwccw_snake1_lendistblank = top_left_ccw_snake_lendistblank(*env, grid, rps, fringe_top);
    std::tuple<int64_t, int64_t, int64_t> lwccw_snake2_lendistblank;
    if(std::get<0>(lwccw_snake1_lendistblank) < int64_t(fringe_top.size())) {
      const Environment::Grid distances = top_left_ccw_distances(*env, rps);
      Environment::Grid distances_rest = distances_rps_init(*env, rps);
      auto rest = fringe_top;
      for(int64_t i = std::get<0>(lwccw_snake1_lendistblank); i; --i) {
        const int64_t tile = *rest.begin();
        const int64_t index = env->grid_index(grid, tile);
        distances_rest[index] = -1;

        if(i == 1) {
          const int64_t target_dist = distances[index] + 1;
          for(int64_t k = 0; k != grid_w * grid_h; ++k) {
            if(distances[k] == target_dist)
              distances_rest[k] = 0;
          }
        }

        rest.erase(tile);
      }
      transitive_closure_distances(distances_rest, *env);
      lwccw_snake2_lendistblank = snake_lendistblank(*env, grid, rps, distances_rest, rest);
    }

    for(int64_t tile = 0; tile != int64_t(env->get_grid().size()); ++tile) {
      if(!m_tile_names[tile])
        m_tile_names[tile] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(tile));
    }

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      Rete::Agenda::Locker locker(agenda);
      clear_old_wmes();
    }

    Rete::Agenda::Locker locker(agenda);

    const auto generate_relative_attribute =
      [this]
      (const int64_t &current, const int64_t &next, const Rete::Symbol_Identifier_Ptr_C &id, const Rete::Symbol_Constant_String_Ptr_C &attr)
    {
      if(next > current)
        insert_new_wme(std::make_shared<Rete::WME>(id, attr, m_increases));
      else if(next < current)
        insert_new_wme(std::make_shared<Rete::WME>(id, attr, m_decreases));
      else
        insert_new_wme(std::make_shared<Rete::WME>(id, attr, m_unchanged));
    };

    const auto generate_action =
      [this, &env, &generate_relative_attribute,
       &grid, &grid_w, &grid_h, &blank, &rps,
       &fringe_top, &fringe_left,
       &top_left_ccw_dist, &top_right_cw_dist, &left_top_cw_dist, &left_bottom_ccw_dist,
       &top_snake_manhattan, &left_snake_manhattan,
       &top_snake_dtb, &left_snake_dtb,
       &top_left_ccw, &top_right_cw, &left_top_cw, &left_bottom_ccw,
       //&moves_to_blank_tlccw,&moves_to_blank_trcc,&moves_to_blank_ltcw,&moves_to_blank_lbccw
       &lwccw_snake1_lendistblank,&lwccw_snake2_lendistblank
      ]
      (const std::pair<int64_t,int64_t> &pos, const Rete::Symbol_Identifier_Ptr_C &move_id, const Rete::Symbol_Constant_Int_Ptr_C &move_name)
    {
      Environment::Grid grid_next = env->get_grid();
      std::swap(grid_next[blank.second * grid_w + blank.first], grid_next[pos.second * grid_w + pos.first]);

      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, move_id));
      insert_new_wme(std::make_shared<Rete::WME>(move_id, m_move_direction_attr, move_name));

      //const auto rps_next = env->remaining_problem_size(grid_next);
      //if(rps_next.first > rps.first || rps_next.second > rps.second)
      //  insert_new_wme(std::make_shared<Rete::WME>(move_id, m_dimensionality_attr, m_increases));
      //else if(rps_next.first < rps.first || rps_next.second < rps.second)
      //  insert_new_wme(std::make_shared<Rete::WME>(move_id, m_dimensionality_attr, m_decreases));
      //else
      //  insert_new_wme(std::make_shared<Rete::WME>(move_id, m_dimensionality_attr, m_unchanged));

      const std::tuple<int64_t, int64_t, int64_t> lwccw_snake1_lendistblank_next = top_left_ccw_snake_lendistblank(*env, grid_next, rps, fringe_top);
      std::tuple<int64_t, int64_t, int64_t> lwccw_snake2_lendistblank_next;
      if(std::get<0>(lwccw_snake1_lendistblank_next) < int64_t(fringe_top.size())) {
        const Environment::Grid distances = top_left_ccw_distances(*env, rps);
        Environment::Grid distances_rest = distances_rps_init(*env, rps);
        auto rest = fringe_top;
        for(int64_t i = std::get<0>(lwccw_snake1_lendistblank_next); i; --i) {
          const int64_t tile = *rest.begin();
          const int64_t index = env->grid_index(grid_next, tile);
          distances_rest[index] = -1;

          if(i == 1) {
            const int64_t target_dist = distances[index] + 1;
            for(int64_t k = 0; k != grid_w * grid_h; ++k) {
              if(distances[k] == target_dist)
                distances_rest[k] = 0;
            }
          }

          rest.erase(tile);
        }
        transitive_closure_distances(distances_rest, *env);
        lwccw_snake2_lendistblank_next = snake_lendistblank(*env, grid_next, rps, distances_rest, rest);
      }

      generate_relative_attribute(std::get<0>(lwccw_snake1_lendistblank), std::get<0>(lwccw_snake1_lendistblank_next), move_id, m_snake1_length_attr);
      generate_relative_attribute(std::get<0>(lwccw_snake2_lendistblank), std::get<0>(lwccw_snake2_lendistblank_next), move_id, m_snake2_length_attr);
      generate_relative_attribute(std::get<1>(lwccw_snake1_lendistblank), std::get<1>(lwccw_snake1_lendistblank_next), move_id, m_snake1_dist_attr);
      generate_relative_attribute(std::get<1>(lwccw_snake2_lendistblank), std::get<1>(lwccw_snake2_lendistblank_next), move_id, m_snake2_dist_attr);
      generate_relative_attribute(std::get<2>(lwccw_snake1_lendistblank), std::get<2>(lwccw_snake1_lendistblank_next), move_id, m_snake1_blank_attr);
      generate_relative_attribute(std::get<2>(lwccw_snake2_lendistblank), std::get<2>(lwccw_snake2_lendistblank_next), move_id, m_snake2_blank_attr);

      //generate_relative_attribute(top_snake_manhattan, snake_manhattan(*env, grid_next, fringe_top), move_id, m_top_snake_manhattan_attr);
      //generate_relative_attribute(left_snake_manhattan, snake_manhattan(*env, grid_next, fringe_left), move_id, m_left_snake_manhattan_attr);

      //generate_relative_attribute(top_snake_dtb, snake_dist_to_blank(*env, grid_next, fringe_top), move_id, m_top_snake_dist_to_blank_attr);
      //generate_relative_attribute(left_snake_dtb, snake_dist_to_blank(*env, grid_next, fringe_left), move_id, m_left_snake_dist_to_blank_attr);

      //const int64_t tlccwd_next = top_left_ccw_dist(grid_next, rps_next);
      //const int64_t trcwd_next = top_right_cw_dist(grid_next, rps_next);
      //const int64_t ltcwd_next = left_top_cw_dist(grid_next, rps_next);
      //const int64_t lbccwd_next = left_bottom_ccw_dist(grid_next, rps_next);

      //generate_relative_attribute(top_left_ccw, tlccwd_next, move_id, m_top_left_ccw_dist_attr);
      //generate_relative_attribute(top_right_cw, trcwd_next, move_id, m_top_right_cw_dist_attr);
      //generate_relative_attribute(left_top_cw, ltcwd_next, move_id, m_left_top_cw_dist_attr);
      //generate_relative_attribute(left_bottom_ccw, lbccwd_next, move_id, m_left_bottom_ccw_dist_attr);

      //int64_t mtb_tl_ccw_next = 0;
      //if(tlccwd_next && tlccwd_next <= rps_next.first)
      //  mtb_tl_ccw_next = moves_to_blank(*env, grid_next, std::make_pair(grid_w - tlccwd_next, grid_h - rps_next.second), fringe_top);
      //else if(tlccwd_next > rps_next.first) {
      //  const auto target_blank = env->grid_pos(grid_next, *fringe_top.begin());
      //  if(target_blank.first + 1 != grid_w)
      //    mtb_tl_ccw_next = moves_to_blank(*env, grid_next, std::make_pair(target_blank.first + 1, blank.second), fringe_top);
      //  if(target_blank.second != grid_h - rps_next.second)
      //    mtb_tl_ccw_next = moves_to_blank(*env, grid_next, std::make_pair(target_blank.first, blank.second - 1), fringe_top);
      //}

      //generate_relative_attribute(moves_to_blank_tlccw, mtb_tl_ccw_next, move_id, m_moves_to_blank_tl_ccw_attr);
      //generate_relative_attribute(moves_to_blank_trcc, mtb_tr_cw_next, move_id, m_moves_to_blank_tr_cw_attr);
      //generate_relative_attribute(moves_to_blank_ltcw, mtb_lt_cw_next, move_id, m_moves_to_blank_lt_cw_attr);
      //generate_relative_attribute(moves_to_blank_lbccw, mtb_lb_ccw_next, move_id, m_moves_to_blank_lb_ccw_attr);
    };

    if(blank.second + 1 != grid_h)
      generate_action(std::make_pair(blank.first, blank.second + 1), m_move_up_id, m_move_direction_up);
    if(blank.second)
      generate_action(std::make_pair(blank.first, blank.second - 1), m_move_down_id, m_move_direction_down);
    if(blank.first + 1 != grid_w)
      generate_action(std::make_pair(blank.first + 1, blank.second), m_move_left_id, m_move_direction_left);
    if(blank.first)
      generate_action(std::make_pair(blank.first - 1, blank.second), m_move_right_id, m_move_direction_right);

    clear_old_wmes();
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
  }

}
