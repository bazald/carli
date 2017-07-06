#ifndef ADVENT_H
#define ADVENT_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>

#if !defined(_WINDOWS)
#define ADVENT_LINKAGE
#elif !defined(ADVENT_INTERNAL)
#define ADVENT_LINKAGE __declspec(dllimport)
#else
#define ADVENT_LINKAGE __declspec(dllexport)
#endif

namespace Advent {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  typedef int64_t block_id;
  
  enum Direction : int64_t {DIR_NONE = 0, DIR_NORTH = 1, DIR_SOUTH = 2, DIR_EAST = 3, DIR_WEST = 4};
  enum Item : int64_t {ITEM_NONE = 0, ITEM_SWORD = 1, ITEM_MACE = 2, ITEM_MAGIC_SWORD = 3, ITEM_FIREBOLT = 5, ITEM_ICEBOLT = 6};
  enum Weapon : int64_t {WEAPON_FISTS = 0, WEAPON_MACE = 1, WEAPON_SWORD = 2, WEAPON_MAGIC_SWORD = 3};
  enum Spell : int64_t {SPELL_NONE = 0, SPELL_HEAL = 4, SPELL_FIREBOLT = 5, SPELL_ICEBOLT = 6};
  enum Creature : int64_t {CREATURE_SOLID = 0, CREATURE_SKELETAL = 1, CREATURE_TROLL = 2, CREATURE_WATER = 3};
  
  inline bool can_Equip(const Item &item) {
    return item < 4;
  }
  inline bool can_Cast(const Item &item) {
    return item > 3;
  }
  
  inline Item to_Item(const Weapon &weapon) {
    assert(weapon != Weapon::WEAPON_FISTS);
    return Item(weapon);
  }
  inline Item to_Item(const Spell &spell) {
    return Item(spell);
  }
  inline Weapon to_Weapon(const Item &item) {
    assert(can_Equip(item));
    return Weapon(item);
  }
  inline Spell to_Spell(const Item &item) {
    assert(can_Cast(item));
    return Spell(item);
  }
  
  class Move;
  class Attack;
  class Take;
  class Drop;
  class Equip;
  class Cast;
  
  class ADVENT_LINKAGE Action : public Carli::Action {
  public:
    virtual int64_t compare(const Move &rhs) const = 0;
    virtual int64_t compare(const Attack &rhs) const = 0;
    virtual int64_t compare(const Take &rhs) const = 0;
    virtual int64_t compare(const Drop &rhs) const = 0;
    virtual int64_t compare(const Equip &rhs) const = 0;
    virtual int64_t compare(const Cast &rhs) const = 0;
  };
  
  class ADVENT_LINKAGE Move : public Action {
  public:
    Move()
     : direction(Direction())
    {
    }

    Move(const Direction &direction_)
     : direction(direction_)
    {
    }

    Move(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : direction(Direction(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("direction")->second]).value))
    {
      assert(direction == DIR_NONE || direction == DIR_NORTH || direction == DIR_SOUTH || direction == DIR_EAST || direction == DIR_WEST);
    }

    Move * clone() const {
      return new Move(direction);
    }

    int64_t compare(const Carli::Action &rhs) const {return -debuggable_cast<const Action *>(&rhs)->compare(*this);}
    int64_t compare(const Move &rhs) const override {return direction - rhs.direction;}
    int64_t compare(const Attack &) const override {return -1;}
    int64_t compare(const Take &) const override {return -1;}
    int64_t compare(const Drop &) const override {return -1;}
    int64_t compare(const Equip &) const override {return -1;}
    int64_t compare(const Cast &) const override {return -1;}

    void print_impl(ostream &os) const {
      os << "move(" << int(direction) << ')';
    }

    Direction direction;
  };
  
  class ADVENT_LINKAGE Attack : public Action {
  public:
    Attack * clone() const {
      return new Attack();
    }

    int64_t compare(const Carli::Action &rhs) const {return -debuggable_cast<const Action *>(&rhs)->compare(*this);}
    int64_t compare(const Move &) const override {return 1;}
    int64_t compare(const Attack &) const override {return 0;}
    int64_t compare(const Take &) const override {return -1;}
    int64_t compare(const Drop &) const override {return -1;}
    int64_t compare(const Equip &) const override {return -1;}
    int64_t compare(const Cast &) const override {return -1;}

    void print_impl(ostream &os) const {
      os << "attack()";
    }
  };
  
  class ADVENT_LINKAGE Take : public Action {
  public:
    Take()
     : item(Item())
    {
    }

    Take(const Item &item_)
     : item(item_)
    {
    }

    Take(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : item(Item(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("item")->second]).value))
    {
      assert(item > 0 && item < 7);
    }

    Take * clone() const {
      return new Take(item);
    }

    int64_t compare(const Carli::Action &rhs) const {return -debuggable_cast<const Action *>(&rhs)->compare(*this);}
    int64_t compare(const Move &) const override {return 1;}
    int64_t compare(const Attack &) const override {return 1;}
    int64_t compare(const Take &rhs) const override {return item - rhs.item;}
    int64_t compare(const Drop &) const override {return -1;}
    int64_t compare(const Equip &) const override {return -1;}
    int64_t compare(const Cast &) const override {return -1;}

    void print_impl(ostream &os) const {
      os << "equip(" << int(item) << ')';
    }

    Item item;
  };
  
  class ADVENT_LINKAGE Drop : public Action {
  public:
    Drop()
     : item(Item())
    {
    }

    Drop(const Item &item_)
     : item(item_)
    {
    }

    Drop(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : item(Item(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("item")->second]).value))
    {
      assert(item > 0 && item < 7);
    }

    Drop * clone() const {
      return new Drop(item);
    }

    int64_t compare(const Carli::Action &rhs) const {return -debuggable_cast<const Action *>(&rhs)->compare(*this);}
    int64_t compare(const Move &) const override {return 1;}
    int64_t compare(const Attack &) const override {return 1;}
    int64_t compare(const Take &) const override {return 1;}
    int64_t compare(const Drop &rhs) const override {return item - rhs.item;}
    int64_t compare(const Equip &) const override {return -1;}
    int64_t compare(const Cast &) const override {return -1;}

    void print_impl(ostream &os) const {
      os << "equip(" << int(item) << ')';
    }

    Item item;
  };
  
  class ADVENT_LINKAGE Equip : public Action {
  public:
    Equip()
     : weapon(Weapon())
    {
    }

    Equip(const Weapon &weapon_)
     : weapon(weapon_)
    {
    }

    Equip(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : weapon(Weapon(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("item")->second]).value))
    {
      assert(can_Equip(Item(weapon)));
    }

    Equip * clone() const {
      return new Equip(weapon);
    }

    int64_t compare(const Carli::Action &rhs) const {return -debuggable_cast<const Action *>(&rhs)->compare(*this);}
    int64_t compare(const Move &) const override {return 1;}
    int64_t compare(const Attack &) const override {return 1;}
    int64_t compare(const Take &) const override {return 1;}
    int64_t compare(const Drop &) const override {return 1;}
    int64_t compare(const Equip &rhs) const override {return weapon - rhs.weapon;}
    int64_t compare(const Cast &) const override {return -1;}

    void print_impl(ostream &os) const {
      os << "equip(" << int(weapon) << ')';
    }

    Weapon weapon;
  };

  class ADVENT_LINKAGE Cast : public Action {
  public:
    Cast()
     : spell(Spell())
    {
    }

    Cast(const Spell &spell_)
     : spell(spell_)
    {
    }

    Cast(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : spell(Spell(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("item")->second]).value))
    {
      assert(can_Cast(Item(spell)));
    }

    Cast * clone() const {
      return new Cast(spell);
    }

    int64_t compare(const Carli::Action &rhs) const {return -debuggable_cast<const Action *>(&rhs)->compare(*this);}
    int64_t compare(const Move &) const override {return 1;}
    int64_t compare(const Attack &) const override {return 1;}
    int64_t compare(const Take &) const override {return 1;}
    int64_t compare(const Drop &) const override {return 1;}
    int64_t compare(const Equip &) const override {return 1;}
    int64_t compare(const Cast &rhs) const override {return spell - rhs.spell;}

    void print_impl(ostream &os) const {
      os << "cast(" << int(spell) << ')';
    }

    Spell spell;
  };

  inline std::shared_ptr<Carli::Action> make_Action(const Rete::Variable_Indices &variables, const Rete::WME_Token &token) {
    const int64_t action = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("name")->second]).value;
    
    switch(action) {
      case 1:
        return std::make_shared<Move>(variables, token);
      case 2:
        return std::make_shared<Attack>();
      case 3:
        return std::make_shared<Take>(variables, token);
      case 4:
        return std::make_shared<Drop>(variables, token);
      case 5:
        return std::make_shared<Equip>(variables, token);
      case 6:
        return std::make_shared<Cast>(variables, token);
      default:
        abort();
    }
  }

  class ADVENT_LINKAGE Items {
  public:
    int64_t size() const {
      int64_t rv = 0;
      for(auto val : items)
        rv += val;
      return rv;
    }
    
    int64_t has(const int64_t index) const {
      return items[index];
    }
    
    void give(const int64_t index) {
      assert(items.at(index) >= 0);
      ++items[index];
    }
    
    void take(const int64_t index) {
      assert(items.at(index) > 0);
      --items[index];
    }
    
    Items operator+(const Items &rhs) const {
      Items rv = *this;
      rv += rhs;
      return rv;
    }
    
    Items & operator+=(const Items &rhs) {
      for(size_t i = 0; i != items.size(); ++i)
        items[i] += rhs.items[i];
      return *this;
    }
    
    void clear() {
      items = {{0, 0, 0, 0, 0, 0, 0}};
    }
    
  private:
    std::array<int64_t, 7> items = {{0, 0, 0, 0, 0, 0, 0}};
  };
  
  class ADVENT_LINKAGE Character {
  public:
    Items items;
    int64_t items_max = 1;
    Weapon weapon = WEAPON_FISTS;
    int64_t health = 10;
    int64_t health_max = 10;
    bool is_dead = false;
    Creature creature = CREATURE_SOLID;
    
    char print_char = 'm';
    
    void receive_attack(const Weapon &enemy_weapon) {
      switch(enemy_weapon) {
        case WEAPON_FISTS:
          if(creature == CREATURE_SOLID)
            health -= 2;
          else if(creature == CREATURE_SKELETAL)
            health -= 1;
          else if(creature == CREATURE_TROLL)
            health -= 5;
          break;
          
        case WEAPON_MACE:
          if(creature == CREATURE_SOLID)
            health -= 3;
          else if(creature == CREATURE_SKELETAL)
            health -= 5;
          else if(creature == CREATURE_TROLL)
            health -= 3;
          break;
          
        case WEAPON_SWORD:
          if(creature == CREATURE_SOLID)
            health -= 5;
          else if(creature == CREATURE_SKELETAL)
            health -= 2;
          else if(creature == CREATURE_TROLL)
            health -= 4;
          break;
          
        case WEAPON_MAGIC_SWORD:
          if(creature == CREATURE_WATER)
            health -= 3;
          else if(creature == CREATURE_TROLL) {
            health -= 6;
            if(health <= 0)
              is_dead = true;
          }
          else
            health -= 10;
          break;
          
        default:
          abort();
      }
    }
    
    void receive_cast(const Spell &spell) {
      switch(spell) {
        case SPELL_HEAL:
          health = std::min(health + 5, health_max);
          break;
          
        case SPELL_FIREBOLT:
          if(creature == CREATURE_SOLID || creature == CREATURE_TROLL)
            health -= 10;
          else if(creature != CREATURE_SKELETAL)
            health -= 2;
          if(creature == CREATURE_TROLL && health <= 0)
            is_dead = true;
          break;
          
        case SPELL_ICEBOLT:
          if(creature == CREATURE_WATER)
            creature = CREATURE_SOLID;
          else if(creature != CREATURE_SKELETAL)
            health -= 5;
          break;
          
        default:
          abort();
      }
    }
  };
  
  class ADVENT_LINKAGE Room {
  public:
    Items items;
    std::shared_ptr<Character> enemy;
  };
  
  class ADVENT_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &);
    Environment & operator=(const Environment &);

  public:
    Environment();

    bool success() const;
    bool failure() const;
    
    std::shared_ptr<const Room> get_Room(const int64_t &x, const int64_t &y) const {
      return x < 0 || y < 0 || x >= int64_t(m_rooms.size()) || y >= int64_t(m_rooms.size()) ? nullptr : m_rooms[x][y];
    }
    std::shared_ptr<Room> get_Room(const int64_t &x, const int64_t &y) {
      return x < 0 || y < 0 || x >= int64_t(m_rooms.size()) || y >= int64_t(m_rooms.size()) ? nullptr : m_rooms[x][y];
    }
    
    int64_t get_Rooms_size() const {return int64_t(m_rooms.size());}
    const Character get_Player() const {return m_player;}
    std::pair<int64_t, int64_t> get_Player_pos() const {return m_player_pos;}

  private:
    void init_impl();

    std::pair<reward_type, reward_type> transition_impl(const Carli::Action &action);

    void print_impl(ostream &os) const;

    Zeni::Random m_random;
    std::array<std::array<std::shared_ptr<Room>, 5>, 5> m_rooms;
    Character m_player;
    std::pair<int64_t, int64_t> m_player_pos = std::make_pair(2, 0);
    
    std::shared_ptr<Character> m_water_dragon;
    std::shared_ptr<Character> m_troll;
  };

  class ADVENT_LINKAGE Agent : public Carli::Agent {
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

    /*
     * Objects: workspace/goal
     *          stack
     *          block
     *          name
     *
     * Relations: stack ^matches goal-stack
     *            stack ^top block
     *            block ^matches-top stack
     */

    const Rete::Symbol_Identifier_Ptr_C m_player_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("PLAYER"));
    const Rete::Symbol_Identifier_Ptr_C m_enemy_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("ENEMY"));
    const Rete::Symbol_Identifier_Ptr_C m_room_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("ROOM"));
    const Rete::Symbol_Identifier_Ptr_C m_attack_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("ATTACK"));
    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action"));
    const Rete::Symbol_Constant_String_Ptr_C m_name_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("name"));
    const Rete::Symbol_Constant_String_Ptr_C m_direction_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("direction"));
    const Rete::Symbol_Constant_String_Ptr_C m_item_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("item"));
    const Rete::Symbol_Constant_String_Ptr_C m_player_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("player"));
    const Rete::Symbol_Constant_String_Ptr_C m_enemy_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("enemy"));
    const Rete::Symbol_Constant_String_Ptr_C m_type_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("type"));
    const Rete::Symbol_Constant_String_Ptr_C m_x_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("x"));
    const Rete::Symbol_Constant_String_Ptr_C m_y_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("y"));
    const Rete::Symbol_Constant_String_Ptr_C m_dead_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("dead"));
    const Rete::Symbol_Constant_String_Ptr_C m_health_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("health"));
    const Rete::Symbol_Constant_String_Ptr_C m_in_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("in"));
    const Rete::Symbol_Constant_String_Ptr_C m_has_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("has"));
    const Rete::Symbol_Constant_String_Ptr_C m_equipped_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("equipped"));
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("true"));
    const Rete::Symbol_Constant_String_Ptr_C m_false_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("false"));
    const Rete::Symbol_Constant_Int_Ptr_C m_move_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_attack_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_take_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_Int_Ptr_C m_drop_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(4));
    const Rete::Symbol_Constant_Int_Ptr_C m_equip_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(5));
    const Rete::Symbol_Constant_Int_Ptr_C m_cast_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(6));

    std::map<Direction, Rete::Symbol_Identifier_Ptr_C> m_move_ids;
    std::map<Item, Rete::Symbol_Identifier_Ptr_C> m_take_ids;
    std::map<Item, Rete::Symbol_Identifier_Ptr_C> m_drop_ids;
    std::map<Weapon, Rete::Symbol_Identifier_Ptr_C> m_equip_ids;
    std::map<Spell, Rete::Symbol_Identifier_Ptr_C> m_cast_ids;
    std::map<Direction, Rete::Symbol_Constant_Int_Ptr_C> m_direction_values;
    std::map<Item, Rete::Symbol_Constant_Int_Ptr_C> m_item_values;
//     std::map<Weapon, Rete::Symbol_Constant_Int_Ptr_C> m_weapon_values;
//     std::map<Spell, Rete::Symbol_Constant_Int_Ptr_C> m_spell_values;
    std::map<int64_t, Rete::Symbol_Constant_Int_Ptr_C> m_position_values;
    std::map<int64_t, Rete::Symbol_Constant_Int_Ptr_C> m_health_values;
    std::map<Creature, Rete::Symbol_Constant_Int_Ptr_C> m_creature_values;

    std::list<Rete::WME_Ptr_C> m_wmes_prev;

//    double m_feature_generation_time = 0.0;
  };

}

#endif
