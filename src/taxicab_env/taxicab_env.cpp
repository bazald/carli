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
    //if(m_fuel == 0 && std::find(m_filling_stations.begin(), m_filling_stations.end(), m_taxi_position) == m_filling_stations.end() &&
    //   (m_passenger != ONBOARD || m_taxi_position != m_destinations[m_passenger_destination]))
    //{
    //  m_optimal_solution->print_solution(std::cerr);
    //  m_solution->print_solution(std::cerr);
    //  assert(false);
    //}

    return m_fuel == 0 && std::find(m_filling_stations.begin(), m_filling_stations.end(), m_taxi_position) == m_filling_stations.end() &&
      (m_passenger != ONBOARD || m_taxi_position != m_destinations[m_passenger_destination]);
  }

  std::pair<std::shared_ptr<const Environment::State>, int64_t> Environment::optimal_from(const std::pair<int64_t, int64_t> &taxi_position, const int64_t &fuel, const Passenger &passenger, const int64_t &source, const int64_t &destination) const {
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
    std::set<int64_t> indices;
    Grid distances;
    do {
      indices.clear();
      for(int64_t i = 0; i != m_grid_w * m_grid_h; ++i)
        indices.insert(i);

      m_filling_stations.clear();
      for(int i = 0; i != m_num_filling_stations; ++i) {
        const int64_t index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
        indices.erase(index);
        m_filling_stations.push_back(std::make_pair(index % m_grid_w, index / m_grid_w));
      }
    } while(!solveable_fuel(distances));

    m_destinations.clear();
    for(int i = 0; i != m_num_destinations; ++i) {
      int64_t index;
      do {
        index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
        indices.erase(index);
      } while(distances[index] > m_fuel_max / 2);
      m_destinations.push_back(std::make_pair(index % m_grid_w, index / m_grid_w));
    }

    int64_t index;
    do {
      index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
      indices.erase(index);
    } while(distances[index] > m_fuel_max / 2); //m_fuel_max);
    m_taxi_position = std::make_pair(index % m_grid_w, index / m_grid_w);
    m_fuel = distances[index] + m_random.rand_lte(int32_t(m_fuel_max - distances[index]));

    m_passenger = AT_SOURCE;
    m_passenger_source = m_random.rand_lt(int32_t(m_num_destinations));
    m_passenger_destination = m_random.rand_lt(int32_t(m_num_destinations) - 1);
    if(m_passenger_destination >= m_passenger_source)
      ++m_passenger_destination;

    //if(m_evaluate_optimality)
    const auto optimal = optimal_from(m_taxi_position, m_fuel, m_passenger, m_passenger_source, m_passenger_destination);
    m_optimal_solution = optimal.first;
    m_num_steps_to_goal = -optimal.second;
    m_solution = std::make_shared<State>(m_taxi_position, m_fuel, m_passenger, 0, 0);
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

    m_solution = std::make_shared<State>(m_taxi_position, m_fuel, m_passenger, 0, 0, m_solution);

    if(failure())
      return std::make_pair(-50.0, -50.0);
    else
      return std::make_pair(-1.0, -1.0);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Taxicab " << m_fuel << '/' << m_fuel_max << ' ' << char('a' + (m_passenger_source % 25)) << "->" << char('a' + (m_passenger_destination % 25)) << ':' << endl;
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

  bool Environment::solveable_fuel(Grid &distances_out) const {
    distances_out.clear();
    distances_out.resize(m_grid_w * m_grid_h, std::numeric_limits<int64_t>::max());

    for(auto filling_station : m_filling_stations)
      distances_out[filling_station.second * m_grid_w + filling_station.first] = 0;
    transitive_closure_distances(distances_out);

    int64_t spots_available = 0;
    for(auto spot : distances_out) {
      if(spot >= 0 && spot != std::numeric_limits<int64_t>::max())
        ++spots_available;
    }

    if(spots_available < m_num_filling_stations + m_num_destinations)
      return false; /// Insufficient space for all filling stations and destinations

    std::unordered_set<std::pair<int64_t, int64_t>> reachable;
    std::queue<std::pair<int64_t, int64_t>> reachable_queue;
    reachable_queue.push(*m_filling_stations.begin());
    while(!reachable_queue.empty()) {
      const auto filling_station = reachable_queue.front();
      reachable_queue.pop();
      reachable.insert(filling_station);
      for(auto fs2 : m_filling_stations) {
        if(fs2 != filling_station && reachable.find(fs2) == reachable.end() && distance(filling_station, fs2) <= m_fuel_max)
          reachable_queue.push(fs2);
      }
    }
    if(reachable.size() != m_filling_stations.size())
      return false;

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

    if(env->get_fuel() && env->get_taxi_position().second) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_north_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_north_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_north_id, m_direction_attr, m_direction_north));
    }
    if(env->get_fuel() && env->get_taxi_position().second + 1 != env->get_grid_h()) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_south_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_south_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_south_id, m_direction_attr, m_direction_south));
    }
    if(env->get_fuel() && env->get_taxi_position().first + 1 != env->get_grid_w()) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_east_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_east_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_east_id, m_direction_attr, m_direction_east));
    }
    if(env->get_fuel() && env->get_taxi_position().first) {
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_west_id));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_west_id, m_type_attr, m_type_move));
      insert_new_wme(std::make_shared<Rete::WME>(m_move_west_id, m_direction_attr, m_direction_west));
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

    bool refuel_required = false;
    if(env->get_fuel() != env->get_fuel_max() && std::find(env->get_filling_stations().begin(), env->get_filling_stations().end(), env->get_taxi_position()) != env->get_filling_stations().end()) {
      const auto moves_to_goal = env->optimal_from(env->get_taxi_position(), env->get_fuel_max(), env->get_passenger(), env->get_passenger_source(), env->get_passenger_destination());
      auto state = moves_to_goal.first;
      if(state->passenger != Environment::AT_DESTINATION)
        refuel_required = true;
      else {
        auto state_next = state;
        while(state->prev_state) {
          state_next = state;
          state = state->prev_state;
        }
        if(state_next->fuel == env->get_fuel_max())
          refuel_required = true;
      }
    }
    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_refuel_required_attr, refuel_required ? m_true_value : m_false_value));

    std::pair<std::set<Rete::Symbol_Identifier_Ptr_C>, int64_t> min_through_source;
    std::pair<std::set<Rete::Symbol_Identifier_Ptr_C>, int64_t> min_to_dest;
    std::map<Rete::Symbol_Identifier_Ptr_C, std::pair<int64_t, int64_t>> next_positions;
    if(env->get_fuel()) {
      if(env->get_taxi_position().second)
        next_positions[m_move_north_id] = std::make_pair(env->get_taxi_position().first, env->get_taxi_position().second - 1);
      if(env->get_taxi_position().second + 1 != env->get_grid_h())
        next_positions[m_move_south_id] = std::make_pair(env->get_taxi_position().first, env->get_taxi_position().second + 1);
      if(env->get_taxi_position().first + 1 != env->get_grid_w())
        next_positions[m_move_east_id] = std::make_pair(env->get_taxi_position().first + 1, env->get_taxi_position().second);
      if(env->get_taxi_position().first)
        next_positions[m_move_west_id] = std::make_pair(env->get_taxi_position().first - 1, env->get_taxi_position().second);
    }
    for(const auto next_position : next_positions) {
      const auto through_source = env->optimal_from(next_position.second, refuel_required ? env->get_fuel_max() : env->get_fuel() - 1, Environment::AT_SOURCE, env->get_passenger_source(), env->get_passenger_destination());
      if(min_through_source.first.empty() || through_source.second < min_through_source.second) {
        min_through_source.first.clear();
        min_through_source.second = through_source.second;
      }
      if(through_source.second <= min_through_source.second)
        min_through_source.first.insert(next_position.first);

      const auto to_dest = env->optimal_from(next_position.second, refuel_required ? env->get_fuel_max() : env->get_fuel() - 1, Environment::ONBOARD, env->get_passenger_source(), env->get_passenger_destination());
      if(min_to_dest.first.empty() || to_dest.second < min_to_dest.second) {
        min_to_dest.first.clear();
        min_to_dest.second = to_dest.second;
      }
      if(to_dest.second <= min_to_dest.second)
        min_to_dest.first.insert(next_position.first);
    }

    for(auto id : {m_move_north_id, m_move_south_id, m_move_east_id, m_move_west_id}) {
      const bool is_min = min_through_source.second != std::numeric_limits<int64_t>::max() && min_through_source.first.find(id) != min_through_source.first.end();
      insert_new_wme(std::make_shared<Rete::WME>(id, m_toward_pickup_attr, is_min ? m_true_value : m_false_value));
    }

    for(auto id : {m_move_north_id, m_move_south_id, m_move_east_id, m_move_west_id}) {
      const bool is_min = min_to_dest.second != std::numeric_limits<int64_t>::max() && min_to_dest.first.find(id) != min_to_dest.first.end();
      insert_new_wme(std::make_shared<Rete::WME>(id, m_toward_dropoff_attr, is_min ? m_true_value : m_false_value));
    }

    if(env->get_passenger() == Environment::AT_SOURCE)
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_attr, m_passenger_at_source));
    else if(env->get_passenger() == Environment::ONBOARD)
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_attr, m_passenger_onboard));
    else
      insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_attr, m_passenger_at_destination));

    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_source_attr, Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(env->get_passenger_source()))));
    insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_passenger_destination_attr, Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(env->get_passenger_destination()))));

    clear_old_wmes();
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
    else if(env->failure())
      m_metastate = Metastate::FAILURE;
  }

}
