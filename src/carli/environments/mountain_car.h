#ifndef MOUNTAIN_CAR_H
#define MOUNTAIN_CAR_H

#include "../agent.h"
#include "../environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>

namespace Mountain_Car {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::make_pair;
  using std::map;
  using std::ostream;
  using std::pair;
  using std::set;
  using std::shared_ptr;

  class Feature;
  class Feature : public Feature_Ranged {
  public:
    enum Axis : size_t {X = 0, X_DOT = 1};

    Feature(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_)
    {
    }

    Feature * clone() const {
      return new Feature(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    void print(ostream &os) const {
      switch(axis.first) {
        case X:     os << 'x';     break;
        case X_DOT: os << "x-dot"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Move : public Action {
  public:
    enum Direction : char {LEFT = 0, IDLE = 1, RIGHT = 2};

    Move(const Direction &direction_ = IDLE)
     : direction(direction_)
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
      os << "move(";

      switch(direction) {
        case LEFT:  os << "left";  break;
        case IDLE:  os << "idle";  break;
        case RIGHT: os << "right"; break;
        default: abort();
      }

      os << ')';
    }

    Direction direction;
  };

  class Environment : public ::Environment {
  public:
    typedef pair<double, double> double_pair;

    Environment();

    const double & get_x() const {return m_x;}
    const double & get_x_dot() const {return m_x_dot;}
//    const double & get_value(const Feature_Axis &index) const {return *(&m_x + index);}
    bool is_random_start() const {return m_random_start;}
    bool is_reward_negative() const {return m_reward_negative;}

    void set_x(const double &x_) {m_x = x_;}
    void set_x_dot(const double &x_dot_) {m_x_dot = x_dot_;}

    bool success() const {
      return m_x >= m_goal_position;
    }

  private:
    void init_impl();

    void alter_impl();

    reward_type transition_impl(const Action &action);

    void print_impl(ostream &os) const;

    Zeni::Random m_random_init;

    double m_x = 0.0;
    double m_x_dot = 0.0;

    const double m_min_position = -1.2;
    const double m_max_position = 0.6;
    const double m_max_velocity = 0.07;
    const double m_goal_position = 0.5;

    double m_cart_force = 0.001;
    double m_grav_force = 0.0025;

    const bool m_random_start = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["random-start"]).get_value();
    const bool m_reward_negative = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["reward-negative"]).get_value();
  };

  class Agent : public ::Agent {
  public:
    Agent(const shared_ptr< ::Environment> &env);
    ~Agent();

    void print_policy(ostream &os, const size_t &granularity);

  private:
    void generate_cmac(const Rete::Rete_Node_Ptr &parent);

    void generate_rete(const Rete::Rete_Node_Ptr &parent);

    void generate_features();

    void update();

    const double m_min_x = -1.2;
    const double m_max_x = 0.6;
    const double m_min_x_dot = -0.07;
    const double m_max_x_dot = 0.07;

    Rete::Symbol_Variable_Ptr_C m_first_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::First);
    Rete::Symbol_Variable_Ptr_C m_third_var = std::make_shared<Rete::Symbol_Variable>(Rete::Symbol_Variable::Third);

    Rete::Symbol_Constant_String_Ptr_C m_x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    Rete::Symbol_Constant_String_Ptr_C m_x_dot_attr = std::make_shared<Rete::Symbol_Constant_String>("x-dot");
    Rete::Symbol_Constant_Float_Ptr m_x_value = std::make_shared<Rete::Symbol_Constant_Float>(dynamic_pointer_cast<Environment>(get_env())->get_x());
    Rete::Symbol_Constant_Float_Ptr m_x_dot_value = std::make_shared<Rete::Symbol_Constant_Float>(dynamic_pointer_cast<Environment>(get_env())->get_x_dot());

    Rete::WME_Ptr_C m_x_wme;
    Rete::WME_Ptr_C m_x_dot_wme;

    std::array<std::shared_ptr<const Action>, 3> m_action = {{std::make_shared<Move>(Move::LEFT),
                                                              std::make_shared<Move>(Move::IDLE),
                                                              std::make_shared<Move>(Move::RIGHT)}};
  };

}

#endif
