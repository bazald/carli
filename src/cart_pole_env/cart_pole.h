#ifndef CART_POLE_H
#define CART_POLE_H

#include "carli/agent.h"
#include "carli/environment.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <list>
#include <stdexcept>

#if !defined(_WINDOWS)
#define CART_POLE_LINKAGE
#elif !defined(CART_POLE_INTERNAL)
#define CART_POLE_LINKAGE __declspec(dllimport)
#else
#define CART_POLE_LINKAGE __declspec(dllexport)
#endif

namespace Cart_Pole {
  enum Direction : char {LEFT = 0, RIGHT = 1};
}

inline std::ostream & operator<<(std::ostream &os, const Cart_Pole::Direction &direction);

namespace Cart_Pole {

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::make_pair;
  using std::map;
  using std::ostream;
  using std::pair;
  using std::set;
  using std::shared_ptr;

  class CART_POLE_LINKAGE Move: public Carli::Action {
  public:
    Move(const Direction &direction_ = LEFT)
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
      return compare(debuggable_cast<const Move &>(rhs));
    }

    int64_t compare(const Move &rhs) const {
      return direction - rhs.direction;
    }

    void print_impl(ostream &os) const {
      os << "move(";

      switch(direction) {
        case LEFT:  os << "left";  break;
        case RIGHT: os << "right"; break;
        default: abort();
      }

      os << ')';
    }

    Direction direction;
  };

  class CART_POLE_LINKAGE Environment : public Carli::Environment {
    Environment(const Environment &);
    Environment & operator=(const Environment &);

  public:
    typedef pair<double, double> double_pair;

    Environment();

    const double & get_theta() const {return m_theta;}
    const double & get_theta_dot() const {return m_theta_dot;}
    const double & get_x() const {return m_x;}
    const double & get_x_dot() const {return m_x_dot;}
//    const double & get_value(const Feature_Axis &index) const {return *(&m_x + index);}
    const double & get_max_theta() const {return m_max_theta;}
    const double & get_max_theta_dot() const {return m_max_theta_dot;}
    const double & get_max_x() const {return m_max_x;}
    const double & get_max_x_dot() const {return m_max_x_dot;}
    bool has_goal() const {return m_has_goal;}
    bool is_ignoring_x() const {return m_ignore_x;}

    void set_theta_dot(const double &theta_dot_) {m_theta = theta_dot_;}
    void set_theta(const double &theta_) {m_theta = theta_;}
    void set_x(const double &x_) {m_x = x_;}
    void set_x_dot(const double &x_dot_) {m_x = x_dot_;}

    bool success() const;
    bool failed() const;

  private:
    void init_impl();

    reward_type transition_impl(const Carli::Action &action);

    void print_impl(ostream &os) const;

    Zeni::Random m_random_init;
    Zeni::Random m_random_motion;

    double m_theta = 0.0;
    double m_theta_dot = 0.0;
    double m_x = 0.0;
    double m_x_dot = 0.0;

    const double GRAVITY = 9.8;
    const double MASSCART = 1.0;
    const double MASSPOLE = 0.1;
    const double TOTAL_MASS = MASSPOLE + MASSCART;
    const double LENGTH = 0.5;      /* actually half the pole's length */
    const double POLEMASS_LENGTH = MASSPOLE * LENGTH;
    const double FORCE_MAG = 10.0;
    const double TAU = 0.02;      /* seconds between state updates */
    const double FOURTHIRDS = 1.3333333333333;

    const bool m_has_goal = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["set-goal"]).get_value();
    const bool m_ignore_x = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["ignore-x"]).get_value();

    const double m_max_theta = m_has_goal ? 1.57079632679 : 0.2094384; ///< pi/2.0 : 12 degrees
    const double m_max_theta_dot = m_has_goal ? 2.0 : 10.0;
    const double m_max_x = m_has_goal ? 10.0 : 2.4;
    const double m_max_x_dot = m_has_goal ? 4.0 : 10.0;
  };

  class CART_POLE_LINKAGE Agent : public Carli::Agent {
    Agent(const Agent &);
    Agent & operator=(const Agent &);

  public:
    Agent(const std::shared_ptr<Carli::Environment> &env);
    ~Agent();

    bool is_ignoring_x() const {return m_ignore_x;}

  private:
    void generate_rete();
    void generate_features();

    void update();

    const bool m_ignore_x = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["ignore-x"]).get_value();

    Rete::Symbol_Constant_Float_Ptr_C m_x_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_x()));
    Rete::Symbol_Constant_Float_Ptr_C m_x_dot_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_x_dot()));
    Rete::Symbol_Constant_Float_Ptr_C m_theta_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_theta()));
    Rete::Symbol_Constant_Float_Ptr_C m_theta_dot_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(dynamic_pointer_cast<Environment>(get_env())->get_theta_dot()));

    Rete::WME_Ptr m_x_wme;
    Rete::WME_Ptr m_x_dot_wme;
    Rete::WME_Ptr m_theta_wme;
    Rete::WME_Ptr m_theta_dot_wme;

    /// http://msdn.microsoft.com/en-us/library/dn793970.aspx
    std::array<std::shared_ptr<const Carli::Action>, 2> m_action;
  };

}

std::ostream & operator<<(std::ostream &os, const Cart_Pole::Direction &direction) {
  switch(direction) {
    case Cart_Pole::LEFT: os << "left"; break;
    case Cart_Pole::RIGHT: os << "right"; break;
    default: abort();
  }

  return os;
}

#endif
