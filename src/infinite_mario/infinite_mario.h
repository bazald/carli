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
  void infinite_mario_reinit(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current);

  using std::dynamic_pointer_cast;
  using std::endl;
  using std::ostream;

  class Feature;
  class Feature_Position;
  class Feature_Velocity;
  class Feature_Mode;
  class Feature_Flag;
  class Feature_Numeric;
  class Feature_Button;

  bool tile_can_jump_into(const Tile &tile);
  bool tile_can_jump_through(const Tile &tile);
  bool tile_can_land_on(const Tile &tile);
  bool tile_can_pass_through(const Tile &tile);

  bool object_dangerous(const Object &object);
  bool object_flies(const Object &object);
  bool object_powerup(const Object &object);
  bool object_killable_by_fireball(const Object &object);
  bool object_killable_by_jump(const Object &object);

  class Feature : public Carli::Feature {
  public:
    Feature() {}

    virtual Feature * clone() const = 0;

    int64_t compare_axis(const Carli::Feature &rhs) const {
      return compare_axis(debuggable_cast<const Feature &>(rhs));
    }

    virtual int64_t compare_axis(const Feature &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Position &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Velocity &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Mode &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Flag &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Numeric &rhs) const = 0;
    virtual int64_t compare_axis(const Feature_Button &rhs) const = 0;

    enum {ACTION_INDEX = 21};
    Rete::WME_Bindings bindings() const override {
      Rete::WME_Bindings bindings;
      bindings.insert(std::make_pair(Rete::WME_Token_Index(ACTION_INDEX, 2), Rete::WME_Token_Index(ACTION_INDEX, 2)));
      return bindings;
    }
  };
  
  class Feature_Position : public Carli::Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {X = 0,
                        Y = X + 1};

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
    int64_t compare_axis(const Feature_Velocity &) const {return -1;}
    int64_t compare_axis(const Feature_Mode &) const {return -1;}
    int64_t compare_axis(const Feature_Flag &) const {return -1;}
    int64_t compare_axis(const Feature_Numeric &) const {return -1;}
    int64_t compare_axis(const Feature_Button &) const {return -1;}

    void print(ostream &os) const {
      switch(axis.first) {
        case X: os << "x"; break;
        case Y: os << "y"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Feature_Velocity : public Carli::Feature_Ranged<Feature> {
  public:
    enum Axis : size_t {X_DOT = Feature_Position::Y + 1,
                        Y_DOT = X_DOT + 1};

    Feature_Velocity(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, true)
    {
    }

    Feature_Velocity * clone() const {
      return new Feature_Velocity(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int64_t compare_axis(const Mario::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Velocity &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }
    int64_t compare_axis(const Feature_Mode &) const {return -1;}
    int64_t compare_axis(const Feature_Flag &) const {return -1;}
    int64_t compare_axis(const Feature_Numeric &) const {return -1;}
    int64_t compare_axis(const Feature_Button &) const {return -1;}

    void print(ostream &os) const {
      switch(axis.first) {
        case X_DOT: os << "x-dot"; break;
        case Y_DOT: os << "y-dot"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Feature_Mode : public Carli::Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {MODE = Feature_Velocity::Y_DOT + 1};

    Feature_Mode(const Mode &mode)
     : Feature_Enumerated<Feature>(Rete::WME_Token_Index(MODE, 2), mode)
    {
    }

    Feature_Mode * clone() const {
      return new Feature_Mode(Mode(value));
    }

    int64_t compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Velocity &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 0;}
    int64_t compare_axis(const Feature_Flag &) const {return -1;}
    int64_t compare_axis(const Feature_Numeric &) const {return -1;}
    int64_t compare_axis(const Feature_Button &) const {return -1;}
    
    void print(ostream &os) const {
      os << "mode(" << value << ')';
    }
  };
  
  class Feature_Flag : public Carli::Feature_Enumerated<Feature> {
  public:
    enum Axis : size_t {
      START = Feature_Mode::MODE + 1,
      ON_GROUND = START + 0,
      MAY_JUMP = START + 1,
      IS_CARRYING = START + 2,
      IS_HIGH_JUMPING = START + 3,
      IS_ABOVE_PIT = START + 4,
      IS_IN_PIT = START + 5,
      PIT_RIGHT = START + 6,
      OBSTACLE_RIGHT = START + 7,
      END = OBSTACLE_RIGHT + 1
    };

    Feature_Flag(const Axis &axis_, const bool &flag)
     : Feature_Enumerated<Feature>(Rete::WME_Token_Index(axis_, 2), flag)
    {
    }

    Feature_Flag * clone() const {
      return new Feature_Flag(Axis(axis.first), value != 0);
    }

    int64_t compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Velocity &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 1;}
    int64_t compare_axis(const Feature_Flag &rhs) const {
      return axis.first - rhs.axis.first;
    }
    int64_t compare_axis(const Feature_Numeric &) const {return -1;}
    int64_t compare_axis(const Feature_Button &) const {return -1;}
    
    void print(ostream &os) const {
      switch(axis.first) {
      case ON_GROUND       : os << "on-ground";       break;
      case MAY_JUMP        : os << "may-jump";        break;
      case IS_CARRYING     : os << "is-carrying";     break;
      case IS_HIGH_JUMPING : os << "is-high-jumping"; break;
      case IS_ABOVE_PIT    : os << "is-above-pit";    break;
      case IS_IN_PIT       : os << "is-in-pit";       break;
      case PIT_RIGHT       : os << "pit-right";       break;
      case OBSTACLE_RIGHT  : os << "obstacle-right";  break;
      default: abort();
      }

      os << '(' << value << ')';
    }
  };
  
  class Feature_Numeric : public Carli::Feature_Ranged<Feature> {
  public:
    enum Axis {
      START = Feature_Flag::END,
      RIGHT_PIT_DIST = START + 0,
      RIGHT_PIT_WIDTH = START + 1,
      RIGHT_JUMP_DIST = START + 2,
      RIGHT_JUMP_HEIGHT = START + 3,
      END = RIGHT_JUMP_HEIGHT + 1
    };

    Feature_Numeric(const Axis &axis_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_)
     : Feature_Ranged(Rete::WME_Token_Index(axis_, 2), bound_lower_, bound_upper_, depth_, upper_, true)
    {
    }

    Feature_Numeric * clone() const {
      return new Feature_Numeric(Axis(axis.first), bound_lower, bound_upper, depth, upper);
    }

    int64_t compare_axis(const Mario::Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Velocity &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 1;}
    int64_t compare_axis(const Feature_Flag &) const {return 1;}
    int64_t compare_axis(const Feature_Numeric &rhs) const {
      return Feature_Ranged_Data::compare_axis(rhs);
    }
    int64_t compare_axis(const Feature_Button &) const {return -1;}

    void print(ostream &os) const {
      switch(axis.first) {
        case RIGHT_PIT_DIST: os << "right-pit-dist"; break;
        case RIGHT_PIT_WIDTH: os << "right-pit-width"; break;
        case RIGHT_JUMP_DIST: os << "right-jump-dist"; break;
        case RIGHT_JUMP_HEIGHT: os << "right-jump-height"; break;
        default: abort();
      }

      os << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
    }
  };

  class Feature_Button : public Carli::Feature_Enumerated<Feature> {
  public:
    enum Axis {
      IN_JOIN = Feature_Numeric::END,
      IN_START = IN_JOIN + 1,
      IN_DPAD = IN_START + 0,
      IN_JUMP = IN_START + 1,
      IN_SPEED = IN_START + 2,
      IN_END = IN_SPEED + 1,

      OUT_JOIN = IN_END,
      OUT_START = OUT_JOIN + 1,
      OUT_DPAD = OUT_START + 0,
      OUT_JUMP = OUT_START + 1,
      OUT_SPEED = OUT_START + 2,
      END = OUT_SPEED + 1
    };

    Feature_Button(const Axis &axis_, const int64_t &flag)
     : Feature_Enumerated<Feature>(Rete::WME_Token_Index(axis_, 2), flag)
    {
      static_assert(ACTION_INDEX == OUT_JOIN, "ACTION_INDEX is misspecified in Mario::Feature");
    }

    Feature_Button * clone() const {
      return new Feature_Button(Axis(axis.first), value);
    }

    int64_t compare_axis(const Feature &rhs) const {
      return -rhs.compare_axis(*this);
    }
    int64_t compare_axis(const Feature_Position &) const {return 1;}
    int64_t compare_axis(const Feature_Velocity &) const {return 1;}
    int64_t compare_axis(const Feature_Mode &) const {return 1;}
    int64_t compare_axis(const Feature_Flag &) const {return 1;}
    int64_t compare_axis(const Feature_Numeric &) const {return 1;}
    int64_t compare_axis(const Feature_Button &rhs) const {
      return axis.first - rhs.axis.first;
    }

    void print(ostream &os) const {
      switch(axis.first) {
      case IN_DPAD:
      case OUT_DPAD:
        os << "dpad-" << (axis.first == IN_DPAD ? "in" : "out") << '(' << (value == BUTTON_LEFT ? "left" : value == BUTTON_RIGHT ? "right" : value == BUTTON_DOWN ? "down" : "released") << ')';
        break;

      case IN_JUMP:
      case OUT_JUMP:
        os << "jump-" << (axis.first == IN_JUMP ? "in" : "out") << '(' << value << ')';
        break;

      case IN_SPEED:
      case OUT_SPEED:
        os << "speed-" << (axis.first == IN_SPEED ? "in" : "out") << '(' << value << ')';
        break;

      default:
        abort();
      }
    }
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
      memset(&action, 0, sizeof(action));
      const int64_t dpad = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Feature_Button::OUT_DPAD, 2)]).value;
      if(dpad != BUTTON_NONE)
        action.at(size_t(dpad)) = true;
      action[BUTTON_JUMP] = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Feature_Button::OUT_JUMP, 2)]).value != 0;
      action[BUTTON_SPEED] = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[Rete::WME_Token_Index(Feature_Button::OUT_SPEED, 2)]).value != 0;
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
    Agent(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current);
    ~Agent();

    void act_part_1(Action &action);
    void act_part_2(const std::shared_ptr<State> &prev, const std::shared_ptr<State> &current, const bool &terminal);

  private:
    template<typename SUBFEATURE, typename AXIS>
    void generate_rete_continuous(const Carli::Node_Unsplit_Ptr &node_unsplit,
                                  const AXIS &axis,
                                  const double &lower_bound,
                                  const double &upper_bound);
    void generate_rete();

    void generate_features();

    void update();
    
    const std::shared_ptr<State> &m_current_state;
    const std::shared_ptr<State> &m_prev_state;

    const Rete::Symbol_Variable_Ptr_C m_first_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::First));
    const Rete::Symbol_Variable_Ptr_C m_third_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::Third));

    const Rete::Symbol_Constant_String_Ptr_C m_button_presses_in_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("button-presses-in"));
    const Rete::Symbol_Constant_String_Ptr_C m_button_presses_out_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("button-presses-out"));
    const Rete::Symbol_Constant_String_Ptr_C m_enemy_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("enemy"));
    const Rete::Symbol_Constant_String_Ptr_C m_x_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("x"));
    const Rete::Symbol_Constant_String_Ptr_C m_y_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("y"));
    const Rete::Symbol_Constant_String_Ptr_C m_x_dot_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("x-dot"));
    const Rete::Symbol_Constant_String_Ptr_C m_y_dot_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("y-dot"));
    const Rete::Symbol_Constant_String_Ptr_C m_mode_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("mode"));
    const Rete::Symbol_Constant_String_Ptr_C m_on_ground_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("on-ground"));
    const Rete::Symbol_Constant_String_Ptr_C m_may_jump_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("may-jump"));
    const Rete::Symbol_Constant_String_Ptr_C m_is_carrying_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("is-carrying"));
    const Rete::Symbol_Constant_String_Ptr_C m_is_high_jumping_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("is-high-jumping"));
    const Rete::Symbol_Constant_String_Ptr_C m_is_above_pit_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("is-above-pit"));
    const Rete::Symbol_Constant_String_Ptr_C m_is_in_pit_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("is-in-pit"));
    const Rete::Symbol_Constant_String_Ptr_C m_pit_right_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("pit-right"));
    const Rete::Symbol_Constant_String_Ptr_C m_obstacle_right_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("obstacle-right"));
    const Rete::Symbol_Constant_String_Ptr_C m_right_pit_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("right-pit-dist"));
    const Rete::Symbol_Constant_String_Ptr_C m_right_pit_width_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("right-pit-width"));
    const Rete::Symbol_Constant_String_Ptr_C m_right_jump_dist_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("right-jump-dist"));
    const Rete::Symbol_Constant_String_Ptr_C m_right_jump_height_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("right-jump-height"));
    const Rete::Symbol_Constant_String_Ptr_C m_dpad_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("dpad"));
    const Rete::Symbol_Constant_String_Ptr_C m_jump_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("jump"));
    const Rete::Symbol_Constant_String_Ptr_C m_speed_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("speed"));
    const Rete::Symbol_Constant_String_Ptr_C m_type_attr = Rete::Symbol_Constant_String_Ptr_C(new Rete::Symbol_Constant_String("type"));
    const Rete::Symbol_Constant_Int_Ptr_C m_true_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(1));
    const Rete::Symbol_Constant_Int_Ptr_C m_false_value = Rete::Symbol_Constant_Int_Ptr_C(new Rete::Symbol_Constant_Int(0));

    const Rete::Symbol_Identifier_Ptr_C m_button_presses_in_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("I1"));

    std::list<Rete::WME_Ptr_C> m_wmes_prev;
  };

}

#endif
