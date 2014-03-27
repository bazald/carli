#ifndef INFINITE_MARIO_H
#define INFINITE_MARIO_H

#include "carli/agent.h"
#include "carli/environment.h"

#include "jni_layer.h"

#include <array>
#include <stdexcept>

namespace Mario {

  typedef std::array<bool, 5> Action;
  class State;

  void infinite_mario_ai(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, Action &action);
  
  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class Feature;
  class Feature_Position;
  class Feature_Mode;
  class Feature_Flag;
  class Feature_Button;

  class Feature : public Carli::Feature {
  public:
    Feature() {}

    virtual Feature * clone() const = 0;

    int64_t compare_axis(const Carli::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int64_t compare_axis(const Feature &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Position &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Mode &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Flag &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Button &rhs) const = 0;

    virtual Rete::WME_Token_Index wme_token_index() const = 0;
  };
  
  class Feature_Position : public Carli::Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {X = 1, Y = 2};

    Feature_Position(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, true)
    {
    }

    Feature_Position * clone() const {
      return new Feature_Position(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int64_t compare_axis(const Mario::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }
    int64_t compare_axis(const Feature_Mode &) const {return -1;}
    int64_t compare_axis(const Feature_Flag &) const {return -1;}
    int64_t compare_axis(const Feature_Button &) const {return -1;}

    Rete::WME_Token_Index wme_token_index() const {
      return axis;
    }

    void print(ostream &os) const {
      switch(axis.first) {
        case X: os << "x"; break;
        case Y: os << "y"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };
  
  class Feature_Mode : public Carli::Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {MODE = 3};

    Feature_Mode(const Mode &mode)
     : Feature_Enumerated<Feature>(mode)
    {
    }

    Feature_Mode * clone() const {
      return new Feature_Mode(Mode(value));
    }

    int64_t compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 0;}
    int64_t compare_axis(const Feature_Flag &) const {return -1;}
    int64_t compare_axis(const Feature_Button &) const {return -1;}

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(MODE, 2);
    }

    void print(ostream &os) const {
      os << "mode(" << value << ')';
    }
  };
  
  class Feature_Flag : public Carli::Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {ON_GROUND = 4, MAY_JUMP = 5, IS_CARRYING = 6};

    Feature_Flag(const Axis &axis_, const bool &flag)
     : Feature_Enumerated<Feature>(flag), axis(axis_)
    {
    }

    Feature_Flag * clone() const {
      return new Feature_Flag(axis, value != 0);
    }

    int64_t compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 1;}
    int64_t compare_axis(const Feature_Flag &rhs) const {
      return axis - rhs.axis;
    }
    int64_t compare_axis(const Feature_Button &) const {return -1;}

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(axis, 2);
    }

    void print(ostream &os) const {
      switch(axis) {
      case ON_GROUND   : os << "on-ground";   break;
      case MAY_JUMP    : os << "may-jump";    break;
      case IS_CARRYING : os << "is-carrying"; break;
      }

      os << '(' << value << ')';
    }

    Axis axis;
  };
  
  class Feature_Button : public Carli::Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {LEFT = 7 + BUTTON_LEFT,
                        RIGHT = 7 + BUTTON_RIGHT,
                        DOWN = 7 + BUTTON_DOWN,
                        JUMP = 7 + BUTTON_JUMP,
                        SPEED = 7 + BUTTON_SPEED};

    Feature_Button(const Axis &axis_, const bool &flag)
     : Feature_Enumerated<Feature>(flag), axis(axis_)
    {
    }

    Feature_Button * clone() const {
      return new Feature_Button(axis, value != 0);
    }

    int64_t compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 1;}
    int64_t compare_axis(const Feature_Flag &) const {return 1;}
    int64_t compare_axis(const Feature_Button &rhs) const {
      return axis - rhs.axis;
    }

    Rete::WME_Token_Index wme_token_index() const {
      return Rete::WME_Token_Index(axis, 2);
    }

    void print(ostream &os) const {
      switch(axis) {
      case LEFT  : os << "left";  break;
      case RIGHT : os << "right"; break;
      case DOWN  : os << "down";  break;
      case JUMP  : os << "jump";  break;
      case SPEED : os << "speed"; break;
      }

      os << '(' << value << ')';
    }

    Axis axis;
  };
  
  class Button_Presses : public Carli::Action {
  public:
    Button_Presses() {
      memset(&action, 0, sizeof(action));
    }

    Button_Presses(const Mario::Action &action_)
     : action(action_)
    {
    }

    Button_Presses(const Rete::WME_Token &token) {
      for(int i = 0; i != BUTTON_END; ++i)
        action[i] = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(7 + i, 2)]).value != 0;
    }

    Button_Presses * clone() const {
      return new Button_Presses(action);
    }

    int64_t compare(const Action &rhs) const {
      return compare(debuggable_cast<const Button_Presses &>(rhs));
    }

    int64_t compare(const Button_Presses &rhs) const {
      for(int i = 0; i != BUTTON_END; ++i) {
        if(action[i] != rhs.action[i])
          return action[i] - rhs.action[i];
      }

      return 0;
    }

    void print_impl(ostream &os) const {
      os << "button-presses(";
      for(int i = 0; i != BUTTON_END; ++i)
        os << action[i];
      os << ')';
    }

    Mario::Action action;
  };

  class Agent : public Carli::Agent {
  public:
    Agent(const std::shared_ptr<State> &state);
    ~Agent();

  private:
    template<typename SUBFEATURE, typename AXIS>
    void generate_rete_continuous(const Carli::Node_Unsplit_Ptr &node_unsplit,
                                  const std::function<action_ptrsc(const Rete::WME_Token &token)> &get_action,
                                  const AXIS &axis,
                                  const double &lower_bound,
                                  const double &upper_bound);
    void generate_rete();

    void generate_features();

    void update();

    const Rete::Symbol_Variable_Ptr_C m_first_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::First));
    const Rete::Symbol_Variable_Ptr_C m_third_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::Third));

    const Rete::Symbol_Identifier_Ptr_C m_input_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("I1"));
    const Rete::Symbol_Constant_String_Ptr_C m_input_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("input"));
    const Rete::Symbol_Constant_String_Ptr_C m_button_presses_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("button-presses"));
    const Rete::Symbol_Constant_String_Ptr_C m_x_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("x"));
    const Rete::Symbol_Constant_String_Ptr_C m_y_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("y"));
    const Rete::Symbol_Constant_String_Ptr_C m_mode_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("mode"));
    const Rete::Symbol_Constant_String_Ptr_C m_on_ground_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("on-ground"));
    const Rete::Symbol_Constant_String_Ptr_C m_may_jump_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("may-jump"));
    const Rete::Symbol_Constant_String_Ptr_C m_is_carrying_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("is-carrying"));
    const Rete::Symbol_Constant_String_Ptr_C m_left_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("left"));
    const Rete::Symbol_Constant_String_Ptr_C m_right_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("right"));
    const Rete::Symbol_Constant_String_Ptr_C m_down_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("down"));
    const Rete::Symbol_Constant_String_Ptr_C m_jump_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("jump"));
    const Rete::Symbol_Constant_String_Ptr_C m_speed_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("speed"));
    const Rete::Symbol_Constant_String_Ptr_C m_true_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("true"));
    const Rete::Symbol_Constant_String_Ptr_C m_false_value = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("false"));

    std::list<Rete::WME_Ptr_C> m_wmes_prev;
  };

}

#endif
