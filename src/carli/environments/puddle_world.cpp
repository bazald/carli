#include "puddle_world.h"

namespace Puddle_World {

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

  Environment::reward_type Environment::transition_impl(const Action &action) {
    const double shift = m_random_motion.frand_gaussian() * 0.01;
    const double step_size = shift + 0.05;

    switch(debuggable_cast<const Move &>(action).direction) {
      case Move::NORTH: m_position.second += step_size; break;
      case Move::SOUTH: m_position.second -= step_size; break;
      case Move::EAST:  m_position.first  += step_size; break;
      case Move::WEST:  m_position.first  -= step_size; break;
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

  Agent::Agent(const shared_ptr< ::Environment> &env)
   : ::Agent(env)
  {
    auto s_id = std::make_shared<Rete::Symbol_Identifier>("S1");
    auto x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    auto y_attr = std::make_shared<Rete::Symbol_Constant_String>("y");

    Rete::WME_Bindings state_bindings;
    state_bindings.insert(Rete::WME_Binding(Rete::WME_Token_Index(0, 0), Rete::WME_Token_Index(0, 0)));
    auto x = make_filter(Rete::WME(m_first_var, x_attr, m_third_var));
    auto y = make_filter(Rete::WME(m_first_var, y_attr, m_third_var));
    auto xy = make_join(state_bindings, x, y);
    const bool cmac = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value();
    if(cmac)
      generate_cmac(xy);
    else
      generate_rete(xy);

    m_x_wme = std::make_shared<Rete::WME>(s_id, x_attr, m_x_value);
    m_y_wme = std::make_shared<Rete::WME>(s_id, y_attr, m_y_value);
    insert_wme(m_x_wme);
    insert_wme(m_y_wme);
  }

  Agent::~Agent() {
    destroy();
    for(auto &action : m_action)
      action.delete_and_zero();
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
          case Move::NORTH: os << 'N'; break;
          case Move::SOUTH: os << 'S'; break;
          case Move::EAST:  os << 'E'; break;
          case Move::WEST:  os << 'W'; break;
          default: abort();
        }
      }
      os << endl;
    }

    env->set_position(position);
    generate_features();
  }

  void Agent::generate_cmac(const Rete::Rete_Node_Ptr &parent) {
    const size_t cmac_tilings = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["cmac-tilings"]).get_value();
    const size_t cmac_resolution = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["cmac-resolution"]).get_value();
    const size_t cmac_offset = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["cmac-offset"]).get_value();

    assert(cmac_offset < cmac_tilings);
    const double xy_size = 1.0 / cmac_resolution;

    for(size_t tiling = -cmac_offset, tend = tiling + cmac_tilings; tiling != tend; ++tiling) {
      const double xy_offset = xy_size * tiling;

      for(size_t i = 0; i != cmac_resolution; ++i) {
        const double left = (i - xy_offset) * xy_size;
        const double right = (i + 1 - xy_offset) * xy_size;
        auto xgte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Feature::X, 2), std::make_shared<Rete::Symbol_Constant_Float>(left), parent);
        auto xlt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Feature::X, 2), std::make_shared<Rete::Symbol_Constant_Float>(right), xgte);
        for(size_t j = 0; j != cmac_resolution; ++j) {
          const double top = (j - xy_offset) * xy_size;
          const double bottom = (j + 1 - xy_offset) * xy_size;
          auto ygte = make_predicate_vc(Rete::Rete_Predicate::GTE, Rete::WME_Token_Index(Feature::Y, 2), std::make_shared<Rete::Symbol_Constant_Float>(top), xlt);
          auto ylt = make_predicate_vc(Rete::Rete_Predicate::LT, Rete::WME_Token_Index(Feature::Y, 2), std::make_shared<Rete::Symbol_Constant_Float>(bottom), ygte);
          for(const auto &action : m_action) {
            m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, top), std::make_pair(left, bottom)));
            m_lines[action].insert(Node_Ranged::Line(std::make_pair(left, bottom), std::make_pair(right, bottom)));
            m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, bottom), std::make_pair(right, top)));
            m_lines[action].insert(Node_Ranged::Line(std::make_pair(right, top), std::make_pair(left, top)));
            auto node_split = std::make_shared<Node_Split>(*this, new Q_Value(0.0, Q_Value::Type::SPLIT, 1));
            make_action_retraction([this,action,node_split](const Rete::Rete_Action &, const Rete::WME_Token &) {
              this->m_next_q_values[action].push_back(node_split->q_value);
            }, [this,action,node_split](const Rete::Rete_Action &, const Rete::WME_Token &) {
              this->purge_q_value_next(action, node_split->q_value);
            }, ylt);
          }
        }
      }
    }
  }

  void Agent::generate_rete(const Rete::Rete_Node_Ptr &parent) {
    for(const auto &action : m_action) {
      auto node_unsplit = std::make_shared<Node_Unsplit>(*this, 1);
      node_unsplit->action = make_action_retraction([this,action,node_unsplit](const Rete::Rete_Action &, const Rete::WME_Token &) {
        if(!this->specialize(action, node_unsplit))
          this->m_next_q_values[action].push_back(node_unsplit->q_value);
      }, [this,action,node_unsplit](const Rete::Rete_Action &, const Rete::WME_Token &) {
        this->purge_q_value_next(action, node_unsplit->q_value);
      }, parent);

      {
        Node_Ranged::Lines lines;
        lines.push_back(Node_Ranged::Line(std::make_pair(0.5, 0.0), std::make_pair(0.5, 1.0)));
        auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                        Node_Ranged::Range(std::make_pair(0.0, 0.0), std::make_pair(0.5, 1.0)),
                                                        lines);
        nfr->feature = new Feature(Feature::X, 0.0, 0.5, 2, false);
        auto predicate = make_predicate_vc(nfr->feature->predicate(), Rete::WME_Token_Index(Feature::X, 2), nfr->feature->symbol_constant(), node_unsplit->action.lock()->parent());
        nfr->action = make_action_retraction([this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(nfr->q_value);
        }, [this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, nfr->q_value);
        }, predicate);
        node_unsplit->fringe_values.push_back(nfr);
      }

      {
        auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                        Node_Ranged::Range(std::make_pair(0.5, 0.0), std::make_pair(1.0, 1.0)),
                                                        Node_Ranged::Lines());
        nfr->feature = new Feature(Feature::X, 0.5, 1.0, 2, true);
        auto predicate = make_predicate_vc(nfr->feature->predicate(), Rete::WME_Token_Index(Feature::X, 2), nfr->feature->symbol_constant(), node_unsplit->action.lock()->parent());
        nfr->action = make_action_retraction([this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(nfr->q_value);
        }, [this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, nfr->q_value);
        }, predicate);
        node_unsplit->fringe_values.push_back(nfr);
      }

      {
        Node_Ranged::Lines lines;
        lines.push_back(Node_Ranged::Line(std::make_pair(0.0, 0.5), std::make_pair(1.0, 0.5)));
        auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                        Node_Ranged::Range(std::make_pair(0.0, 0.0), std::make_pair(1.0, 0.5)),
                                                        lines);
        nfr->feature = new Feature(Feature::Y, 0.0, 0.5, 2, false);
        auto predicate = make_predicate_vc(nfr->feature->predicate(), Rete::WME_Token_Index(Feature::Y, 2), nfr->feature->symbol_constant(), node_unsplit->action.lock()->parent());
        nfr->action = make_action_retraction([this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(nfr->q_value);
        }, [this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, nfr->q_value);
        }, predicate);
        node_unsplit->fringe_values.push_back(nfr);
      }

      {
        auto nfr = std::make_shared<Node_Fringe_Ranged>(*this, 2,
                                                        Node_Ranged::Range(std::make_pair(0.0, 0.5), std::make_pair(1.0, 1.0)),
                                                        Node_Ranged::Lines());
        nfr->feature = new Feature(Feature::Y, 0.5, 1.0, 2, true);
        auto predicate = make_predicate_vc(nfr->feature->predicate(), Rete::WME_Token_Index(Feature::Y, 2), nfr->feature->symbol_constant(), node_unsplit->action.lock()->parent());
        nfr->action = make_action_retraction([this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->m_next_q_values[action].push_back(nfr->q_value);
        }, [this,action,nfr](const Rete::Rete_Action &, const Rete::WME_Token &) {
          this->purge_q_value_next(action, nfr->q_value);
        }, predicate);
        node_unsplit->fringe_values.push_back(nfr);
      }
    }
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());
    const auto pos = env->get_position();

    clear_wmes();

    m_x_value->value = pos.first;
    m_y_value->value = pos.second;
    insert_wme(m_x_wme);
    insert_wme(m_y_wme);
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    m_metastate = env->goal_reached() ? Metastate::SUCCESS : Metastate::NON_TERMINAL;
  }

}
