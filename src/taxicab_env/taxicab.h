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
#define TAXICAB_LINKAGE
#elif !defined(TAXICAB_INTERNAL)
#define TAXICAB_LINKAGE __declspec(dllimport)
#else
#define TAXICAB_LINKAGE __declspec(dllexport)
#endif

namespace Taxicab {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class TAXICAB_LINKAGE Action : public Carli::Action {
  public:
    enum Type { MOVE = 1, REFUEL = 2, PICKUP = 3, PUTDOWN = 4 };
    enum Direction { NONE = 0, NORTH = 1, SOUTH = 2, EAST = 3, WEST = 4 };

    Action()
      : type(Type()), direction(Direction())
    {
    }

    Action(const Type &type_, const Direction &direction_)
      : type(type_), direction(direction_)
    {
    }

    Action(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
      : type(Type(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("type")->second]).value)),
      direction(type == MOVE ? Direction(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("direction")->second]).value) : Direction())
    {
    }

    Action * clone() const {
      return new Action(type, direction);
    }

    int64_t compare(const Carli::Action &rhs) const {
      return compare(debuggable_cast<const Action &>(rhs));
    }

    int64_t compare(const Action &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "action(" << type << ", " << direction << ')';
    }

    Type type;
    Direction direction;
  };

  class TAXICAB_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &);
    Environment & operator=(const Environment &);

  public:
    enum Passenger { AT_SOURCE = 1, ONBOARD = 2, AT_DESTINATION = 3 };

    struct State : public std::enable_shared_from_this<State> {
      State(const std::pair<int64_t, int64_t> &taxi_position_, const int64_t &fuel_, const Passenger &passenger_,
        const int64_t &num_steps_, const int64_t &heuristic_, const std::shared_ptr<const State> &prev_state_ = nullptr)
        : prev_state(prev_state_), taxi_position(taxi_position_), fuel(fuel_), passenger(passenger_), num_steps(num_steps_), heuristic(heuristic_)
      {
      }

      bool operator<(const State &rhs) const {
        return num_steps + heuristic < rhs.num_steps + rhs.heuristic ||
          (num_steps + heuristic == rhs.num_steps + rhs.heuristic &&
          (num_steps < rhs.num_steps ||
            (num_steps == rhs.num_steps &&
            (heuristic < rhs.heuristic ||
              (heuristic == rhs.heuristic &&
              (taxi_position < rhs.taxi_position ||
                (taxi_position == rhs.taxi_position &&
                (fuel < rhs.fuel ||
                  (fuel == rhs.fuel &&
                    passenger < rhs.passenger)))))))));
      }

      void print_solution(std::ostream &os) const {
        std::stack<std::shared_ptr<const State>> states;
        for(auto ss = shared_from_this(); ss; ss = ss->prev_state)
          states.push(ss);
        int64_t step = 0;
        while(!states.empty()) {
          auto ss = states.top();
          states.pop();

          os << "Step " << step++ << " -- " << ss->num_steps << "+" << ss->heuristic << " -- " << std::endl;
          os << "  (" << ss->taxi_position.first << ',' << ss->taxi_position.second << "), fuel=" << ss->fuel << ", passenger=" << ss->passenger << std::endl;
        }
        os << "Num steps = " << num_steps << std::endl;
      }

      std::shared_ptr<const State> prev_state;
      std::pair<int64_t, int64_t> taxi_position;
      int64_t fuel;
      Passenger passenger;
      int64_t num_steps;
      int64_t heuristic;
    };

    Environment();

    typedef std::vector<int64_t> Grid;

    int64_t get_grid_w() const { return m_grid_w; }
    int64_t get_grid_h() const { return m_grid_h; }
    const std::vector<std::pair<int64_t, int64_t>> & get_filling_stations() const { return m_filling_stations; }
    const std::vector<std::pair<int64_t, int64_t>> & get_destinations() const { return m_destinations; }
    const std::pair<int64_t, int64_t> & get_taxi_position() const { return m_taxi_position; }
    int64_t get_fuel() const { return m_fuel; }
    int64_t get_fuel_max() const { return m_fuel_max; }
    Passenger get_passenger() const { return m_passenger; }
    int64_t get_passenger_source() const { return m_passenger_source; }
    int64_t get_passenger_destination() const { return m_passenger_destination; }

    bool success() const;
    bool failure() const;

    bool supports_optimal() const { return true; }
    double optimal_reward() const { return double(-m_num_steps_to_goal); }

    std::pair<std::shared_ptr<const State>, int64_t> optimal_from(const std::pair<int64_t, int64_t> &taxi_position, const int64_t &fuel, const Passenger &passenger, const int64_t &source, const int64_t &destination) const;

    static int64_t distance(const std::pair<int64_t, int64_t> &lhs, const std::pair<int64_t, int64_t> &rhs) {
      return std::abs(lhs.first - rhs.first) + std::abs(lhs.second - rhs.second);
    }

  private:
    void init_impl();

    std::pair<reward_type, reward_type> transition_impl(const Carli::Action &action);

    void print_impl(ostream &os) const;

    bool solveable_fuel(Grid &distances_out) const;
    void transitive_closure_distances(Grid &distances) const;

    Zeni::Random m_random;

    const int64_t m_grid_w = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["grid-width"]).get_value();
    const int64_t m_grid_h = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["grid-height"]).get_value();
    const int64_t m_num_filling_stations = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["num-filling-stations"]).get_value();
    const int64_t m_num_destinations = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["num-destinations"]).get_value();

    std::vector<std::pair<int64_t, int64_t>> m_filling_stations;
    std::vector<std::pair<int64_t, int64_t>> m_destinations;

    std::pair<int64_t, int64_t> m_taxi_position;
    int64_t m_fuel;
    const int64_t m_fuel_max = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["fuel-max"]).get_value();

    Passenger m_passenger;
    int64_t m_passenger_source;
    int64_t m_passenger_destination;

    const bool m_evaluate_optimality = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["evaluate-optimality"]).get_value() && supports_optimal() && dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value() != "null";
    int64_t m_num_steps_to_goal = 0;
    std::shared_ptr<const State> m_optimal_solution;
    //std::shared_ptr<const State> m_solution;
  };

  class TAXICAB_LINKAGE Agent : public Carli::Agent {
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
    const Rete::Symbol_Constant_String_Ptr_C m_type_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("type"));
    const Rete::Symbol_Constant_String_Ptr_C m_direction_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("direction"));
    const Rete::Symbol_Constant_String_Ptr_C m_passenger_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("passenger"));
    const Rete::Symbol_Constant_String_Ptr_C m_passenger_source_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("passenger-source"));
    const Rete::Symbol_Constant_String_Ptr_C m_passenger_destination_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("passenger-destination"));
    const Rete::Symbol_Constant_String_Ptr_C m_toward_pickup_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("toward-pickup"));
    const Rete::Symbol_Constant_String_Ptr_C m_toward_dropoff_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("toward-dropoff"));
    const Rete::Symbol_Constant_String_Ptr_C m_refuel_required_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("refuel-required"));
    //const Rete::Symbol_Constant_String_Ptr_C m_snake1_length_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("snake1-length"));
    //const Rete::Symbol_Constant_String_Ptr_C m_snake2_length_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("snake2-length"));
    //const Rete::Symbol_Constant_String_Ptr_C m_snake1_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("snake1-dist"));
    //const Rete::Symbol_Constant_String_Ptr_C m_snake2_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("snake2-dist"));
    //const Rete::Symbol_Constant_String_Ptr_C m_snake1_blank_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("snake1-blank"));
    //const Rete::Symbol_Constant_String_Ptr_C m_snake2_blank_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("snake2-blank"));
    //const Rete::Symbol_Constant_String_Ptr_C m_top_snake_manhattan_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-snake-manhattan"));
    //const Rete::Symbol_Constant_String_Ptr_C m_left_snake_manhattan_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-snake-manhattan"));
    //const Rete::Symbol_Constant_String_Ptr_C m_top_snake_dist_to_blank_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-snake-dist-to-blank"));
    //const Rete::Symbol_Constant_String_Ptr_C m_left_snake_dist_to_blank_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-snake-dist-to-blank"));
    //const Rete::Symbol_Constant_String_Ptr_C m_top_left_ccw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-left-ccw-dist"));
    //const Rete::Symbol_Constant_String_Ptr_C m_top_right_cw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("top-right-cw-dist"));
    //const Rete::Symbol_Constant_String_Ptr_C m_left_top_cw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-top-cw-dist"));
    //const Rete::Symbol_Constant_String_Ptr_C m_left_bottom_ccw_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left-bottom-ccw-dist"));
    //const Rete::Symbol_Constant_String_Ptr_C m_moves_to_blank_tl_ccw_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("moves-to-blank-tl-ccw"));
    const Rete::Symbol_Identifier_Ptr_C m_move_north_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-north"));
    const Rete::Symbol_Identifier_Ptr_C m_move_south_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-south"));
    const Rete::Symbol_Identifier_Ptr_C m_move_east_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-east"));
    const Rete::Symbol_Identifier_Ptr_C m_move_west_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("move-west"));
    const Rete::Symbol_Identifier_Ptr_C m_refuel_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("refuel"));
    const Rete::Symbol_Identifier_Ptr_C m_pickup_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("pickup"));
    const Rete::Symbol_Identifier_Ptr_C m_dropoff_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("dropoff"));
    const Rete::Symbol_Constant_Int_Ptr_C m_type_move = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_type_refuel = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_type_pickup = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_Int_Ptr_C m_type_dropoff = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(4));
    const Rete::Symbol_Constant_Int_Ptr_C m_direction_none = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(0));
    const Rete::Symbol_Constant_Int_Ptr_C m_direction_north = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_direction_south = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_direction_east = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_Int_Ptr_C m_direction_west = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(4));
    const Rete::Symbol_Constant_Int_Ptr_C m_passenger_at_source = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_passenger_onboard = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_passenger_at_destination = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("true"));
    const Rete::Symbol_Constant_String_Ptr_C m_false_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("false"));

    std::map<int64_t, Rete::Symbol_Identifier_Ptr_C> m_move_ids;
    std::map<int64_t, Rete::Symbol_Constant_Int_Ptr_C> m_tile_names;

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
