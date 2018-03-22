#include "taxicab.h"

#include "carli/parser/rete_parser.h"
#include "carli/utility/shared_suffix_list.h"

#include <algorithm>
#include <queue>

namespace Taxicab {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  Environment::Environment() {
    init_impl();
  }

  bool Environment::success() const {
    return m_passenger == AT_DESTINATION;
  }

  bool Environment::failure() const {
#ifndef NDEBUG
    if(m_evaluate_optimality &&
       m_fuel == 0 && std::find(m_filling_stations.begin(), m_filling_stations.end(), m_taxi_position) == m_filling_stations.end() &&
       (m_passenger == AT_SOURCE || m_taxi_position != m_destinations[m_passenger_destination]))
    {
      m_optimal_solution->print_solution(std::cerr);
      m_solution->print_solution(std::cerr);
      assert(false);
    }
#endif

    return m_fuel == 0 && std::find(m_filling_stations.begin(), m_filling_stations.end(), m_taxi_position) == m_filling_stations.end() &&
      (m_passenger == AT_SOURCE || m_taxi_position != m_destinations[m_passenger_destination]);
  }

  std::pair<std::shared_ptr<const Environment::State>, int64_t> Environment::optimal_from(const std::pair<int64_t, int64_t> &taxi_position, const int64_t &fuel, const Passenger &passenger, const int64_t &source, const int64_t &destination) const {
    if(m_optimals.find(std::make_tuple(taxi_position, fuel, passenger, source, destination)) != m_optimals.end())
      return m_optimals[std::make_tuple(taxi_position, fuel, passenger, source, destination)];

    std::set<std::shared_ptr<State>, Rete::compare_deref_lt> astar;

    const std::function<int64_t (const std::pair<int64_t, int64_t> &taxi_position, const int64_t &fuel, const Passenger &passenger)> heuristic =
    std::function<int64_t (const std::pair<int64_t, int64_t> &taxi_position, const int64_t &fuel, const Passenger &passenger)>
    ([this,&source,&destination](const std::pair<int64_t, int64_t> &taxi_position, const int64_t &, const Passenger &passenger)->int64_t {
      //return 0;

      if(passenger == AT_SOURCE)
        return distance(taxi_position, m_destinations[source]) + distance(m_destinations[source], m_destinations[destination]) + 2;
      else if(passenger == ONBOARD)
        return distance(taxi_position, m_destinations[destination]) + 1;
      else
        return 0;
    });

    std::set<std::tuple<std::pair<int64_t, int64_t>, int64_t, Passenger>> states_evaluated;

    std::pair<int64_t, int64_t> rps_min(m_grid_w, m_grid_h);

    {
      states_evaluated.insert(std::make_tuple(taxi_position, fuel, passenger));
      astar.insert(std::make_shared<State>(taxi_position, fuel, passenger, 0, heuristic(taxi_position, fuel, passenger)));
    }

    std::shared_ptr<State> optimal_solution;
    int64_t num_steps = std::numeric_limits<int64_t>::max();
    while(!astar.empty()) {
      const auto state = *astar.begin();
      astar.erase(astar.begin());

      //std::cerr << astar.size() << ' ' << state->num_steps << ' ' << state->heuristic << std::endl;

      if(state->passenger == AT_DESTINATION) {
        //state->print_solution(std::cerr);

        for(std::shared_ptr<const State> ss = state; ss; ss = ss->prev_state) {
          auto summary = std::make_shared<State>(ss->taxi_position, ss->fuel, ss->passenger, state->num_steps - ss->num_steps, 0);
          m_optimals[std::make_tuple(ss->taxi_position, ss->fuel, ss->passenger, source, destination)] = std::make_pair(summary, summary->num_steps);
        }

        optimal_solution = state;
        num_steps = state->num_steps;
        break;
      }

      if(state->passenger == AT_SOURCE && state->taxi_position == m_destinations[source]) {
        if(states_evaluated.find(std::make_tuple(state->taxi_position, state->fuel, ONBOARD)) == states_evaluated.end()) {
          states_evaluated.insert(std::make_tuple(state->taxi_position, state->fuel, ONBOARD));
          astar.insert(std::make_shared<State>(state->taxi_position, state->fuel, ONBOARD, state->num_steps + 1, heuristic(state->taxi_position, state->fuel, ONBOARD), state));
        }
      }
      else if(state->passenger == ONBOARD && state->taxi_position == m_destinations[destination]) {
        if(states_evaluated.find(std::make_tuple(state->taxi_position, state->fuel, AT_DESTINATION)) == states_evaluated.end()) {
          states_evaluated.insert(std::make_tuple(state->taxi_position, state->fuel, AT_DESTINATION));
          astar.insert(std::make_shared<State>(state->taxi_position, state->fuel, AT_DESTINATION, state->num_steps + 1, heuristic(state->taxi_position, state->fuel, AT_DESTINATION), state));
        }
      }
      else {
        for(int64_t i = 0; i != int64_t(m_filling_stations.size()); ++i) {
          const int64_t dist = distance(state->taxi_position, m_filling_stations[i]);
          if(dist > state->fuel)
            continue;

          if(!dist) {
            if(state->fuel < m_fuel_max && states_evaluated.find(std::make_tuple(state->taxi_position, m_fuel_max, state->passenger)) == states_evaluated.end()) {
              states_evaluated.insert(std::make_tuple(state->taxi_position, m_fuel_max, state->passenger));
              astar.insert(std::make_shared<State>(state->taxi_position, m_fuel_max, state->passenger, state->num_steps + 1, heuristic(state->taxi_position, m_fuel_max, state->passenger), state));
            }
            continue;
          }

          if(states_evaluated.find(std::make_tuple(m_filling_stations[i], state->fuel - dist, state->passenger)) == states_evaluated.end()) {
            states_evaluated.insert(std::make_tuple(m_filling_stations[i], state->fuel - dist, state->passenger));
            astar.insert(std::make_shared<State>(m_filling_stations[i], state->fuel - dist, state->passenger, state->num_steps + dist, heuristic(m_filling_stations[i], state->fuel - dist, state->passenger), state));
          }
        }

        for(int64_t i = 0; i != int64_t(m_destinations.size()); ++i) {
          const int64_t dist = distance(state->taxi_position, m_destinations[i]);
          if(!dist || dist > state->fuel)
            continue;

          if(states_evaluated.find(std::make_tuple(m_destinations[i], state->fuel - dist, state->passenger)) == states_evaluated.end()) {
            states_evaluated.insert(std::make_tuple(m_destinations[i], state->fuel - dist, state->passenger));
            astar.insert(std::make_shared<State>(m_destinations[i], state->fuel - dist, state->passenger, state->num_steps + dist, heuristic(m_destinations[i], state->fuel - dist, state->passenger), state));
          }
        }
      }
    }

    return std::make_pair(optimal_solution, num_steps);
  }

  void Environment::init_impl() {
    m_optimals.clear();
    
    /// Begin scenario shenanigans

    switch(get_scenario()) {
      case 35:
        m_grid_w = 20;
        m_grid_h = 20;
        m_num_filling_stations = 4;
        m_num_destinations = 10;
        break;
      
      case 31000:
        if(get_total_step_count() < 0) {
          m_grid_w = 20;
          m_grid_h = 20;
          m_num_filling_stations = 2;
          m_num_destinations = 6;
        }
        else {
          m_grid_w = 20;
          m_grid_h = 20;
          m_num_filling_stations = 4;
          m_num_destinations = 10;
        }
        break;
        
      case 343000:
        if(get_total_step_count() < -10000) {
          m_grid_w = 20;
          m_grid_h = 20;
          m_num_filling_stations = 2;
          m_num_destinations = 6;
        }
        else if(get_total_step_count() < 0) {
          m_grid_w = 20;
          m_grid_h = 20;
          m_num_filling_stations = 3;
          m_num_destinations = 8;
        }
        else {
          m_grid_w = 20;
          m_grid_h = 20;
          m_num_filling_stations = 4;
          m_num_destinations = 10;
        }
        break;

      default:
        break;
    }
    
    /// Begin normal init

    std::set<int64_t> indices;
    do {
      indices.clear();
      for(int64_t i = 0; i != m_grid_w * m_grid_h; ++i)
        indices.insert(i);

      m_filling_stations.clear();
      for(int64_t i = 0; i != m_num_filling_stations; ++i) {
        const int64_t index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
        indices.erase(index);
        m_filling_stations.push_back(std::make_pair(index % m_grid_w, index / m_grid_w));
      }
    } while(!solveable_fuel());

    m_distance_from_fuel.clear();
    for(int64_t i = 0; i != m_num_filling_stations; ++i) {
      Grid grid(m_grid_w * m_grid_h, std::numeric_limits<int64_t>::max());
      grid[m_filling_stations[i].second * m_grid_w + m_filling_stations[i].first] = 0;
      transitive_closure_distances(grid);
      m_distance_from_fuel.push_back(grid);
    }

    m_destinations.clear();
    m_distance_from_dest.clear();
    m_fuel2dest_hops.clear();
    m_fuel2dest_hops.resize(m_num_filling_stations * m_num_destinations, std::numeric_limits<int64_t>::max());
    for(int j = 0; j != m_num_destinations; ++j) {
      int64_t index;
      do {
        index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
        indices.erase(index);
      } while(m_fuel_distances[index] > m_fuel_max / 2);
      const auto dest_pos = std::make_pair(index % m_grid_w, index / m_grid_w);
      m_destinations.push_back(dest_pos);

      Grid grid(m_grid_w * m_grid_h, std::numeric_limits<int64_t>::max());
      grid[index] = 0;
      transitive_closure_distances(grid);
      m_distance_from_dest.push_back(grid);

      for(int64_t i = 0; i != m_num_filling_stations; ++i) {
        if(distance(m_filling_stations[i], dest_pos) <= m_fuel_max / 2)
          set_fuel2dest_hops(i, j, 0);
      }

      for(int64_t k = 0; k != m_num_filling_stations; ++k) {
        for(int64_t i = 0; i != m_num_filling_stations; ++i) {
          const int64_t ik = get_fuel2fuel_hops(i, k);
          const int64_t kj = get_fuel2dest_hops(k, j);
          if(ik != std::numeric_limits<int64_t>::max() && kj != std::numeric_limits<int64_t>::max())
            set_fuel2dest_hops(i, j, std::min(get_fuel2dest_hops(i, j), ik + kj));
        }
      }
    }

    int64_t index;
    do {
      index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
      indices.erase(index);
    } while(m_fuel_distances[index] > m_fuel_max / 2); //m_fuel_max);
    m_taxi_position = std::make_pair(index % m_grid_w, index / m_grid_w);
    m_fuel = m_fuel_distances[index] + m_random.rand_lte(int32_t(m_fuel_max - m_fuel_distances[index]));

    m_passenger = AT_SOURCE;
    m_passenger_source = m_random.rand_lt(int32_t(m_num_destinations));
    m_passenger_destination = m_random.rand_lt(int32_t(m_num_destinations) - 1);
    if(m_passenger_destination >= m_passenger_source)
      ++m_passenger_destination;

    if(m_evaluate_optimality) {
      const auto optimal = optimal_from(m_taxi_position, m_fuel, m_passenger, m_passenger_source, m_passenger_destination);
      m_optimal_solution = optimal.first;
      m_num_steps_to_goal = optimal.second;
#ifndef NDEBUG
      m_solution = std::make_shared<State>(m_taxi_position, m_fuel, m_passenger, 0, 0);
#endif
    }
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action_) {
    const Action &action = debuggable_cast<const Action &>(action_);

    switch(action.type) {
    case Action::MOVE:
      switch(action.direction) {
      case Action::NORTH:
        m_taxi_position.second -= 1;
        --m_fuel;
        break;

      case Action::SOUTH:
        m_taxi_position.second += 1;
        --m_fuel;
        break;

      case Action::EAST:
        m_taxi_position.first += 1;
        --m_fuel;
        break;

      case Action::WEST:
        m_taxi_position.first -= 1;
        --m_fuel;
        break;

      default:
        abort();
      }
      break;

    case Action::REFUEL:
      m_fuel = m_fuel_max;
      break;

    case Action::PICKUP:
      m_passenger = ONBOARD;
      break;

    case Action::PUTDOWN:
      m_passenger = AT_DESTINATION;
      break;

    default:
      abort();
    }

#ifndef NDEBUG
    m_solution = std::make_shared<State>(m_taxi_position, m_fuel, m_passenger, 0, 0, m_solution);
#endif

    if(failure())
      return std::make_pair(-100.0, -100.0);
    else
      return std::make_pair(-1.0, -1.0);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Taxicab(" << m_passenger << ") " << m_fuel << '/' << m_fuel_max << ' ' << char('a' + (m_passenger_source % 25) + ('a' + (m_passenger_source % 25) >= 'f' ? 1 : 0)) << "->" << char('a' + (m_passenger_destination % 25) + ('a' + (m_passenger_destination % 25) >= 'f' ? 1 : 0)) << ':' << endl;
    for(int64_t j = 0; j != m_grid_h; ++j) {
      for(int64_t i = 0; i != m_grid_w; ++i) {
        char c = ' ';
        if(std::find(m_filling_stations.begin(), m_filling_stations.end(), std::make_pair(i, j)) != m_filling_stations.end())
          c = 'f';
        else if(std::find(m_destinations.begin(), m_destinations.end(), std::make_pair(i, j)) != m_destinations.end()) {
          size_t which = std::distance(m_destinations.begin(), std::find(m_destinations.begin(), m_destinations.end(), std::make_pair(i, j)));
          which %= 25;
          if('a' + which >= 'f')
            ++which;
          c = 'a' + char(which);
        }

        if(i == m_taxi_position.first && j == m_taxi_position.second)
          c = c == ' ' ? '@' : char(toupper(c));

        os << c;
      }
      os << endl;
    }
  }

  bool Environment::solveable_fuel() {
    m_fuel_distances.clear();
    m_fuel_distances.resize(m_grid_w * m_grid_h, std::numeric_limits<int64_t>::max());

    for(auto filling_station : m_filling_stations)
      m_fuel_distances[filling_station.second * m_grid_w + filling_station.first] = 0;
    transitive_closure_distances(m_fuel_distances);

    int64_t spots_available = 0;
    for(auto spot : m_fuel_distances) {
      if(spot >= 0 && spot != std::numeric_limits<int64_t>::max())
        ++spots_available;
    }

    if(spots_available < m_num_filling_stations + m_num_destinations)
      return false; /// Insufficient space for all filling stations and destinations

    m_fuel2fuel_hops.clear();
    m_fuel2fuel_hops.resize(m_num_filling_stations * m_num_filling_stations, std::numeric_limits<int64_t>::max());
    for(int64_t i = 0; i != m_num_filling_stations; ++i) {
      set_fuel2fuel_hops(i, i, 0);
      for(int64_t j = 0; j != m_num_filling_stations; ++j) {
        if(i != j && distance(m_filling_stations[i], m_filling_stations[j]) <= m_fuel_max)
          set_fuel2fuel_hops(i, j, 1);
      }
    }

    for(int64_t k = 0; k != m_num_filling_stations; ++k) {
      for(int64_t i = 0; i != m_num_filling_stations; ++i) {
        for(int64_t j = 0; j != m_num_filling_stations; ++j) {
          if(i == j)
            continue;
          const int64_t ik = get_fuel2fuel_hops(i, k);
          const int64_t kj = get_fuel2fuel_hops(k, j);
          if(ik != std::numeric_limits<int64_t>::max() && kj != std::numeric_limits<int64_t>::max())
            set_fuel2fuel_hops(i, j, std::min(get_fuel2fuel_hops(i, j), ik + kj));
        }
      }
    }

    for(int64_t j = 1; j < m_num_filling_stations; ++j) {
      if(get_fuel2fuel_hops(0, j) == std::numeric_limits<int64_t>::max())
        return false; /// Filling station 0 is infinite hops away from another fuel source
    }

    return true;
  }

  void Environment::transitive_closure_distances(Grid &distances) const {
    std::multimap<int64_t, int64_t> dist_to_indices;
    for(int64_t i = 0; i != int64_t(m_grid_w * m_grid_h); ++i) {
      if(distances[i] >= 0 && distances[i] != std::numeric_limits<int64_t>::max())
        dist_to_indices.insert(std::make_pair(distances[i], i));
    }

    while(!dist_to_indices.empty()) {
      const std::pair<int64_t, int64_t> dist_and_index = *dist_to_indices.begin();
      dist_to_indices.erase(dist_to_indices.begin());

      const int64_t x = dist_and_index.second % m_grid_w;
      const int64_t y = dist_and_index.second / m_grid_w;

      for(int j = 0; j != m_grid_h; ++j) {
        for(int i = 0; i != m_grid_w; ++i) {
          if(distance(std::make_pair(i, j), std::make_pair(x, y)) != 1)
            continue;
          if(distances[j * m_grid_w + i] == std::numeric_limits<int64_t>::max()) {
            distances[j * m_grid_w + i] = dist_and_index.first + 1;
            dist_to_indices.insert(std::make_pair(dist_and_index.first + 1, j * m_grid_w + i));
          }
        }
      }
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env_)
    : Carli::Agent(env_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Action>(variables, token); })
  {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    //for(int64_t tile = 0; tile != int64_t(env->get_grid().size()); ++tile) {
    //  std::ostringstream oss;
    //  oss << "move-" << tile;
    //  m_move_ids[tile] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
    //  m_tile_names[tile] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(tile));
    //}

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
      rules_in = "rules/taxicab.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
    //    using dseconds = std::chrono::duration<double, std::ratio<1,1>>;
    //    const auto start_time = std::chrono::high_resolution_clock::now();

    auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      Rete::Agenda::Locker locker(agenda);
      clear_old_wmes();
    }

    Rete::Agenda::Locker locker(agenda);

    std::map<Rete::Symbol_Identifier_Ptr_C, std::pair<int64_t, int64_t>> next_positions;
    if(env->get_fuel() && env->get_taxi_position().second) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_north_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_north_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_north_id, m_direction_attr, m_direction_north));
      next_positions[m_move_north_id] = std::make_pair(env->get_taxi_position().first, env->get_taxi_position().second - 1);
    }
    if(env->get_fuel() && env->get_taxi_position().second + 1 != env->get_grid_h()) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_south_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_south_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_south_id, m_direction_attr, m_direction_south));
      next_positions[m_move_south_id] = std::make_pair(env->get_taxi_position().first, env->get_taxi_position().second + 1);
    }
    if(env->get_fuel() && env->get_taxi_position().first + 1 != env->get_grid_w()) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_east_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_east_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_east_id, m_direction_attr, m_direction_east));
      next_positions[m_move_east_id] = std::make_pair(env->get_taxi_position().first + 1, env->get_taxi_position().second);
    }
    if(env->get_fuel() && env->get_taxi_position().first) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_west_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_west_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_west_id, m_direction_attr, m_direction_west));
      next_positions[m_move_west_id] = std::make_pair(env->get_taxi_position().first - 1, env->get_taxi_position().second);
    }
    if(env->get_fuel() != env->get_fuel_max() && std::find(env->get_filling_stations().begin(), env->get_filling_stations().end(), env->get_taxi_position()) != env->get_filling_stations().end()) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_refuel_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_refuel_id, m_type_attr, m_type_refuel));
      insert_new_wme(std::make_shared<Rete::WME>(m_refuel_id, m_direction_attr, m_direction_none));
    }
    if(env->get_passenger() == Environment::AT_SOURCE && env->get_taxi_position() == env->get_destinations()[env->get_passenger_source()]) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_pickup_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_pickup_id, m_type_attr, m_type_pickup));
      insert_new_wme(std::make_shared<Rete::WME>(m_pickup_id, m_direction_attr, m_direction_none));
    }
    if(env->get_passenger() == Environment::ONBOARD && env->get_taxi_position() == env->get_destinations()[env->get_passenger_destination()]) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_dropoff_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_dropoff_id, m_type_attr, m_type_dropoff));
      insert_new_wme(std::make_shared<Rete::WME>(m_dropoff_id, m_direction_attr, m_direction_none));
    }

    std::vector<int64_t> reachable_filling_stations;
    for(int64_t filling_station = 0; filling_station != int64_t(env->get_filling_stations().size()); ++filling_station) {
      const auto filling_station_id = get_filling_station_id(filling_station);
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_filling_station_attr, filling_station_id));

      int64_t dist_min = std::numeric_limits<int64_t>::max();
      for(auto next_position : next_positions) {
        const int64_t dist = env->get_distance_from_fuel(filling_station)[next_position.second.second * env->get_grid_w() + next_position.second.first];
        dist_min = std::min(dist_min, dist);
      }
      if(dist_min != std::numeric_limits<int64_t>::max()) {
        for(auto next_position : next_positions) {
          if(env->get_distance_from_fuel(filling_station)[next_position.second.second * env->get_grid_w() + next_position.second.first] == dist_min)
            insert_new_wme(std::make_shared<Rete::WME>(next_position.first, m_toward_attr, filling_station_id));
        }
      }

      const int64_t dist = env->get_distance_from_fuel(filling_station)[env->get_taxi_position().second * env->get_grid_w() + env->get_taxi_position().first];
      if(dist <= env->get_fuel())
        reachable_filling_stations.push_back(filling_station);
      //insert_new_wme(std::make_shared<Rete::WME>(filling_station_id, m_fuel_attr, dist <= env->get_fuel() ? m_fuel_roundtrip : m_fuel_insufficient));
    }

    for(int64_t destination = 0; destination != int64_t(env->get_destinations().size()); ++destination) {
      const auto destination_id = get_destination_id(destination);
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_destination_attr, destination_id));

      int64_t hops_min = std::numeric_limits<int64_t>::max();
      int64_t dist_min = std::numeric_limits<int64_t>::max();
      for(const int64_t filling_station : reachable_filling_stations) {
        const int64_t hops = env->get_fuel2dest_hops(filling_station, destination);
        const int64_t dist = env->get_distance_from_dest(destination)[env->get_filling_stations()[filling_station].second * env->get_grid_w() + env->get_filling_stations()[filling_station].first];
        if(hops < hops_min || (hops == hops_min && dist < dist_min)) {
          hops_min = hops;
          dist_min = dist;
        }
      }
      if(hops_min != std::numeric_limits<int64_t>::max() && dist_min != std::numeric_limits<int64_t>::max()) {
        for(const int64_t filling_station : reachable_filling_stations) {
          const int64_t hops = env->get_fuel2dest_hops(filling_station, destination);
          const int64_t dist = env->get_distance_from_dest(destination)[env->get_filling_stations()[filling_station].second * env->get_grid_w() + env->get_filling_stations()[filling_station].first];
          if(hops == hops_min && dist == dist_min)
            insert_new_wme(std::make_shared<Rete::WME>(get_filling_station_id(filling_station), m_toward_attr, destination_id));
        }
      }

      const int64_t fuel = env->get_fuel();
      const int64_t taxi_to_dest = env->get_distance_from_dest(destination)[env->get_taxi_position().second * env->get_grid_w() + env->get_taxi_position().first];
      const int64_t dest_to_fuel = env->get_fuel_distances()[env->get_destinations()[destination].second * env->get_grid_w() + env->get_destinations()[destination].first];

      if(fuel >= taxi_to_dest) {
        dist_min = std::numeric_limits<int64_t>::max();
        for(auto next_position : next_positions) {
          const int64_t dist = env->get_distance_from_dest(destination)[next_position.second.second * env->get_grid_w() + next_position.second.first];
          dist_min = std::min(dist_min, dist);
        }
        if(dist_min != std::numeric_limits<int64_t>::max()) {
          for(auto next_position : next_positions) {
            if(env->get_distance_from_dest(destination)[next_position.second.second * env->get_grid_w() + next_position.second.first] == dist_min)
              insert_new_wme(std::make_shared<Rete::WME>(next_position.first, m_toward_attr, destination_id));
          }
        }
      }

      insert_new_wme(std::make_shared<Rete::WME>(destination_id, m_fuel_attr, fuel >= taxi_to_dest + dest_to_fuel ? m_fuel_roundtrip : fuel >= taxi_to_dest ? m_fuel_oneway : m_fuel_insufficient));
    }

    //bool refuel_required = false;
    //if(env->get_fuel() != env->get_fuel_max() && std::find(env->get_filling_stations().begin(), env->get_filling_stations().end(), env->get_taxi_position()) != env->get_filling_stations().end()) {
    //  const auto moves_to_goal = env->optimal_from(env->get_taxi_position(), env->get_fuel(), env->get_passenger(), env->get_passenger_source(), env->get_passenger_destination());
    //  auto state = moves_to_goal.first;
    //  if(state->passenger != Environment::AT_DESTINATION)
    //    refuel_required = true;
    //  else {
    //    auto state_next = state;
    //    while(state->prev_state) {
    //      state_next = state;
    //      state = state->prev_state;
    //    }
    //    if(state_next->fuel == env->get_fuel_max())
    //      refuel_required = true;
    //  }
    //}
    //insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_refuel_required_attr, refuel_required ? m_true_value : m_false_value));

    //std::pair<std::set<Rete::Symbol_Identifier_Ptr_C>, int64_t> min_through_source;
    //std::pair<std::set<Rete::Symbol_Identifier_Ptr_C>, int64_t> min_to_dest;
    //for(const auto next_position : next_positions) {
    //  const auto through_source = env->optimal_from(next_position.second, refuel_required ? env->get_fuel_max() : env->get_fuel() - 1, Environment::AT_SOURCE, env->get_passenger_source(), env->get_passenger_destination());
    //  if(min_through_source.first.empty() || through_source.second < min_through_source.second) {
    //    min_through_source.first.clear();
    //    min_through_source.second = through_source.second;
    //  }
    //  if(through_source.second <= min_through_source.second)
    //    min_through_source.first.insert(next_position.first);

    //  const auto to_dest = env->optimal_from(next_position.second, refuel_required ? env->get_fuel_max() : env->get_fuel() - 1, Environment::ONBOARD, env->get_passenger_source(), env->get_passenger_destination());
    //  if(min_to_dest.first.empty() || to_dest.second < min_to_dest.second) {
    //    min_to_dest.first.clear();
    //    min_to_dest.second = to_dest.second;
    //  }
    //  if(to_dest.second <= min_to_dest.second)
    //    min_to_dest.first.insert(next_position.first);
    //}

    //for(auto id : {m_move_north_id, m_move_south_id, m_move_east_id, m_move_west_id}) {
    //  const bool is_min = min_through_source.second != std::numeric_limits<int64_t>::max() && min_through_source.first.find(id) != min_through_source.first.end();
    //  insert_new_wme(std::make_shared<Rete::WME>(id, m_toward_pickup_attr, is_min ? m_true_value : m_false_value));
    //}

    //for(auto id : {m_move_north_id, m_move_south_id, m_move_east_id, m_move_west_id}) {
    //  const bool is_min = min_to_dest.second != std::numeric_limits<int64_t>::max() && min_to_dest.first.find(id) != min_to_dest.first.end();
    //  insert_new_wme(std::make_shared<Rete::WME>(id, m_toward_dropoff_attr, is_min ? m_true_value : m_false_value));
    //}

    if(env->get_passenger() == Environment::AT_SOURCE)
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_attr, m_passenger_at_source));
    else if(env->get_passenger() == Environment::ONBOARD)
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_attr, m_passenger_onboard));
    else
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_attr, m_passenger_at_destination));

    //insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_source_attr, get_destination_id(env->get_passenger_source())));
    //insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_destination_attr, get_destination_id(env->get_passenger_destination())));

    if(env->get_passenger() == Environment::AT_SOURCE)
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_next_stop_attr, get_destination_id(env->get_passenger_source())));
    else
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_next_stop_attr, get_destination_id(env->get_passenger_destination())));

    clear_old_wmes();
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
    else if(env->failure())
      m_metastate = Metastate::FAILURE;
  }

  Rete::Symbol_Identifier_Ptr_C Agent::get_filling_station_id(const int64_t &filling_station) {
    if(!m_filling_station_ids[filling_station]) {
      std::ostringstream oss;
      oss << "filling-station-" << filling_station;
      m_filling_station_ids[filling_station] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
    }
    return m_filling_station_ids[filling_station];
  }

  Rete::Symbol_Identifier_Ptr_C Agent::get_destination_id(const int64_t &destination) {
    if(!m_destination_ids[destination]) {
      std::ostringstream oss;
      oss << "destination-" << destination;
      m_destination_ids[destination] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
    }
    return m_destination_ids[destination];
  }

}
