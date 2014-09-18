#ifndef MOUNTAIN_CAR_H
#define MOUNTAIN_CAR_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>

#if !defined(_WINDOWS)
#define MOUNTAIN_CAR_LINKAGE
#elif !defined(MOUNTAIN_CAR_INTERNAL)
#define MOUNTAIN_CAR_LINKAGE __declspec(dllimport)
#else
#define MOUNTAIN_CAR_LINKAGE __declspec(dllexport)
#endif

namespace Mountain_Car {
  enum Direction : char {LEFT = 0, IDLE = 1, RIGHT = 2};
}

inline std::ostream & operator<<(std::ostream &os, const Mountain_Car::Direction &direction);

namespace Mountain_Car {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::make_pair;
  using std::map;
  using std::ostream;
  using std::pair;
  using std::set;
  using std::shared_ptr;

  class MOUNTAIN_CAR_LINKAGE Acceleration : public Carli::Action {
  public:
    Acceleration(const Direction &direction_ = IDLE)
     : direction(direction_)
    {
    }

    Acceleration(const Rete::Variable_Indices &variables, const Rete::WME_Token &token)
     : direction(Direction(debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("acceleration")->second]).value))
    {
    }

    Acceleration * clone() const {
      return new Acceleration(direction);
    }

    int64_t compare(const Action &rhs) const {
      return direction - debuggable_cast<const Acceleration &>(rhs).direction;
    }

    int64_t compare(const Acceleration &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "acceleration(" << direction << ')';
    }

    Direction direction;
  };

  class MOUNTAIN_CAR_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &);
    Environment & operator=(const Environment &);

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

    reward_type transition_impl(const Carli::Action &action);

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

    const bool m_random_start = get_Option_Ranged<bool>(Options::get_global(), "random-start");
    const bool m_reward_negative = get_Option_Ranged<bool>(Options::get_global(), "reward-negative");
  };

  class MOUNTAIN_CAR_LINKAGE Agent : public Carli::Agent {
  public:
    Agent(const shared_ptr<Carli::Environment> &env);
    ~Agent();

    void print_policy(ostream &os, const size_t &granularity);

  private:
    void generate_cmac(const Rete::Rete_Node_Ptr &parent);

    void generate_features();

    void update();

    const double m_min_x = -1.2;
    const double m_max_x = 0.6;
    const double m_min_x_dot = -0.07;
    const double m_max_x_dot = 0.07;

    const Rete::Symbol_Constant_String_Ptr_C m_x_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("x"));
    const Rete::Symbol_Constant_String_Ptr_C m_x_dot_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("x-dot"));
    Rete::Symbol_Constant_Float_Ptr_C m_x_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_x()));
    Rete::Symbol_Constant_Float_Ptr_C m_x_dot_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_x_dot()));

    Rete::WME_Ptr m_x_wme;
    Rete::WME_Ptr m_x_dot_wme;
    
    /// http://msdn.microsoft.com/en-us/library/dn793970.aspx
    std::array<std::shared_ptr<const Carli::Action>, 3> m_action;
  };

}

std::ostream & operator<<(std::ostream &os, const Mountain_Car::Direction &direction) {
  switch(direction) {
    case Mountain_Car::LEFT:  os << "left";  break;
    case Mountain_Car::IDLE:  os << "idle";  break;
    case Mountain_Car::RIGHT: os << "right"; break;
    default: abort();
  }

  return os;
}

#endif
