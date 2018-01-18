#ifndef BLOCKS_WORLD_2_H
#define BLOCKS_WORLD_2_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <list>
#include <map>
#include <queue>
#include <stdexcept>

#if !defined(_WINDOWS)
#define SLIDING_PUZZLE_LINKAGE
#elif !defined(SLIDING_PUZZLE_INTERNAL)
#define SLIDING_PUZZLE_LINKAGE __declspec(dllimport)
#else
#define SLIDING_PUZZLE_LINKAGE __declspec(dllexport)
#endif

namespace Sliding_Puzzle {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class SLIDING_PUZZLE_LINKAGE Move : public Carli::Action {
  public:
    enum Direction {UP = 1, DOWN = 2, LEFT = 3, RIGHT = 4};

    Move()
     : direction(Direction())
    {
    }

    Move(const Direction &direction_)
     : direction(direction_)
    {
    }

    Move(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : direction(Direction(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("move-direction")->second]).value))
    {
      assert(direction >= UP && direction <= RIGHT);
    }

    Move * clone() const {
      return new Move(direction);
    }

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Move &>(rhs));
    }

    int64_t compare(const Move &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "move(" << direction << ')';
    }

    Direction direction;
  };

  class SLIDING_PUZZLE_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &);
    Environment & operator=(const Environment &);

  public:
    Environment();

    typedef std::vector<int64_t> Grid;

    const Grid & get_grid() const { return m_grid; }
    int64_t get_grid_w() const { return m_grid_w; }
    int64_t get_grid_h() const { return m_grid_h; }

    bool success() const;

    bool supports_optimal() const { return true; }
    double optimal_reward() const { return double(-m_num_steps_to_goal); }

    int64_t target_index(const int64_t &tile) const;
    int64_t grid_index(const Grid &grid, const int64_t &tile) const;
    std::pair<int64_t, int64_t> target_pos(const int64_t &tile) const;
    std::pair<int64_t, int64_t> grid_pos(const Grid &grid, const int64_t &tile) const;

    template <typename TYPE>
    int64_t manhattan_distance(const Grid &grid, const TYPE &numbers) const {
      int64_t manhattan = 0;
      for(int64_t i : numbers) {
        const auto gp = grid_pos(grid, i);
        const auto tp = target_pos(i);
        manhattan += distance(gp, tp);
      }
      return manhattan;
    }

    static int64_t distance(const std::pair<int64_t, int64_t> &lhs, const std::pair<int64_t, int64_t> &rhs) {
      return std::abs(lhs.first - rhs.first) + std::abs(lhs.second - rhs.second);
    }

    int64_t inversions_column(const Grid &grid, const int64_t &column, const int64_t &starting_row = 0) const;
    int64_t inversions_row(const Grid &grid, const int64_t &row, const int64_t &starting_column = 0) const;

    std::pair<int64_t, int64_t> remaining_problem_size(const Grid &grid) const;

  private:
    bool success(const Grid &grid) const;
    int64_t calculate_num_steps_to_goal() const;

    bool solveable(const Grid &grid, const int64_t &initial_x = 0, const int64_t &initial_y = 0) const;

    void init_impl();

    std::pair<reward_type, reward_type> transition_impl(const Carli::Action &action);

    void print_impl(ostream &os) const;

    Zeni::Random m_random;
    Grid m_grid;
    const int64_t m_grid_w = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["grid-width"]).get_value();
    const int64_t m_grid_h = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["grid-height"]).get_value();

    const bool m_evaluate_optimality = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["evaluate-optimality"]).get_value() && supports_optimal() && dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value() != "null";
    int64_t m_num_steps_to_goal = 0;
  };

  class SLIDING_PUZZLE_LINKAGE Agent : public Carli::Agent {
    Agent(const Agent &);
    Agent & operator=(const Agent &);

  public:
    Agent(const std::shared_ptr<Carli::Environment> &env);
    ~Agent();

  private:
    void generate_rete();

    void generate_features();

    void update();

    Zeni::Random m_random;

    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action"));
    const Rete::Symbol_Constant_String_Ptr_C m_dimensionality_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("dimensionality"));
    const Rete::Symbol_Constant_String_Ptr_C m_top_snake_manhattan_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-snake-manhattan"));
    const Rete::Symbol_Constant_String_Ptr_C m_left_snake_manhattan_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-snake-manhattan"));
    const Rete::Symbol_Constant_String_Ptr_C m_top_snake_dist_to_blank_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-snake-dist-to-blank"));
    const Rete::Symbol_Constant_String_Ptr_C m_left_snake_dist_to_blank_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-snake-dist-to-blank"));
    const Rete::Symbol_Constant_String_Ptr_C m_top_left_ccw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-left-ccw-dist"));
    const Rete::Symbol_Constant_String_Ptr_C m_top_right_cw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-right-cw-dist"));
    const Rete::Symbol_Constant_String_Ptr_C m_left_top_cw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-top-cw-dist"));
    const Rete::Symbol_Constant_String_Ptr_C m_left_bottom_ccw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-bottom-ccw-dist"));
    const Rete::Symbol_Constant_String_Ptr_C m_moves_to_blank_tl_ccw_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("moves-to-blank-tl-ccw"));
    const Rete::Symbol_Constant_String_Ptr_C m_move_direction_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("move-direction"));
    const Rete::Symbol_Identifier_Ptr_C m_move_up_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-up"));
    const Rete::Symbol_Identifier_Ptr_C m_move_down_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-down"));
    const Rete::Symbol_Identifier_Ptr_C m_move_left_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-left"));
    const Rete::Symbol_Identifier_Ptr_C m_move_right_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-right"));
    const Rete::Symbol_Constant_Int_Ptr_C m_increases = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_decreases = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_unchanged = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_Int_Ptr_C m_move_direction_up = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_move_direction_down = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_move_direction_left = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_Int_Ptr_C m_move_direction_right = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(4));

    std::map<int64_t, Rete::Symbol_Constant_Int_Ptr_C> m_tile_names;

    template <typename TYPE>
    int64_t snake_manhattan(const Environment &env, const Environment::Grid &grid, const TYPE &tiles) const {
      int64_t manhattan = 0;
      const auto blank = env.grid_pos(grid, 0);
      for(auto tt = tiles.begin(), t2 = ++tiles.begin(); t2 != tiles.end(); tt = t2, ++t2) {
        const auto ttpos = env.grid_pos(grid, *tt);
        const auto t2pos = env.grid_pos(grid, *t2);

        const int64_t dist_through_blank = env.distance(ttpos, blank) + env.distance(blank, t2pos);
        int64_t dist = env.distance(ttpos, t2pos);
        if(dist_through_blank == dist)
          --dist; /// If blank tile inbetween, reduce distance by 1
        --dist; /// Watt adjacett, not overlapping

        manhattan += dist;
      }
      return manhattan;
    }

    template <typename TYPE>
    int64_t snake_dist_to_blank(const Environment &env, const Environment::Grid &grid, const TYPE &tiles) const {
      int64_t manhattan = std::numeric_limits<int64_t>::max();
      const auto blank = env.grid_pos(grid, 0);
      for(auto tt = tiles.begin(), t2 = ++tiles.begin(); t2 != tiles.end(); tt = t2, ++t2) {
        const auto ttpos = env.grid_pos(grid, *tt);
        const auto t2pos = env.grid_pos(grid, *t2);

        const int64_t dist_through_blank = env.distance(ttpos, blank) + env.distance(blank, t2pos);
        const int64_t dist = env.distance(ttpos, t2pos);
        manhattan = std::min(manhattan, (dist_through_blank - dist) / 2);
      }
      return manhattan;
    }

    template <typename TYPE>
    std::pair<int64_t, int64_t> top_left_ccw_snake_distlen(const Environment &env, const Environment::Grid &grid, const std::pair<int64_t, int64_t> &rps, const TYPE &tiles) const {
      return snake_distlen(env, grid, top_left_ccw_distances(env, rps), tiles);
    }

    template <typename TYPE>
    std::pair<int64_t, int64_t> snake_distlen(const Environment &env, const Environment::Grid &grid, const Environment::Grid &distances, const TYPE &tiles) const {
      int64_t index = env.grid_index(grid, *tiles.begin());
      int64_t dist = distances[index];

      std::pair<int64_t, int64_t> distlen(dist, 1);

      for(auto tt = ++tiles.begin(); tt != tiles.end(); ++tt) {
        const int64_t index_next = env.grid_index(grid, *tt);
        const int64_t dist_next = distances[index_next];
        if(dist + 1 != dist_next || std::abs((index % env.get_grid_w()) - (index_next % env.get_grid_w())) + std::abs(index / env.get_grid_w() - index_next / env.get_grid_w()) != 1)
          break;
        index = index_next;
        dist = dist_next;
        ++distlen.second;
      }

      return distlen;
    }

    int64_t distances_to_moves(const Environment &env, const Environment::Grid &grid, const Environment::Grid &distances, const int64_t &tile) const {
      return distances[env.grid_index(grid, tile)];
    }

    Environment::Grid top_left_ccw_distances(const Environment &env, const std::pair<int64_t, int64_t> &rps) const {
      Environment::Grid distances = distances_rps_init(env, rps);

      for(int64_t i = 0; i != rps.first; ++i)
        distances[(env.get_grid_h() - rps.second + 1) * env.get_grid_w() - rps.first + i] = i;

      if(rps.second > 1) {
        for(int64_t i = 0; i != rps.first; ++i)
          distances[(env.get_grid_h() - rps.second + 2) * env.get_grid_w() - i - 1] = rps.first + i;
      }

      transitive_closure_distances(distances, env);
      return distances;
    }

    Environment::Grid distances_rps_init(const Environment &env, const std::pair<int64_t, int64_t> &rps) const {
      Environment::Grid distances(env.get_grid_w() * env.get_grid_h(), std::numeric_limits<int64_t>::max());

      for(int64_t i = 0; i != env.get_grid_w(); ++i) {
        for(int64_t j = 0; j != env.get_grid_h(); ++j) {
          if(i < env.get_grid_w() - rps.first || j < env.get_grid_h() - rps.second)
            distances[j  * env.get_grid_w() + i] = -1;
        }
      }

      return distances;
    }

    void transitive_closure_distances(Environment::Grid &distances, const Environment &env) const {
      std::multimap<int64_t, int64_t> dist_to_indices;
      for(int64_t i = 0; i != int64_t(env.get_grid_w() * env.get_grid_h()); ++i) {
        if(distances[i] >= 0 && distances[i] != std::numeric_limits<int64_t>::max())
          dist_to_indices.insert(std::make_pair(distances[i], i));
      }

      while(!dist_to_indices.empty()) {
        const std::pair<int64_t, int64_t> dist_and_index = *dist_to_indices.begin();
        dist_to_indices.erase(dist_to_indices.begin());

        const int64_t x = dist_and_index.second % env.get_grid_w();
        const int64_t y = dist_and_index.second / env.get_grid_w();

        if(y && distances[(y - 1) * env.get_grid_w() + x] == std::numeric_limits<int64_t>::max()) {
          distances[(y - 1) * env.get_grid_w() + x] = dist_and_index.first + 1;
          dist_to_indices.insert(std::make_pair(dist_and_index.first + 1, ((y - 1) * env.get_grid_w() + x)));
        }
        if(y + 1 != env.get_grid_h() && distances[(y + 1) * env.get_grid_w() + x] == std::numeric_limits<int64_t>::max()) {
          distances[(y + 1) * env.get_grid_w() + x] = dist_and_index.first + 1;
          dist_to_indices.insert(std::make_pair(dist_and_index.first + 1, ((y + 1) * env.get_grid_w() + x)));
        }
        if(x && distances[y * env.get_grid_w() + x - 1] == std::numeric_limits<int64_t>::max()) {
          distances[y * env.get_grid_w() + x - 1] = dist_and_index.first + 1;
          dist_to_indices.insert(std::make_pair(dist_and_index.first + 1, (y * env.get_grid_w() + x - 1)));
        }
        if(x + 1 != env.get_grid_w() && distances[y * env.get_grid_w() + x + 1] == std::numeric_limits<int64_t>::max()) {
          distances[y * env.get_grid_w() + x + 1] = dist_and_index.first + 1;
          dist_to_indices.insert(std::make_pair(dist_and_index.first + 1, (y * env.get_grid_w() + x + 1)));
        }
      }
    }

    std::unordered_map<Rete::WME_Ptr_C, bool, Rete::hash_deref<Rete::WME>, Rete::compare_deref_eq> m_wmes;

    void clear_old_wmes() {
      auto wt = m_wmes.begin(), wend = m_wmes.end();
      while(wt != wend) {
        if(wt->second)
          wt++->second = false;
        else {
          {
            CPU_Accumulator cpu_accumulator(*this);
            remove_wme(wt->first);
          }
          wt = m_wmes.erase(wt);
        }
      }
    }

    void insert_new_wme(const Rete::WME_Ptr_C &wme) {
      const auto found = m_wmes.find(wme);
      if(found == m_wmes.end()) {
        m_wmes[wme] = true;
        CPU_Accumulator cpu_accumulator(*this);
        insert_wme(wme);
      }
      else
        found->second = true;
    }

//    double m_feature_generation_time = 0.0;
  };

}

#endif
