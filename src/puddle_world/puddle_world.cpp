#include "puddle_world.h"

#include "carli/experiment.h"

namespace Puddle_World {

  using Carli::Metastate;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  Environment::Environment()
   : m_horizontal_puddles({{{0.1, 0.45, 0.75, 0.1}}}),
   m_vertical_puddles({{{0.45, 0.4, 0.8, 0.1}}})
  {
    if(m_random_start) {
      m_init_x = double_pair(0.0, 1.0);
      m_init_y = double_pair(0.0, 1.0);
    }
    else {
      m_init_x = double_pair(0.15, 0.45);
      m_init_y = double_pair(0.15, 0.45);
    }

    Environment::init_impl();
  }

  bool Environment::goal_reached() const {
    if(m_goal_dynamic)
      return m_goal_x.first <= m_position.first  && m_position.first  < m_goal_x.second &&
             m_goal_y.first <= m_position.second && m_position.second < m_goal_y.second;
    else
      return m_position.first + m_position.second > 1.9;
  }

  void Environment::init_impl() {
    do {
      m_position.first = m_init_x.first + m_random_init.frand_lt() * (m_init_x.second - m_init_x.first);
      m_position.second = m_init_y.first + m_random_init.frand_lt() * (m_init_y.second - m_init_y.first);
    } while(goal_reached());

    m_random_motion = Zeni::Random(m_random_init.rand());
  }

  void Environment::alter_impl() {
    /// Too extreme
//       m_horizontal_puddles.at(0)[1] = 0.3;
//       m_vertical_puddles.at(0)[2] = 0.6;
//       m_horizontal_puddles.push_back({{0.6, 0.75, 0.5, 0.1}});
//       m_goal_dynamic = true;
//       m_goal_x = make_pair(0.4, 0.5);
//       m_goal_y = make_pair(0.7, 0.8);

    if(get_scenario() < 100)
      return;

    else if(get_scenario() < 110) {
      m_goal_dynamic = true;
      m_goal_x = make_pair(0.8, 0.85);
      m_goal_y = make_pair(0.9, 1.0);
    }
    else if(get_scenario() < 120) {
      m_goal_dynamic = true;
      m_goal_x = make_pair(0.7, 0.75);
      m_goal_y = make_pair(0.9, 1.0);
    }
    else if(get_scenario() < 130) {
      m_goal_dynamic = true;
      m_goal_x = make_pair(0.5, 0.55);
      m_goal_y = make_pair(0.9, 1.0);
    }
    else if(get_scenario() < 200) {
      m_goal_dynamic = true;
      m_goal_x = make_pair(0.1, 0.15);
      m_goal_y = make_pair(0.9, 1.0);
    }

    else if(get_scenario() < 210) {
      m_horizontal_puddles.at(0)[3] = 0.15;
      m_vertical_puddles.at(0)[3] = 0.15;
    }
    else if(get_scenario() < 220) {
      m_horizontal_puddles.at(0)[3] = 0.2;
      m_vertical_puddles.at(0)[3] = 0.2;
    }
    else if(get_scenario() < 300) {
      m_horizontal_puddles.at(0)[3] = 0.4;
      m_vertical_puddles.at(0)[3] = 0.4;
    }

    else if(get_scenario() < 310) {
      m_noise = 0.02;
    }
    else if(get_scenario() < 320) {
      m_noise = 0.04;
    }
    else if(get_scenario() < 400) {
      m_noise = 0.08;
    }
  }

  Environment::reward_type Environment::transition_impl(const Carli::Action &action) {
    const double shift = m_random_motion.frand_gaussian() * 0.01;
    const double step_size = shift + 0.05;

    switch(debuggable_cast<const Move &>(action).direction) {
      case NORTH: m_position.second += step_size; break;
      case SOUTH: m_position.second -= step_size; break;
      case EAST:  m_position.first  += step_size; break;
      case WEST:  m_position.first  -= step_size; break;
      default: abort();
    }

    if(m_position.first < 0.0)
      m_position.first = 0.0;
    else if(m_position.first >= 1.0)
      m_position.first = 1.0 - DBL_EPSILON;
    if(m_position.second < 0.0)
      m_position.second = 0.0;
    else if(m_position.second >= 1.0)
      m_position.second = 1.0 - DBL_EPSILON;

    ++m_step_count;

    double reward = -1.0;

    for(const Puddle &puddle : m_horizontal_puddles)
      reward -= 400.0 * horizontal_puddle_reward(puddle[0], puddle[1], puddle[2], puddle[3]);
    for(const Puddle &puddle : m_vertical_puddles)
      reward -= 400.0 * vertical_puddle_reward(puddle[0], puddle[1], puddle[2], puddle[3]);

    return reward;
  }

  double Environment::horizontal_puddle_reward(const double &left, const double &right, const double &y, const double &radius) const {
    double dist;

    if(m_position.first < left)
      dist = pythagoras(m_position.first - left, m_position.second - y);
    else if(m_position.first < right)
      dist = std::abs(m_position.second - y);
    else
      dist = pythagoras(m_position.first - right, m_position.second - y);

    return std::max(0.0, radius - dist);
  }

  double Environment::vertical_puddle_reward(const double &x, const double &bottom, const double &top, const double &radius) const {
    double dist;

    if(m_position.second < bottom)
      dist = pythagoras(m_position.first - x, m_position.second - bottom);
    else if(m_position.second < top)
      dist = std::abs(m_position.first - x);
    else
      dist = pythagoras(m_position.first - x, m_position.second - top);

    return std::max(0.0, radius - dist);
  }

  void Environment::print_impl(ostream &os) const {
    os << "Puddle World:" << endl;
    os << " (" << m_position.first << ", " << m_position.second << ')' << endl;
  }

  Agent::Agent(const shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env)
  {
    auto s_id = std::make_shared<Rete::Symbol_Identifier>("S1");
    auto x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    auto y_attr = std::make_shared<Rete::Symbol_Constant_String>("y");
    auto move_attr = std::make_shared<Rete::Symbol_Constant_String>("move");
    const std::array<Rete::Symbol_Constant_Int_Ptr_C, 4> move_values = {{std::make_shared<Rete::Symbol_Constant_Int>(NORTH),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(SOUTH),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(EAST),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(WEST)}};

    Rete::WME_Bindings state_bindings;
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));
    auto x = make_filter(Rete::WME(m_first_var, x_attr, m_third_var));
    auto y = make_filter(Rete::WME(m_first_var, y_attr, m_third_var));
    auto move = make_filter(Rete::WME(m_first_var, move_attr, m_third_var));
    auto xy = make_join(state_bindings, x, y);
    state_bindings.clear();
    const bool cmac = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value();
    for(const auto &move_value : move_values) {
      auto m = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(0, 2), move_value, move);
      auto xym = make_join(state_bindings, xy, m);
      if(cmac)
        generate_cmac(xym);
      else
        generate_rete(xym);
    }

    m_x_wme = std::make_shared<Rete::WME>(s_id, x_attr, m_x_value);
    m_y_wme = std::make_shared<Rete::WME>(s_id, y_attr, m_y_value);
    insert_wme(m_x_wme);
    insert_wme(m_y_wme);
    for(const auto &move_value : move_values)
      insert_wme(std::make_shared<Rete::WME>(s_id, move_attr, move_value));
    insert_wme(m_wme_blink);
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::print_policy(ostream &os, const size_t &granularity) {
    auto env = dynamic_pointer_cast<Environment>(get_env());
    const auto position = env->get_position();

    for(size_t y = granularity; y != 0lu; --y) {
      for(size_t x = 0lu; x != granularity; ++x) {
        env->set_position(Environment::double_pair((x + 0.5) / granularity, (y - 0.5) / granularity));
        generate_features();
        auto action = choose_greedy();
        switch(static_cast<const Move &>(*action).direction) {
          case NORTH: os << 'N'; break;
          case SOUTH: os << 'S'; break;
          case EAST:  os << 'E'; break;
          case WEST:  os << 'W'; break;
          default: abort();
        }
      }
      os << endl;
    }

    env->set_position(position);
    generate_features();
  }

  void Agent::generate_cmac(const Rete::Rete_Node_Ptr &parent) {
    auto get_action = [this](const Rete::WME_Token &token)->Carli::Action_Ptr_C {
      return std::make_shared<Move>(token);
    };

    const int64_t cmac_tilings = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-tilings"]).get_value();
    const int64_t cmac_resolution = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-resolution"]).get_value();
    const int64_t cmac_offset = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-offset"]).get_value();

    assert(cmac_offset < cmac_tilings);
    const double xy_size = 1.0 / cmac_resolution;

    for(int64_t tiling = -cmac_offset, tend = tiling + cmac_tilings; tiling != tend; ++tiling) {
      const double xy_offset = xy_size * tiling;

      for(int64_t i = 0; i != cmac_resolution; ++i) {
        const double left = (i - xy_offset) * xy_size;
        const double right = (i + 1 - xy_offset) * xy_size;
        auto xgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Position::X, 2), std::make_shared<Rete::Symbol_Constant_Float>(left), parent);
        auto xlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Position::X, 2), std::make_shared<Rete::Symbol_Constant_Float>(right), xgte);
        for(int64_t j = 0; j != cmac_resolution; ++j) {
          const double top = (j - xy_offset) * xy_size;
          const double bottom = (j + 1 - xy_offset) * xy_size;
          auto ygte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Position::Y, 2), std::make_shared<Rete::Symbol_Constant_Float>(top), xlt);
          auto ylt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Position::Y, 2), std::make_shared<Rete::Symbol_Constant_Float>(bottom), ygte);

          ///// This does redundant work for actions after the first.
          //for(const auto &action : m_action) {
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, top), std::make_pair(left, bottom)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, bottom), std::make_pair(right, bottom)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, bottom), std::make_pair(right, top)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, top), std::make_pair(left, top)));
          //}

          auto action = make_standard_action(ylt, next_rule_name("puddle-world*rl-action*cmac-"));
          action->data = std::make_shared<Node_Split>(*this, action, get_action, new Q_Value(0.0, Q_Value::Type::SPLIT, 1, nullptr), true);
        }
      }
    }
  }

  void Agent::generate_rete(const Rete::Rete_Node_Ptr &parent) {
    auto get_action = [this](const Rete::WME_Token &token)->Carli::Action_Ptr_C {
      return std::make_shared<Move>(token);
    };

    auto filter_blink = make_filter(*m_wme_blink);

    Carli::Node_Unsplit_Ptr root_action_data;
    {
      auto join_blink = make_existential_join(Rete::WME_Bindings(), parent, filter_blink);

      auto root_action = make_standard_action(join_blink, next_rule_name("puddle-world*rl-action*u"));
      root_action_data = std::make_shared<Node_Unsplit>(*this, root_action, get_action, 1, nullptr);
      root_action->data = root_action_data;
    }

    {
      //Node_Ranged::Lines lines;
      //lines.push_back(Node_Ranged::Line(std::make_pair(0.5, 0.0), std::make_pair(0.5, 1.0)));
      auto feature = new Position(Position::X, 0.0, 0.5, 2, false);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::X, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("puddle-world*rl-action*f"), root_action_data, feature); //, Node_Ranged::Range(std::make_pair(0.0, 0.0), std::make_pair(0.5, 1.0)), lines);
    }

    {
      auto feature = new Position(Position::X, 0.5, 1.0, 2, true);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::X, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("puddle-world*rl-action*f"), root_action_data, feature); //, Node_Ranged::Range(std::make_pair(0.5, 0.0), std::make_pair(1.0, 1.0)), Node_Ranged::Lines());
    }

    {
      //Node_Ranged::Lines lines;
      //lines.push_back(Node_Ranged::Line(std::make_pair(0.0, 0.5), std::make_pair(1.0, 0.5)));
      auto feature = new Position(Position::Y, 0.0, 0.5, 2, false);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::Y, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("puddle-world*rl-action*f"), root_action_data, feature); //, Node_Ranged::Range(std::make_pair(0.0, 0.0), std::make_pair(1.0, 0.5)), lines);
    }

    {
      auto feature = new Position(Position::Y, 0.5, 1.0, 2, true);
      auto predicate = make_predicate_vc(feature->predicate(), Rete::WME_Token_Index(Position::Y, 2), feature->symbol_constant(), root_action_data->rete_action.lock()->parent_left()->parent_left());
      make_standard_fringe(predicate, next_rule_name("puddle-world*rl-action*f"), root_action_data, feature); //, Node_Ranged::Range(std::make_pair(0.0, 0.5), std::make_pair(1.0, 1.0)), Node_Ranged::Lines());
    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());
    const auto pos = env->get_position();

    remove_wme(m_wme_blink);
    if(m_x_value->value != pos.first) {
      remove_wme(m_x_wme);
      m_x_value = std::make_shared<Rete::Symbol_Constant_Float>(pos.first);
      insert_wme(m_x_wme);
    }
    if(m_y_value->value != pos.second) {
      remove_wme(m_y_wme);
      m_y_value = std::make_shared<Rete::Symbol_Constant_Float>(pos.first);
      insert_wme(m_y_wme);
    }
    insert_wme(m_wme_blink);
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->goal_reached() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

}

int main(int argc, char **argv) {
  try {
    Carli::Experiment experiment;

    experiment.take_args(argc, argv);

    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();

    experiment.standard_run([](){return std::make_shared<Puddle_World::Environment>();},
                            [](const std::shared_ptr<Carli::Environment> &env){return std::make_shared<Puddle_World::Agent>(env);},
                            [&output](const std::shared_ptr<Carli::Agent> &agent){
                              if(output == "experiment") {
                                auto pwa = std::dynamic_pointer_cast<Puddle_World::Agent>(agent);
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
