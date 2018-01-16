#ifndef BLOCKS_WORLD_2_H
#define BLOCKS_WORLD_2_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <list>
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

    std::pair<int64_t, int64_t> target_pos(const int64_t &number) const;
    std::pair<int64_t, int64_t> grid_pos(const Grid &grid, const int64_t &number) const;

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
    int64_t snake_manhattan(const Environment &env, const Environment::Grid &grid, const TYPE &numbers) const {
      int64_t manhattan = 0;
      const auto blank = env.grid_pos(grid, 0);
      for(auto nt = numbers.begin(), n2 = ++numbers.begin(); n2 != numbers.end(); nt = n2, ++n2) {
        const auto ntpos = env.grid_pos(grid, *nt);
        const auto n2pos = env.grid_pos(grid, *n2);

        const int64_t dist_through_blank = env.distance(ntpos, blank) + env.distance(blank, n2pos);
        int64_t dist = env.distance(ntpos, n2pos);
        if(dist_through_blank == dist)
          --dist; /// If blank tile inbetween, reduce distance by 1
        --dist; /// Want adjacent, not overlapping

        manhattan += dist;
      }
      return manhattan;
    }

    template <typename TYPE>
    int64_t snake_dist_to_blank(const Environment &env, const Environment::Grid &grid, const TYPE &numbers) const {
      int64_t manhattan = std::numeric_limits<int64_t>::max();
      const auto blank = env.grid_pos(grid, 0);
      for(auto nt = numbers.begin(), n2 = ++numbers.begin(); n2 != numbers.end(); nt = n2, ++n2) {
        const auto ntpos = env.grid_pos(grid, *nt);
        const auto n2pos = env.grid_pos(grid, *n2);

        const int64_t dist_through_blank = env.distance(ntpos, blank) + env.distance(blank, n2pos);
        const int64_t dist = env.distance(ntpos, n2pos);
        manhattan = std::min(manhattan, (dist_through_blank - dist) / 2);
      }
      return manhattan;
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
