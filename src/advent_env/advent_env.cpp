#include "advent.h"

#include "carli/parser/rete_parser.h"

namespace Advent {

  Environment::Environment() {
    init_impl();
  }

  bool Environment::success() const {
    ///< Player not dead ... and killed the dragon
//     return !failure() && m_player_pos.first == 4 && m_player_pos.second == 3;
    return !failure() && m_water_dragon->health <= 0;
  }

  bool Environment::failure() const {
    /// Player dead or (the bridge has been burned and the player moved to the wrong side of it)
    return m_player.is_dead || (!m_rooms[3][3] && m_player_pos.second < 3);
  }

  void Environment::init_impl() {
    m_player = Character();
    m_player_pos = std::make_pair(2, 0);

    for(int x = 0; x != 5; ++x)
      for(int y = 0; y != 5; ++y)
        m_rooms[x][y] = y == 2 && x != 2 ? nullptr : std::make_shared<Room>();
      
    m_player.items_max = 3;
    m_player.print_char = '@';
    
    m_water_dragon = m_rooms[4][3]->enemy = std::make_shared<Character>();
    m_water_dragon->creature = Creature::CREATURE_WATER;
    m_water_dragon->print_char = 'd';
    
    m_troll = m_rooms[2][2]->enemy = std::make_shared<Character>();
    m_troll->creature = Creature::CREATURE_TROLL;
    m_troll->print_char = 't';
    m_troll->items.give(Item::ITEM_MAGIC_SWORD);
//     m_rooms[2][2]->items.give(Item::ITEM_MAGIC_SWORD);
  }

  std::pair<Agent::reward_type, Agent::reward_type> Environment::transition_impl(const Carli::Action &action) {
    auto room = m_rooms[m_player_pos.first][m_player_pos.second];
    assert(room);
    
    bool killed_troll = m_troll->is_dead;
    
    if(const Move * const move = dynamic_cast<const Move *>(&action)) {
      switch(move->direction) {
        case Direction::DIR_NONE:
          break;
          
        case Direction::DIR_NORTH:
          m_player_pos.second += 1;
          break;
          
        case Direction::DIR_SOUTH:
          m_player_pos.second -= 1;
          break;
          
        case Direction::DIR_EAST:
          m_player_pos.first += 1;
          break;
          
        case Direction::DIR_WEST:
          m_player_pos.first -= 1;
          break;
          
        default:
          abort();
      }
      
//       if(move->direction != Direction::DIR_NONE) {
//         room = m_rooms[m_player_pos.first][m_player_pos.second];
//         assert(room);
//         if(room->enemy && !room->enemy->is_dead)
//           room->enemy->health = room->enemy->health_max;
//       }
      if(m_player_pos.first == 0 && m_player_pos.second == 0) {
        if(!m_player.items.has(Item::ITEM_FIREBOLT) && !room->items.has(Item::ITEM_FIREBOLT))
          room->items.give(Item::ITEM_FIREBOLT);
      }
      else if(m_player_pos.first == 0 && m_player_pos.second == 1) {
        if(!m_player.items.has(Item::ITEM_ICEBOLT) && !room->items.has(Item::ITEM_ICEBOLT))
          room->items.give(Item::ITEM_ICEBOLT);
      }
    }
    else if(const Attack * const attack = dynamic_cast<const Attack *>(&action)) {
      assert(room->enemy);
      room->enemy->receive_attack(m_player.weapon);
    }
    else if(const Take * const take = dynamic_cast<const Take *>(&action)) {
      room->items.take(take->item);
      m_player.items.give(take->item);
    }
    else if(const Drop * const drop = dynamic_cast<const Drop *>(&action)) {
      m_player.items.take(drop->item);
      room->items.give(drop->item);
      if(m_player.weapon != WEAPON_FISTS && m_player.items.has(Item(m_player.weapon)) == 0)
        m_player.weapon = WEAPON_FISTS;
    }
    else if(const Equip * const equip = dynamic_cast<const Equip *>(&action)) {
      assert(equip->weapon == WEAPON_FISTS || m_player.items.has(Item(equip->weapon)));
      m_player.weapon = equip->weapon;
    }
    else if(const Cast * const cast = dynamic_cast<const Cast *>(&action)) {
      assert(cast->spell == SPELL_HEAL || (m_player.items.has(Item(cast->spell)) && room->enemy));
      if(cast->spell == SPELL_HEAL) {
        m_player.receive_cast(cast->spell);
      }
      else {
        room->enemy->receive_cast(cast->spell);
        m_player.items.take(Item(cast->spell));
      }
    }
    else
      abort();
    
    if(room->enemy) {
      if(room->enemy->is_dead) {
        room->enemy->health = 0;
        room->items += room->enemy->items;
        room->enemy->items.clear();
      }
      else if(room->enemy->creature == CREATURE_TROLL) {
        room->enemy->health = std::min(std::max(room->enemy->health, int64_t()) + 1, room->enemy->health_max);
      }
      else if(room->enemy->health <= 0) {
        room->enemy->health = 0;
        room->enemy->is_dead = true;
        room->items += room->enemy->items;
        room->enemy->items.clear();
      }
      else
        m_player.health -= 2;
    }
    
    if(m_player.health <= 0) {
      m_player.health = 0;
      m_player.is_dead = true;
    }

    killed_troll ^= m_troll->is_dead;
    
    return std::make_pair(success() ? 100.0 : failure() ? -100.0 : killed_troll ? 50.0 : -1.0, -1.0);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Advent:" << endl;
    for(int y = int(m_rooms.size()) - 1; y != -1; --y) {
      os << "  ";
      for(int x = 0; x != int(m_rooms.size()); ++x) {
        const auto room = m_rooms[x][y];
        if(room && room->enemy && !room->enemy->is_dead)
          os << (m_player_pos.first == x && m_player_pos.second == y ? char(toupper(room->enemy->print_char)) : room->enemy->print_char);
        else if(room)
          os << (m_player_pos.first == x && m_player_pos.second == y ? m_player.print_char : '.');
        else
          os << ' ';
      }
      os << endl;
    }
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env_)
   : Carli::Agent(env_, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return make_Action(variables, token);})
  {
    auto env = dynamic_pointer_cast<const Environment>(get_env());
    std::ostringstream oss;

    for(int64_t i : std::array<int64_t, 5>({{1, 2, 3, 5, 6}})) {
      oss << "TAKE-" << i;
      m_take_ids[Item(i)] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
      oss.str("");
      oss << "DROP-" << i;
      m_drop_ids[Item(i)] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
      oss.str("");
    }

    for(int64_t i = 0; i != 4; ++i) {
      oss << "EQUIP-" << i;
      m_equip_ids[Weapon(i)] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
      oss.str("");\
    }

    for(int64_t i = 4; i != 7; ++i) {
      oss << "CAST-" << i;
      m_cast_ids[Spell(i)] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier(oss.str()));
      oss.str("");\
    }

    m_move_ids[DIR_NONE] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("WAIT"));
    m_move_ids[DIR_NORTH] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("MOVE-NORTH"));
    m_move_ids[DIR_SOUTH] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("MOVE-SOUTH"));
    m_move_ids[DIR_EAST] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("MOVE-EAST"));
    m_move_ids[DIR_WEST] = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("MOVE-WEST"));
    for(int64_t i = 0; i != 5; ++i)
      m_direction_values[Direction(i)] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(i));

    for(int64_t i = 0; i != 7; ++i)
      m_item_values[Item(i)] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(i));

    for(int64_t i = 0; i != env->get_Rooms_size(); ++i)
      m_position_values[i] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(i));

    for(int64_t i = 0; i != 11; ++i)
      m_health_values[i] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(i));

    for(int64_t i = 0; i != 4; ++i)
      m_creature_values[Creature(i)] = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(i));

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
      rules_in = "rules/advent.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
//    using dseconds = std::chrono::duration<double, std::ratio<1,1>>;
//    const auto start_time = std::chrono::high_resolution_clock::now();

    auto env = dynamic_pointer_cast<const Environment>(get_env());

    const auto &player = env->get_Player();
    const auto &player_pos = env->get_Player_pos();
    const auto &room = env->get_Room(player_pos.first, player_pos.second);
    const auto &enemy = room->enemy;

    std::list<Rete::WME_Ptr_C> wmes_current;
    
    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_player_attr, m_player_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_x_attr, m_position_values[player_pos.first]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_y_attr, m_position_values[player_pos.second]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_dead_attr, player.is_dead ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_health_attr, m_health_values[player.health]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_equipped_attr, m_item_values[Item(player.weapon)]));

    wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_in_attr, m_room_id));

    wmes_current.push_back(std::make_shared<Rete::WME>(m_room_id, m_enemy_attr, m_enemy_id));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_enemy_id, m_type_attr, m_creature_values[enemy ? Creature(enemy->creature) : Creature::CREATURE_SOLID]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_enemy_id, m_dead_attr, (!enemy || enemy->is_dead) ? m_true_value : m_false_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_enemy_id, m_health_attr, m_health_values[enemy ? enemy->health : 0]));
    
    for(int i = 1; i != 7; ++i) {
      if(room->items.has(Item(i)))
        wmes_current.push_back(std::make_shared<Rete::WME>(m_room_id, m_has_attr, m_item_values[Item(i)]));
      if(player.items.has(Item(i)))
        wmes_current.push_back(std::make_shared<Rete::WME>(m_player_id, m_has_attr, m_item_values[Item(i)]));
    }
    
    /// 1. Move

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_ids[Direction::DIR_NONE]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_NONE], m_name_attr, m_move_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_NONE], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_NONE], m_item_attr, m_item_values[Item::ITEM_NONE]));
    if(!room->enemy || room->enemy->is_dead || (room->enemy->health == 1 && room->enemy->creature == CREATURE_TROLL)) {
      if(env->get_Room(player_pos.first, player_pos.second + 1)) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_ids[Direction::DIR_NORTH]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_NORTH], m_name_attr, m_move_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_NORTH], m_direction_attr, m_direction_values[Direction::DIR_NORTH]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_NORTH], m_item_attr, m_item_values[Item::ITEM_NONE]));
      }
      if(env->get_Room(player_pos.first, player_pos.second - 1)) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_ids[Direction::DIR_SOUTH]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_SOUTH], m_name_attr, m_move_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_SOUTH], m_direction_attr, m_direction_values[Direction::DIR_SOUTH]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_SOUTH], m_item_attr, m_item_values[Item::ITEM_NONE]));
      }
      if(env->get_Room(player_pos.first + 1, player_pos.second)) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_ids[Direction::DIR_EAST]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_EAST], m_name_attr, m_move_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_EAST], m_direction_attr, m_direction_values[Direction::DIR_EAST]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_EAST], m_item_attr, m_item_values[Item::ITEM_NONE]));
      }
      if(env->get_Room(player_pos.first - 1, player_pos.second)) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_move_ids[Direction::DIR_WEST]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_WEST], m_name_attr, m_move_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_WEST], m_direction_attr, m_direction_values[Direction::DIR_WEST]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_move_ids[Direction::DIR_WEST], m_item_attr, m_item_values[Item::ITEM_NONE]));
      }
    }
    
    /// 2. Attack
    
    if(room->enemy) {
      wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_attack_id));
      wmes_current.push_back(std::make_shared<Rete::WME>(m_attack_id, m_name_attr, m_attack_value));
      wmes_current.push_back(std::make_shared<Rete::WME>(m_attack_id, m_direction_attr, m_direction_values[Direction::DIR_NONE]));
      wmes_current.push_back(std::make_shared<Rete::WME>(m_attack_id, m_item_attr, m_item_values[Item::ITEM_NONE]));
    }
    
    /// 3. Take
    
    for(int i = 1; i != 7; ++i) {
      if(room->items.has(Item(i))) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_take_ids[Item(i)]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_take_ids[Item(i)], m_name_attr, m_take_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_take_ids[Item(i)], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_take_ids[Item(i)], m_item_attr, m_item_values[Item(i)]));
      }
    }
    
    /// 4. Drop
    
//     for(int i = 1; i != 7; ++i) {
//       if(player.items.has(Item(i))) {
//         wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_drop_ids[Item(i)]));
//         wmes_current.push_back(std::make_shared<Rete::WME>(m_drop_ids[Item(i)], m_name_attr, m_drop_value));
//         wmes_current.push_back(std::make_shared<Rete::WME>(m_drop_ids[Item(i)], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
//         wmes_current.push_back(std::make_shared<Rete::WME>(m_drop_ids[Item(i)], m_item_attr, m_item_values[Item(i)]));
//       }
//     }
    
    /// 5. Equip

//     if(player.weapon != Weapon::WEAPON_FISTS) {
//       wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_equip_ids[Weapon::WEAPON_FISTS]));
//       wmes_current.push_back(std::make_shared<Rete::WME>(m_equip_ids[Weapon::WEAPON_FISTS], m_name_attr, m_equip_value));
//       wmes_current.push_back(std::make_shared<Rete::WME>(m_equip_ids[Weapon::WEAPON_FISTS], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
//       wmes_current.push_back(std::make_shared<Rete::WME>(m_equip_ids[Weapon::WEAPON_FISTS], m_item_attr, m_item_values[Item(Weapon::WEAPON_FISTS)]));
//     }
    for(int i = 1; i != 4; ++i) {
      if(player.items.has(Item(i)) && player.weapon != Weapon(i)) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_equip_ids[Weapon(i)]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_equip_ids[Weapon(i)], m_name_attr, m_equip_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_equip_ids[Weapon(i)], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_equip_ids[Weapon(i)], m_item_attr, m_item_values[Item(i)]));
      }
    }
    
    /// 6. Cast

    wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_cast_ids[Spell::SPELL_HEAL]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_cast_ids[Spell::SPELL_HEAL], m_name_attr, m_cast_value));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_cast_ids[Spell::SPELL_HEAL], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
    wmes_current.push_back(std::make_shared<Rete::WME>(m_cast_ids[Spell::SPELL_HEAL], m_item_attr, m_item_values[Item(Spell::SPELL_HEAL)]));
    for(int i = 5; i != 7; ++i) {
      if(player.items.has(Item(i)) && room->enemy && !room->enemy->is_dead) {
        wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_action_attr, m_cast_ids[Spell(i)]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_cast_ids[Spell(i)], m_name_attr, m_cast_value));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_cast_ids[Spell(i)], m_direction_attr, m_direction_values[Direction::DIR_NONE]));
        wmes_current.push_back(std::make_shared<Rete::WME>(m_cast_ids[Spell(i)], m_item_attr, m_item_values[Item(i)]));
      }
    }

//     wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_item_attr, m_item_ids[Item::ITEM_NONE]));
//     wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_weapon_attr, m_weapon_ids[m_player.weapon]));
//     wmes_current.push_back(std::make_shared<Rete::WME>(m_s_id, m_spell_attr, m_spell_ids[Spell::SPELL_NONE]));

//    const auto end_time = std::chrono::high_resolution_clock::now();
//    m_feature_generation_time += std::chrono::duration_cast<dseconds>(end_time - start_time).count();

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
    const auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->success())
      m_metastate = Carli::Metastate::SUCCESS;
    else if(env->failure())
      m_metastate = Carli::Metastate::FAILURE;
  }

}
