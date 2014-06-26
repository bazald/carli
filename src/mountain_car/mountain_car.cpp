#include "mountain_car.h"

#include "carli/experiment.h"

namespace Mountain_Car {

  using Carli::Metastate;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

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

  Environment::reward_type Environment::transition_impl(const Carli::Action &action) {
    const int64_t a = int64_t(debuggable_cast<const Acceleration &>(action).direction);

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

  Agent::Agent(const shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Acceleration>(token);})
  {
    auto s_id = std::make_shared<Rete::Symbol_Identifier>("S1");
    auto x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    auto x_dot_attr = std::make_shared<Rete::Symbol_Constant_String>("x-dot");
    auto acceleration_attr = std::make_shared<Rete::Symbol_Constant_String>("acceleration");
    const std::array<Rete::Symbol_Constant_Int_Ptr_C, 3> acceleration_values = {{std::make_shared<Rete::Symbol_Constant_Int>(LEFT),
                                                                                 std::make_shared<Rete::Symbol_Constant_Int>(IDLE),
                                                                                 std::make_shared<Rete::Symbol_Constant_Int>(RIGHT)}};

    Rete::WME_Bindings state_bindings;
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));
    auto x = make_filter(Rete::WME(m_first_var, x_attr, m_third_var));
    auto x_dot = make_filter(Rete::WME(m_first_var, x_dot_attr, m_third_var));
    auto acceleration = make_filter(Rete::WME(m_first_var, acceleration_attr, m_third_var));
    auto xxdot = make_join(state_bindings, x, x_dot);
    state_bindings.clear();
    const bool cmac = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value();
    for(const auto &acceleration_value : acceleration_values) {
      auto a = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(0, 2), acceleration_value, acceleration);
      auto xxdota = make_join(state_bindings, xxdot, a);
      if(cmac)
        generate_cmac(xxdota);
      else
        generate_rete(xxdota);
    }

    m_x_wme = std::make_shared<Rete::WME>(s_id, x_attr, m_x_value);
    m_x_dot_wme = std::make_shared<Rete::WME>(s_id, x_dot_attr, m_x_dot_value);
    insert_wme(m_x_wme);
    insert_wme(m_x_dot_wme);
    for(const auto &acceleration_value : acceleration_values)
      insert_wme(std::make_shared<Rete::WME>(s_id, acceleration_attr, acceleration_value));
    insert_wme(m_wme_blink);
  }

  Agent::~Agent() {
    destroy();
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
        switch(static_cast<const Acceleration &>(*action).direction) {
          case LEFT:  os << '-'; break;
          case IDLE:  os << '0'; break;
          case RIGHT: os << '+'; break;
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
    const int64_t cmac_tilings = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-tilings"]).get_value();
    const int64_t cmac_resolution = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-resolution"]).get_value();
    const int64_t cmac_offset = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-offset"]).get_value();

    assert(cmac_offset < cmac_tilings);
    const double x_size = (m_max_x - m_min_x) / cmac_resolution;
    const double xdot_size = (m_max_x_dot - m_min_x_dot) / cmac_resolution;

    for(int64_t tiling = -cmac_offset, tend = tiling + cmac_tilings; tiling != tend; ++tiling) {
      const double x_offset = x_size * tiling;
      const double xdot_offset = xdot_size * tiling;

      for(int64_t i = 0; i != cmac_resolution; ++i) {
        const double left = m_min_x + (i - x_offset) * x_size;
        const double right = m_min_x + (i + 1 - x_offset) * x_size;
        auto xgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Position::index, 2), std::make_shared<Rete::Symbol_Constant_Float>(left), parent);
        auto xlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Position::index, 2), std::make_shared<Rete::Symbol_Constant_Float>(right), xgte);
        for(int64_t j = 0; j != cmac_resolution; ++j) {
          const double top = m_min_x_dot + (j - xdot_offset) * xdot_size;
          const double bottom = m_min_x_dot + (j + 1 - xdot_offset) * xdot_size;
          auto xdotgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Velocity::index, 2), std::make_shared<Rete::Symbol_Constant_Float>(top), xlt);
          auto xdotlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Velocity::index, 2), std::make_shared<Rete::Symbol_Constant_Float>(bottom), xdotgte);

          ///// This does redundant work for actions after the first.
          //for(const auto &action : m_action) {
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, top), std::make_pair(left, bottom)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, bottom), std::make_pair(right, bottom)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, bottom), std::make_pair(right, top)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, top), std::make_pair(left, top)));
          //}

          auto action = make_standard_action(xdotlt, next_rule_name("mountain-car*rl-action*cmac-"), false);
          action->data = std::make_shared<Node_Split>(*this, action, new Q_Value(0.0, Q_Value::Type::SPLIT, 1, nullptr), true);
        }
      }
    }
  }

  void Agent::generate_rete(const Rete::Rete_Node_Ptr &parent) {
    auto filter_blink = make_filter(*m_wme_blink);

    const double m_half_x = (m_min_x + m_max_x) / 2.0;
    const double m_half_x_dot = (m_min_x_dot + m_max_x_dot) / 2.0;

    Carli::Node_Unsplit_Ptr root_action_data;
    {
      auto join_blink = make_existential_join(Rete::WME_Bindings(), false, parent, filter_blink);

      auto root_action = make_standard_action(join_blink, next_rule_name("mountain-car*rl-action*u"), false);
      root_action_data = std::make_shared<Node_Unsplit>(*this, root_action, 1, nullptr);
      root_action->data = root_action_data;
    }

    {
      //Node_Ranged::Lines lines;
      //lines.push_back(Node_Ranged::Line(std::make_pair(m_half_x, m_min_x_dot), std::make_pair(m_half_x, m_max_x_dot)));
      auto feature = new Position(m_min_x, m_half_x, 2, false);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::index, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("mountain-car*rl-action*f"), false, root_action_data, feature); //, Node_Ranged::Range(std::make_pair(m_min_x, m_min_x_dot), std::make_pair(m_half_x, m_max_x_dot)), lines);
    }

    {
      auto feature = new Position(m_half_x, m_max_x, 2, true);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::index, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("mountain-car*rl-action*f"), false, root_action_data, feature); //, Node_Ranged::Range(std::make_pair(m_half_x, m_min_x_dot), std::make_pair(m_max_x, m_max_x_dot)), Node_Ranged::Lines());
    }

    {
      //Node_Ranged::Lines lines;
      //lines.push_back(Node_Ranged::Line(std::make_pair(m_min_x, m_half_x_dot), std::make_pair(m_max_x, m_half_x_dot)));
      auto feature = new Velocity(m_min_x_dot, m_half_x_dot, 2, false);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Velocity::index, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("mountain-car*rl-action*f"), false, root_action_data, feature); //, Node_Ranged::Range(std::make_pair(m_min_x, m_min_x_dot), std::make_pair(m_max_x, m_half_x_dot)), lines);
    }

    {
      auto feature = new Velocity(m_half_x_dot, m_max_x_dot, 2, true);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Velocity::index, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("mountain-car*rl-action*f"), false, root_action_data, feature); //, Node_Ranged::Range(std::make_pair(m_min_x, m_half_x_dot), std::make_pair(m_max_x, m_max_x_dot)), Node_Ranged::Lines());
    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    remove_wme(m_wme_blink);
    if(m_x_value->value != env->get_x()) {
      remove_wme(m_x_wme);
      m_x_wme->symbols[2] = m_x_value = std::make_shared<Rete::Symbol_Constant_Float>(env->get_x());
      insert_wme(m_x_wme);
    }
    if(m_x_dot_value->value != env->get_x_dot()) {
      remove_wme(m_x_dot_wme);
      m_x_dot_wme->symbols[2] = m_x_dot_value = std::make_shared<Rete::Symbol_Constant_Float>(env->get_x_dot());
      insert_wme(m_x_dot_wme);
    }
    insert_wme(m_wme_blink);
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->success() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

}

int main(int argc, char **argv) {
  try {
    Carli::Experiment experiment;

    experiment.take_args(argc, argv);

    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();

    experiment.standard_run([](){return std::make_shared<Mountain_Car::Environment>();},
                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Mountain_Car::Agent>(env);},
                            [&output](const std::shared_ptr<Carli::Agent> &agent){
                              if(output == "experiment") {
                                auto pwa = std::dynamic_pointer_cast<Mountain_Car::Agent>(agent);
                                pwa->print_policy(std::cerr, 32);
                                //if(!dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value())
                                //  pwa->print_value_function_grid(std::cerr);
                              }
                            }
                           );

    return 0;
  }
  catch(std::exception &ex) {
    std::cerr << "Exiting with exception: " << ex.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Exiting with unknown exception." << std::endl;
  }

  return -1;
}
