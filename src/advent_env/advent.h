#ifndef ADVENT_H
#define ADVENT_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <list>
#include <stdexcept>

#if !defined(_WINDOWS)
#define ADVENT_LINKAGE
#elif !defined(BLOCKS_WORLD_2_INTERNAL)
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
  enum Item : int64_t {ITEM_NONE = 0, SWORD = 1, MACE = 2, MAGIC_SWORD = 3, SCROLL_HEAL = 4, SCROLL_FIREBOLT = 5, SCROLL_ICEBOLT = 6};
  enum Weapon : int64_t {WEAPON_FISTS = 0, WEAPON_MACE = 1, WEAPON_SWORD = 2, WEAPON_MAGIC_SWORD = 3};
  enum Spell : int64_t {SPELL_NONE = 0, SPELL_HEAL = 4, SPELL_FIREBOLT = 5, SPELL_ICEBOLT = 6};
  
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
  
  class ADVENT_LINKAGE Move : public Carli::Action {
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
  
  class ADVENT_LINKAGE Attack : public Carli::Action {
  public:
    Attack * clone() const {
      return new Attack();
    }

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Attack &>(rhs));
    }

    int64_t compare(const Attack &) const {
      return 0;
    }

    void print_impl(ostream &os) const {
      os << "attack()";
    }
  };
  
  class ADVENT_LINKAGE Take : public Carli::Action {
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

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Take &>(rhs));
    }

    int64_t compare(const Take &rhs) const {
      return item - rhs.item;
    }

    void print_impl(ostream &os) const {
      os << "equip(" << item << ')';
    }

    Item item;
  };
  
  class ADVENT_LINKAGE Drop : public Carli::Action {
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

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Drop &>(rhs));
    }

    int64_t compare(const Drop &rhs) const {
      return item - rhs.item;
    }

    void print_impl(ostream &os) const {
      os << "equip(" << item << ')';
    }

    Item item;
  };
  
  class ADVENT_LINKAGE Equip : public Carli::Action {
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

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Equip &>(rhs));
    }

    int64_t compare(const Equip &rhs) const {
      return weapon - rhs.weapon;
    }

    void print_impl(ostream &os) const {
      os << "equip(" << weapon << ')';
    }

    Weapon weapon;
  };

  class ADVENT_LINKAGE Cast : public Carli::Action {
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

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Cast &>(rhs));
    }

    int64_t compare(const Cast &rhs) const {
      return spell - rhs.spell;
    }

    void print_impl(ostream &os) const {
      os << "equip(" << spell << ')';
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
      assert(items.at(index));
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
    
    bool is_fleshy = false;
    bool is_skeletal = false;
    bool is_troll = false;
    bool is_water = false;
    
    char print_char = 'm';
    
    void receive_attack(const Weapon &weapon) {
      switch(weapon) {
        case WEAPON_FISTS:
          if(is_fleshy)
            health -= 2;
          else if(is_skeletal)
            health -= 1;
          else if(is_troll)
            health -= 1;
          break;
          
        case WEAPON_MACE:
          if(is_fleshy)
            health -= 3;
          else if(is_skeletal)
            health -= 5;
          else if(is_troll)
            health -= 3;
          break;
          
        case WEAPON_SWORD:
          if(is_fleshy)
            health -= 5;
          else if(is_skeletal)
            health -= 3;
          else if(is_troll)
            health -= 3;
          break;
          
        case WEAPON_MAGIC_SWORD:
          if(is_water)
            health -= 2;
          else if(is_troll) {
            health -= 11;
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
          if(is_fleshy)
            health -= 2;
          else if(is_skeletal)
            health -= 1;
          break;
          
        case SPELL_FIREBOLT:
          if(is_fleshy)
            health -= 10;
          else if(!is_skeletal)
            health -= 5;
          if(is_troll && health <= 0)
            is_dead = true;
          break;
          
        case SPELL_ICEBOLT:
          if(is_water) {
            health -= 10;
            is_water = false;
          }
          else if(!is_skeletal)
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
    std::array<std::array<std::shared_ptr<Room>, 7>, 7> m_rooms;
    Character m_player;
    std::pair<int64_t, int64_t> m_player_pos = std::make_pair(3, 0);
    
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

    const Rete::Symbol_Constant_String_Ptr_C m_action_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("action"));
    const Rete::Symbol_Constant_String_Ptr_C m_name_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("name"));
    const Rete::Symbol_Constant_String_Ptr_C m_direction_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("direction"));
    const Rete::Symbol_Constant_String_Ptr_C m_item_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("item"));
//     const Rete::Symbol_Constant_String_Ptr_C m_weapon_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("weapon"));
//     const Rete::Symbol_Constant_String_Ptr_C m_spell_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("spell"));
    const Rete::Symbol_Constant_Int_Ptr_C m_move_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_attack_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(2));
    const Rete::Symbol_Constant_Int_Ptr_C m_take_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(3));
    const Rete::Symbol_Constant_Int_Ptr_C m_drop_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(4));
    const Rete::Symbol_Constant_Int_Ptr_C m_equip_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(5));
    const Rete::Symbol_Constant_Int_Ptr_C m_cast_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(6));

    std::map<Direction, Rete::Symbol_Identifier_Ptr_C> m_move_ids;
    std::map<Item, Rete::Symbol_Constant_Int_Ptr_C> m_item_ids;
//     std::map<Weapon, Rete::Symbol_Constant_Int_Ptr_C> m_weapon_ids;
//     std::map<Spell, Rete::Symbol_Constant_Int_Ptr_C> m_spell_ids;
    std::map<int64_t, Rete::Symbol_Constant_Int_Ptr_C> m_position_ids;
    std::map<Direction, Rete::Symbol_Constant_Int_Ptr_C> m_direction_values;

    std::list<Rete::WME_Ptr_C> m_wmes_prev;

//    double m_feature_generation_time = 0.0;
  };

}

#endif
