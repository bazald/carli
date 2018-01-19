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
      } while(distances[index] > m_taxi_fuel_max / 2);
      m_destinations.push_back(std::make_pair(index % m_grid_w, index / m_grid_w));
    }

    int64_t index;
    do {
      index = *std::next(indices.begin(), m_random.rand_lt(int32_t(indices.size())));
      indices.erase(index);
    } while(distances[index] > m_taxi_fuel_max / 2); //m_taxi_fuel_max);
    m_taxi_position = std::make_pair(index % m_grid_w, index / m_grid_w);
    m_taxi_fuel = distances[index] + m_random.rand_lte(int32_t(m_taxi_fuel_max - distances[index]));

    m_passenger = AT_SOURCE;
    m_passenger_source = m_random.rand_lt(int32_t(m_num_destinations));
    m_passenger_destination = m_random.rand_lt(int32_t(m_num_destinations) - 1);
    if(m_passenger_destination >= m_passenger_source)
      ++m_passenger_destination;

    if(m_evaluate_optimality) {
      m_num_steps_to_goal = 0;
    }
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action) {
    if(const Move * const move = dynamic_cast<const Move *>(&action)) {
    }
    else if(const Dropoff * const dropoff = dynamic_cast<const Dropoff *>(&action)) {
    }
    else if(const Pickup * const pickup = dynamic_cast<const Pickup *>(&action)) {
    }
    else if(const Refuel * const refuel = dynamic_cast<const Refuel *>(&action)) {
    }

    return std::make_pair(-1.0, -1.0);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Taxicab " << m_taxi_fuel << '/' << m_taxi_fuel_max << ' ' << char('a' + (m_passenger_source % 25)) << "->" << char('a' + (m_passenger_destination % 25)) << ':' << endl;
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
        if(fs2 != filling_station && reachable.find(fs2) == reachable.end() && distance(filling_station, fs2) <= m_taxi_fuel_max)
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
    : Carli::Agent(env_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token); })
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
      rules_in = "rules/sliding-puzzle.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
    //    using dseconds = std::chrono::duration<double, std::ratio<1,1>>;
    //    const auto start_time = std::chrono::high_resolution_clock::now();

    auto env = dynamic_pointer_cast<const Environment>(get_env());



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

    if(get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes")) {
      Rete::Agenda::Locker locker(agenda);
      clear_old_wmes();
    }

    Rete::Agenda::Locker locker(agenda);

    //const auto generate_action =
    //  [this, &env, &generate_relative_attribute,]
    //  (const std::pair<int64_t,int64_t> &pos, const Rete::Symbol_Identifier_Ptr_C &move_id, const Rete::Symbol_Constant_Int_Ptr_C &tile_name)
    //{
    //  insert_new_wme(std::make_shared<Rete::WME>(m_s_id, m_action_attr, move_id));
    //  insert_new_wme(std::make_shared<Rete::WME>(move_id, m_tile_attr, tile_name));

    //  //generate_relative_attribute(std::get<0>(lwccw_snake1_lendistblank), std::get<0>(lwccw_snake1_lendistblank_next), move_id, m_snake1_length_attr);
    //  //generate_relative_attribute(std::get<0>(lwccw_snake2_lendistblank), std::get<0>(lwccw_snake2_lendistblank_next), move_id, m_snake2_length_attr);
    //  //generate_relative_attribute(std::get<1>(lwccw_snake1_lendistblank), std::get<1>(lwccw_snake1_lendistblank_next), move_id, m_snake1_dist_attr);
    //  //generate_relative_attribute(std::get<1>(lwccw_snake2_lendistblank), std::get<1>(lwccw_snake2_lendistblank_next), move_id, m_snake2_dist_attr);
    //  //generate_relative_attribute(std::get<2>(lwccw_snake1_lendistblank), std::get<2>(lwccw_snake1_lendistblank_next), move_id, m_snake1_blank_attr);
    //  //generate_relative_attribute(std::get<2>(lwccw_snake2_lendistblank), std::get<2>(lwccw_snake2_lendistblank_next), move_id, m_snake2_blank_attr);
    //};

    //for(int j = 0; j != grid_h; ++j) {
    //  for(int i = 0; i != grid_w; ++i) {
    //    if(env->distance(std::make_pair(i, j), blank) != 1)
    //      continue;
    //    generate_action(std::make_pair(i, j), m_move_ids[grid[j * grid_w + i]], m_tile_names[grid[j * grid_w + i]]);
    //  }
    //}

    clear_old_wmes();
  }

  void Agent::update() {
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Metastate::SUCCESS;
  }

}
