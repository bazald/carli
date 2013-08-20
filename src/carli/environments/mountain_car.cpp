#include "mountain_car.h"

namespace Mountain_Car {

  Environment::Environment() {
    Environment::init_impl();
  }

  void Environment::init_impl() {
    if(m_random_start) {
      m_x = m_random_init.frand_lt() * (m_goal_position - m_min_position) + m_min_position;
      m_x_dot = m_random_init.frand_lt() * (2 * m_max_velocity) - m_max_velocity;
    }
    else {
      m_x = -0.5;
      m_x_dot = 0.0;
    }
  }

  void Environment::alter_impl() {
    if(get_scenario() < 400)
      return;

    else if(get_scenario() < 410) {
      m_cart_force = 0.0009;
    }
    else if(get_scenario() < 420) {
      m_cart_force = 0.0008;
    }
    else if(get_scenario() < 500) {
      m_cart_force = 0.0006;
    }
  }

  Environment::reward_type Environment::transition_impl(const Action &action) {
    const int a = int(debuggable_cast<const Move &>(action).direction);

    assert(0 <= a && a <= 2);

    m_x_dot += (a - 1) * m_cart_force + cos(3 * m_x) * -m_grav_force;
    m_x_dot = std::max(-m_max_velocity, std::min(m_max_velocity, m_x_dot));

    m_x += m_x_dot;
    m_x = std::max(m_min_position, std::min(m_max_position, m_x));

    if(m_x == m_min_position && m_x_dot < 0)
      m_x_dot = 0;

    return m_reward_negative ? -1 : success() ? 1 : 0;
  }

  void Environment::print_impl(ostream &os) const {
    os << "Mountain Car:" << endl;
    os << " (" << m_x << ", " << m_x_dot << ')' << endl;
  }

  Agent::Agent(const shared_ptr< ::Environment> &env)
   : ::Agent(env)
  {
    auto s_id = std::make_shared<Rete::Symbol_Identifier>("S1");
    auto x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    auto x_dot_attr = std::make_shared<Rete::Symbol_Constant_String>("x-dot");

    Rete::WME_Bindings state_bindings;
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));
    auto x = make_filter(Rete::WME(m_first_var, x_attr, m_third_var));
    auto x_dot = make_filter(Rete::WME(m_first_var, x_dot_attr, m_third_var));
    auto xxdot = make_join(state_bindings, x, x_dot);
    const bool cmac = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value();
    if(cmac)
      generate_cmac(xxdot);
    else
      generate_rete(xxdot);

    m_x_wme = std::make_shared<Rete::WME>(s_id, x_attr, m_x_value);
    m_x_dot_wme = std::make_shared<Rete::WME>(s_id, x_dot_attr, m_x_dot_value);
    insert_wme(m_x_wme);
    insert_wme(m_x_dot_wme);
  }

  Agent::~Agent() {
    destroy();
    for(auto &action : m_action)
      action.delete_and_zero();
  }

  void Agent::print_policy(ostream &os, const size_t &granularity) {
    auto env = dynamic_pointer_cast<Environment>(get_env());
    const auto x_bak = env->get_x();
    const auto x_dot_bak = env->get_x_dot();

    for(size_t x_dot = granularity; x_dot != 0lu; --x_dot) {
      env->set_x_dot(((x_dot - 0.5) / granularity) * 0.14 - 0.07);
      for(size_t x = 0lu; x != granularity; ++x) {
        env->set_x(((x + 0.5) / granularity) * 1.8 - 1.2);
        generate_features();
        auto action = choose_greedy();
        switch(static_cast<const Move &>(*action).direction) {
          case Move::LEFT:  os << '-'; break;
          case Move::IDLE:  os << '0'; break;
          case Move::RIGHT: os << '+'; break;
          default: abort();
        }
      }
      os << endl;
    }

    env->set_x(x_bak);
    env->set_x_dot(x_dot_bak);
    generate_features();
  }

  void Agent::generate_cmac(const Rete::Rete_Node_Ptr &parent) {
    const size_t cmac_tilings = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["cmac-tilings"]).get_value();
    const size_t cmac_resolution = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["cmac-resolution"]).get_value();
    const size_t cmac_offset = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["cmac-offset"]).get_value();

    assert(cmac_offset < cmac_tilings);
    const double x_size = (m_max_x - m_min_x) / cmac_resolution;
    const double xdot_size = (m_max_x_dot - m_min_x_dot) / cmac_resolution;

    for(size_t tiling = -cmac_offset, tend = tiling + cmac_tilings; tiling != tend; ++tiling) {
      const double x_offset = x_size * tiling;
      const double xdot_offset = xdot_size * tiling;

      for(size_t i = 0; i != cmac_resolution; ++i) {
        const double left = m_min_x + (i - x_offset) * x_size;
        const double right = m_min_x + (i + 1 - x_offset) * x_size;
        auto xgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Feature::X, 2), std::make_shared<Rete::Symbol_Constant_Float>(left), parent);
        auto xlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Feature::X, 2), std::make_shared<Rete::Symbol_Constant_Float>(right), xgte);
        for(size_t j = 0; j != cmac_resolution; ++j) {
          const double top = m_min_x_dot + (j - xdot_offset) * xdot_size;
          const double bottom = m_min_x_dot + (j + 1 - xdot_offset) * xdot_size;
          auto xdotgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Feature::X_DOT, 2), std::make_shared<Rete::Symbol_Constant_Float>(top), xlt);
          auto xdotlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Feature::X_DOT, 2), std::make_shared<Rete::Symbol_Constant_Float>(bottom), xdotgte);
          auto q_value = std::make_shared<Q_Value>(0.0, Q_Value::Type::UNSPLIT, 1);
          for(const auto &action : m_action) {
            RL::Lines lines;
            lines.push_back(RL::Line(std::make_pair(left, top), std::make_pair(left, bottom)));
            lines.push_back(RL::Line(std::make_pair(left, bottom), std::make_pair(right, bottom)));
            lines.push_back(RL::Line(std::make_pair(right, bottom), std::make_pair(right, top)));
            lines.push_back(RL::Line(std::make_pair(right, top), std::make_pair(left, top)));
            auto rl = std::make_shared<RL>(*this, 1,
                                           RL::Range(std::make_pair(left, top), std::make_pair(right, bottom)),
                                           lines);
            rl->q_value = new Q_Value(0.0, Q_Value::Type::UNSPLIT, rl->depth);
            ++this->m_q_value_count;
            make_action_retraction([this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
              this->m_next_q_values[action].push_back(rl->q_value);
            }, [this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
              this->purge_q_value_next(action, rl->q_value);
            }, xdotlt);
          }
        }
      }
    }
  }

  void Agent::generate_rete(const Rete::Rete_Node_Ptr &parent) {
    const double m_half_x = (m_min_x + m_max_x) / 2.0;
    const double m_half_x_dot = (m_min_x_dot + m_max_x_dot) / 2.0;

    for(const auto &action : m_action) {
      auto rl = std::make_shared<RL>(*this, 1,
                                     RL::Range(std::make_pair(m_min_x, m_max_x), std::make_pair(m_min_x_dot, m_max_x_dot)),
                                     RL::Lines());
      rl->q_value = new Q_Value(0.0, Q_Value::Type::UNSPLIT, rl->depth);
      ++this->m_q_value_count;
      rl->fringe_values = new RL::Fringe_Values;
      rl->action = make_action_retraction([this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
        if(!this->specialize(action, rl))
          this->m_next_q_values[action].push_back(rl->q_value);
      }, [this,action,rl](const Rete::Rete_Action &, const Rete::WME_Token &) {
        this->purge_q_value_next(action, rl->q_value);
      }, parent);

      {
        RL::Lines lines;
        lines.push_back(RL::Line(std::make_pair(m_half_x, m_min_x_dot), std::make_pair(m_half_x, m_max_x_dot)));
        auto rlf = std::make_shared<RL>(*this, 2,
                                        RL::Range(std::make_pair(m_min_x, m_min_x_dot), std::make_pair(m_half_x, m_max_x_dot)),
                                        lines);
        rlf->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE, rlf->depth);
        rlf->feature = new Feature(Feature::X, m_min_x, m_half_x, 2, false);
        auto predicate = make_predicate_vc(rlf->feature->predicate(), Rete::WME_Token_Index(Feature::X, 2), rlf->feature->symbol_constant(), rl->action.lock()->parent());
        rlf->action = make_action_retraction([this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(rlf->q_value);
        }, [this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, rlf->q_value);
        }, predicate);
        rl->fringe_values->push_back(rlf);
      }

      {
        auto rlf = std::make_shared<RL>(*this, 2,
                                        RL::Range(std::make_pair(m_half_x, m_min_x_dot), std::make_pair(m_max_x, m_max_x_dot)),
                                        RL::Lines());
        rlf->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE, rlf->depth);
        rlf->feature = new Feature(Feature::X, m_half_x, m_max_x, 2, true);
        auto predicate = make_predicate_vc(rlf->feature->predicate(), Rete::WME_Token_Index(Feature::X, 2), rlf->feature->symbol_constant(), rl->action.lock()->parent());
        rlf->action = make_action_retraction([this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(rlf->q_value);
        }, [this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, rlf->q_value);
        }, predicate);
        rl->fringe_values->push_back(rlf);
      }

      {
        RL::Lines lines;
        lines.push_back(RL::Line(std::make_pair(m_min_x, m_half_x_dot), std::make_pair(m_max_x, m_half_x_dot)));
        auto rlf = std::make_shared<RL>(*this, 2,
                                        RL::Range(std::make_pair(m_min_x, m_min_x_dot), std::make_pair(m_max_x, m_half_x_dot)),
                                        lines);
        rlf->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE, rlf->depth);
        rlf->feature = new Feature(Feature::X_DOT, m_min_x_dot, m_half_x_dot, 2, false);
        auto predicate = make_predicate_vc(rlf->feature->predicate(), Rete::WME_Token_Index(Feature::X_DOT, 2), rlf->feature->symbol_constant(), rl->action.lock()->parent());
        rlf->action = make_action_retraction([this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(rlf->q_value);
        }, [this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, rlf->q_value);
        }, predicate);
        rl->fringe_values->push_back(rlf);
      }

      {
        auto rlf = std::make_shared<RL>(*this, 2,
                                        RL::Range(std::make_pair(m_min_x, m_half_x_dot), std::make_pair(m_max_x, m_max_x_dot)),
                                        RL::Lines());
        rlf->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE, rlf->depth);
        rlf->feature = new Feature(Feature::X_DOT, m_half_x_dot, m_max_x_dot, 2, true);
        auto predicate = make_predicate_vc(rlf->feature->predicate(), Rete::WME_Token_Index(Feature::X_DOT, 2), rlf->feature->symbol_constant(), rl->action.lock()->parent());
        rlf->action = make_action_retraction([this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(rlf->q_value);
        }, [this,action,rlf](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, rlf->q_value);
        }, predicate);
        rl->fringe_values->push_back(rlf);
      }
    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    clear_wmes();

    m_x_value->value = env->get_x();
    m_x_dot_value->value = env->get_x_dot();
    insert_wme(m_x_wme);
    insert_wme(m_x_dot_wme);
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->success() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

}
