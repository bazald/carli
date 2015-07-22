#include "puddle_world.h"

#include "carli/parser/rete_parser.h"

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
   : Carli::Agent(env, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);}),
   m_action({{std::shared_ptr<const Carli::Action>(new Move(NORTH)),
              std::shared_ptr<const Carli::Action>(new Move(SOUTH)),
              std::shared_ptr<const Carli::Action>(new Move(EAST)),
              std::shared_ptr<const Carli::Action>(new Move(WEST))}})
  {
    const Rete::Symbol_Variable_Ptr_C m_first_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::First));
    const Rete::Symbol_Variable_Ptr_C m_third_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::Third));

    const auto move_attr = std::make_shared<Rete::Symbol_Constant_String>("move");
    const auto x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    const auto y_attr = std::make_shared<Rete::Symbol_Constant_String>("y");
    const std::array<Rete::Symbol_Constant_Int_Ptr_C, 4> move_values = {{std::make_shared<Rete::Symbol_Constant_Int>(NORTH),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(SOUTH),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(EAST),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(WEST)}};

    if(dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value()) {
      Rete::WME_Bindings state_bindings;
      state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0, 0), Rete::WME_Token_Index(0, 0, 0)));
      const auto move = make_filter(Rete::WME(m_first_var, move_attr, m_third_var));
      const auto x = make_filter(Rete::WME(m_first_var, x_attr, m_third_var));
      const auto y = make_filter(Rete::WME(m_first_var, y_attr, m_third_var));
      for(const auto &move_value : move_values) {
        const auto move_pred = make_predicate_vc(Rete::Rete_Predicate::EQ, Rete::WME_Token_Index(0, 0, 2), move_value, move);
        const auto move_x = make_join(state_bindings, move_pred, x);
        const auto x_y = make_join(state_bindings, move_x, y);
        generate_cmac(x_y);
      }
    }
    else {
      std::string rules_in = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
      if(rules_in == "default")
        rules_in = "rules/puddle-world.carli";
      if(rete_parse_file(*this, rules_in))
        abort();
    }

    m_x_wme = std::make_shared<Rete::WME>(m_s_id, x_attr, m_x_value);
    m_y_wme = std::make_shared<Rete::WME>(m_s_id, y_attr, m_y_value);
    insert_wme(m_x_wme);
    insert_wme(m_y_wme);
    for(const auto &move_value : move_values)
      insert_wme(std::make_shared<Rete::WME>(m_s_id, move_attr, move_value));
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
        auto action = choose_greedy(nullptr);
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
    const int64_t cmac_tilings = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-tilings"]).get_value();
    const int64_t cmac_resolution = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-resolution"]).get_value();
    const int64_t cmac_offset = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["cmac-offset"]).get_value();

    assert(cmac_offset < cmac_tilings);
    const double xy_size = 1.0 / cmac_resolution;

    const auto variables = std::make_shared<Rete::Variable_Indices>();
    variables->insert(std::make_pair("move", Rete::WME_Token_Index(0, 0, 2)));
    variables->insert(std::make_pair("x", Rete::WME_Token_Index(1, 1, 2)));
    variables->insert(std::make_pair("y", Rete::WME_Token_Index(2, 2, 2)));

    for(int64_t tiling = -cmac_offset, tend = tiling + cmac_tilings; tiling != tend; ++tiling) {
      const double xy_offset = xy_size * tiling;

      for(int64_t i = 0; i != cmac_resolution; ++i) {
        const double left = (i - xy_offset) * xy_size;
        const double right = (i + 1 - xy_offset) * xy_size;
        auto xgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(1, 1, 2), std::make_shared<Rete::Symbol_Constant_Float>(left), parent);
        auto xlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(1, 1, 2), std::make_shared<Rete::Symbol_Constant_Float>(right), xgte);
        for(int64_t j = 0; j != cmac_resolution; ++j) {
          const double top = (j - xy_offset) * xy_size;
          const double bottom = (j + 1 - xy_offset) * xy_size;
          auto ygte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(2, 2, 2), std::make_shared<Rete::Symbol_Constant_Float>(top), xlt);
          auto ylt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(2, 2, 2), std::make_shared<Rete::Symbol_Constant_Float>(bottom), ygte);

          ///// This does redundant work for actions after the first.
          //for(const auto &action : m_action) {
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, top), std::make_pair(left, bottom)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, bottom), std::make_pair(right, bottom)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, bottom), std::make_pair(right, top)));
          //  m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, top), std::make_pair(left, top)));
          //}

          auto action = make_standard_action(ylt, next_rule_name("puddle-world*rl-action*cmac-"), false, variables);
          action->data = std::make_shared<Node_Split>(*this, Rete::Rete_Action_Ptr(), action, new Q_Value(Q_Value::Token(), Q_Value::Type::SPLIT, 1, nullptr, 0));
        }
      }
    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());
    const auto pos = env->get_position();

    Rete::Agenda::Locker locker(agenda);
    CPU_Accumulator cpu_accumulator(*this);

    const bool flush_wmes = get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes");

    if(flush_wmes || m_x_value->value != pos.first) {
      remove_wme(m_x_wme);
      m_x_wme->symbols[2] = m_x_value = std::make_shared<Rete::Symbol_Constant_Float>(pos.first);
      insert_wme(m_x_wme);
    }
    if(flush_wmes || m_y_value->value != pos.second) {
      remove_wme(m_y_wme);
      m_y_wme->symbols[2] = m_y_value = std::make_shared<Rete::Symbol_Constant_Float>(pos.second);
      insert_wme(m_y_wme);
    }
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->goal_reached() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

}
