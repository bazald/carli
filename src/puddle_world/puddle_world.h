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

namespace Puddle_World {
  enum Direction : char {NORTH, SOUTH, EAST, WEST};
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

  class Feature;
  class Position;
  class Move_Direction;

  class Feature : public Carli::Feature {
  public:
    Feature() {}

    virtual Feature * clone() const = 0;

    int compare_axis(const Carli::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int compare_axis(const Feature &rhs) const = 0;
    virtual int compare_axis(const Position &rhs) const = 0;
    virtual int compare_axis(const Move_Direction &rhs) const = 0;

    virtual Rete::WME_Token_Index wme_token_index() const = 0;
  };

  class Position : public Carli::Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {X = 0, Y = 1};

    Position(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, false)
    {
    }

    Position * clone() const {
      return new Position(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int compare_axis(const Puddle_World::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Position &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }
    int compare_axis(const Move_Direction &) const {
      return -1;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return axis;
    }

    void print(ostream &os) const {
      switch(axis.first) {
        case X: os << 'x'; break;
        case Y: os << 'y'; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Move_Direction : public Carli::Feature {
  public:
    enum Index {index = 2};

    Move_Direction(const Direction &direction_)
     : direction(direction_)
    {
    }

    Move_Direction * clone() const {
      return new Move_Direction(direction);
    }

    int get_depth() const {
      return 0;
    }

    int compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int compare_axis(const Position &) const {
      return 1;
    }
    int compare_axis(const Move_Direction &rhs) const {
      return direction - debuggable_cast<const Move_Direction &>(rhs).direction;
    }

    int compare_value(const Carli::Feature &) const {
      return 0;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(2, 2);
    }

    void print(ostream &os) const {
      os << "move(" << direction << ')';
    }

    Direction direction;
  };

  class Move : public Carli::Action {
  public:
    Move(const Direction &direction_ = NORTH)
     : direction(direction_)
    {
    }

    Move(const Rete::WME_Token &token)
     : direction(Direction(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Move_Direction::index, 2)]).value))
    {
    }

    Move * clone() const {
      return new Move(direction);
    }

    int compare(const Action &rhs) const {
      return direction - debuggable_cast<const Move &>(rhs).direction;
    }

    int compare(const Move &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "move(" << direction << ')';
    }

    Direction direction;
  };

  class Environment : public Carli::Environment {
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
    const bool m_random_start = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["random-start"]).get_value();

    std::vector<Puddle> m_horizontal_puddles;
    std::vector<Puddle> m_vertical_puddles;

    double m_noise = 0.01;
  };

  class Agent : public Carli::Agent {
  public:
    Agent(const shared_ptr<Carli::Environment> &env);
    ~Agent();

    void print_policy(ostream &os, const size_t &granularity);

  private:
    void generate_cmac(const Rete::Rete_Node_Ptr &parent);

    void generate_rete(const Rete::Rete_Node_Ptr &parent);

    void generate_features();

    void update();

    const Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    const Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    const Rete::Symbol_Constant_Float_Ptr m_x_value = std::make_shared<Rete::Symbol_Constant_Float>(dynamic_pointer_cast<Environment>(get_env())->get_position().first);
    const Rete::Symbol_Constant_Float_Ptr m_y_value = std::make_shared<Rete::Symbol_Constant_Float>(dynamic_pointer_cast<Environment>(get_env())->get_position().second);

    Rete::WME_Ptr_C m_x_wme;
    Rete::WME_Ptr_C m_y_wme;

    std::array<std::shared_ptr<const Carli::Action>, 4> m_action = {{std::make_shared<Move>(NORTH),
                                                                     std::make_shared<Move>(SOUTH),
                                                                     std::make_shared<Move>(EAST),
                                                                     std::make_shared<Move>(WEST)}};
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