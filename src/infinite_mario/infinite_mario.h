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

  bool tile_can_jump_into(const Tile &tile);
  bool tile_can_jump_through(const Tile &tile);
  bool tile_can_land_on(const Tile &tile);
  bool tile_can_pass_through(const Tile &tile);

  bool object_dangerous(const Object &object);
  bool object_flies(const Object &object);
  bool object_powerup(const Object &object);
  bool object_killable_by_fireball(const Object &object);
  bool object_killable_by_jump(const Object &object);

  class Button_Presses : public Carli::Action {
  public:
    Button_Presses() {
      memset(&action, 0, sizeof(action));
    }

    Button_Presses(const Mario::Action &action_)
     : action(action_)
    {
    }

    Button_Presses(const Rete::Variable_Indices &variables, const Rete::WME_Token &token) {
      memset(&action, 0, sizeof(action));
      const int64_t dpad = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("dpad")->second]).value;
      if(dpad != BUTTON_NONE)
        action.at(size_t(dpad)) = true;
      action[BUTTON_JUMP] = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("jump")->second]).value != 0;
      action[BUTTON_SPEED] = debuggable_cast<const Rete::Symbol_Constant_Int &>(*token[variables.find("speed")->second]).value != 0;
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
    void generate_rete();

    void generate_features();

    void update();

    const std::shared_ptr<State> &m_current_state;
    const std::shared_ptr<State> &m_prev_state;

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
    double m_rho = double();
  };

}

#endif
