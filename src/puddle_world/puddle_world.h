#ifndef PUDDLE_WORLD_H
#define PUDDLE_WORLD_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>
#include <vector>

#if !defined(_WINDOWS)
#define PUDDLE_WORLD_LINKAGE
#elif !defined(PUDDLE_WORLD_INTERNAL)
#define PUDDLE_WORLD_LINKAGE __declspec(dllimport)
#else
#define PUDDLE_WORLD_LINKAGE __declspec(dllexport)
#endif

namespace Puddle_World {
  enum Direction : char {NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3};
}

inline std::ostream & operator<<(std::ostream &os, const Puddle_World::Direction &direction);

namespace Puddle_World {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::make_pair;
  using std::map;
  using std::ostream;
  using std::pair;
  using std::set;
  using std::shared_ptr;

  class PUDDLE_WORLD_LINKAGE Move : public Carli::Action {
  public:
    Move(const Direction &direction_ = NORTH)
     : direction(direction_)
    {
    }

    Move(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : direction(Direction(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("move")->second]).value))
    {
    }

    Move * clone() const {
      return new Move(direction);
    }

    int64_t compare(const Action &rhs) const {
      return direction - debuggable_cast<const Move &>(rhs).direction;
    }

    int64_t compare(const Move &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "move(" << direction << ')';
    }

    Direction direction;
  };

  class PUDDLE_WORLD_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &) = delete;
    Environment & operator=(const Environment &) = delete;

    typedef std::array<double, 4> Puddle;

  public:
    typedef pair<double, double> double_pair;

    Environment();

    const double_pair & get_position() const {return m_position;}
//    const double & get_value(const Feature_Axis &index) const {return *(&m_position.first + index);}
    bool is_random_start() const {return m_random_start;}

    void set_position(const double_pair &position_) {m_position = position_;}

    bool goal_reached() const;

  private:
    void init_impl();

    void alter_impl();

    reward_type transition_impl(const Carli::Action &action);

    double horizontal_puddle_reward(const double &left, const double &right, const double &y, const double &radius) const;

    double vertical_puddle_reward(const double &x, const double &bottom, const double &top, const double &radius) const;

    template <typename TYPE>
    TYPE pythagoras(const TYPE &lhs, const TYPE &rhs) const {
      return sqrt(squared(lhs) + squared(rhs));
    }

    template <typename TYPE>
    TYPE squared(const TYPE &value) const {
      return value * value;
    }

    void print_impl(ostream &os) const;

    Zeni::Random m_random_init;
    Zeni::Random m_random_motion;

    double_pair m_position;

    double_pair m_init_x;
    double_pair m_init_y;

    bool m_goal_dynamic = false;
    double_pair m_goal_x;
    double_pair m_goal_y;

    size_t m_step_count = 0lu;
    const bool m_random_start = get_Option_Ranged<bool>(Options::get_global(), "random-start");

    std::vector<Puddle> m_horizontal_puddles;
    std::vector<Puddle> m_vertical_puddles;

    double m_noise = 0.01;
  };

  class PUDDLE_WORLD_LINKAGE Agent : public Carli::Agent {
  public:
    Agent(const shared_ptr<Carli::Environment> &env);
    ~Agent();

    void print_policy(ostream &os, const size_t &granularity);

  private:
    void generate_cmac(const Rete::Rete_Node_Ptr &parent);

    void generate_features();

    void update();

    Rete::Symbol_Constant_Float_Ptr_C m_x_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_position().first));
    Rete::Symbol_Constant_Float_Ptr_C m_y_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_position().second));

    Rete::WME_Ptr m_x_wme;
    Rete::WME_Ptr m_y_wme;

    std::array<std::shared_ptr<const Carli::Action>, 4> m_action = {{std::shared_ptr<const Carli::Action>(new Move(NORTH)),
                                                                     std::shared_ptr<const Carli::Action>(new Move(SOUTH)),
                                                                     std::shared_ptr<const Carli::Action>(new Move(EAST)),
                                                                     std::shared_ptr<const Carli::Action>(new Move(WEST))}};
  };

}

std::ostream & operator<<(std::ostream &os, const Puddle_World::Direction &direction) {
  switch(direction) {
    case Puddle_World::NORTH: os << "north"; break;
    case Puddle_World::SOUTH: os << "south"; break;
    case Puddle_World::EAST:  os << "east";  break;
    case Puddle_World::WEST:  os << "west";  break;
    default: abort();
  }

  return os;
}

#endif
