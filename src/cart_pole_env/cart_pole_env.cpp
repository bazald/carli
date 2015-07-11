#include "cart_pole.h"

#include "carli/parser/rete_parser.h"

namespace Cart_Pole {

  using Carli::Metastate;
  using Carli::Node_Fringe;
  using Carli::Node_Split;
  using Carli::Node_Unsplit;
  using Carli::Q_Value;

  Environment::Environment() {
    init_impl();
  }

  bool Environment::success() const {
    return m_has_goal &&
           (m_ignore_x || (m_x >= 4.1 && m_x <= 4.5)) &&
           m_theta >= -0.06981317007 && m_theta <= 0.06981317007; ///< pi / 45.0
  }

  bool Environment::failed() const {
    return m_x < -m_max_x || m_x > m_max_x ||
           m_theta < -m_max_theta || m_theta > m_max_theta;
  }

  void Environment::init_impl() {
    m_theta = 0.0;
    m_theta_dot = 0.0;
    m_x = 0.0;
    m_x_dot = 0.0;

    if(m_has_goal) {
      do {
        if(!m_ignore_x) {
          m_x = m_random_init.frand_lte() * (m_max_x - 0.2) - m_max_x / 2.0; ///< Inner half of range
          if(m_x >= 4.1 && m_x <= 4.5)
            m_x += 0.4;
        }

        m_theta = m_random_init.frand_lte() * (m_max_theta - 0.06981317007) - m_max_theta / 2.0; ///< Inner half of range
        if(m_theta >= -0.06981317007 && m_theta <= 0.06981317007)
          m_theta += 2 * 0.06981317007;
      } while(success());
    }

    m_random_motion = Zeni::Random(m_random_init.rand());
  }

  Agent::reward_type Environment::transition_impl(const Carli::Action &action) {
    const bool move_right = static_cast<const Move &>(action).direction == RIGHT;

    const double force = move_right ? FORCE_MAG : -FORCE_MAG;
    const double costheta = cos(m_theta);
    const double sintheta = sin(m_theta);

    const double temp = (force + POLEMASS_LENGTH * m_theta_dot * m_theta_dot * sintheta) / TOTAL_MASS;

    const double thetaacc = (GRAVITY * sintheta - costheta * temp) /
                            (LENGTH * (FOURTHIRDS - MASSPOLE * costheta * costheta / TOTAL_MASS));

    if(!m_ignore_x) {
      const double xacc  = temp - POLEMASS_LENGTH * thetaacc* costheta / TOTAL_MASS;

      m_x += TAU * m_x_dot;
      m_x_dot += std::max(-m_max_x_dot, std::min(m_max_x_dot, m_x_dot + TAU * xacc));
    }

    m_theta += TAU * m_theta_dot;
    m_theta_dot = std::max(-m_max_theta_dot, std::min(m_max_theta_dot, m_theta_dot + TAU * thetaacc));

    double reward = 0.0;

    reward += fabs(m_theta) / m_max_theta < 0.1 ? 10.0 : fabs(m_theta) / m_max_theta < 0.2 ? 5.0 : 0.0;
    if(m_theta < 0.0) {
      if(m_theta_dot < 0.0)
        reward += -m_theta_dot / m_max_theta_dot < 0.1 ? 0.0 : -m_theta_dot / m_max_theta_dot < 0.2 ? -1.0 : -10.0;
      else
        reward += 1.0;
    }
    else {
      if(m_theta_dot < 0.0)
        reward += 1.0;
      else
        reward += m_theta_dot / m_max_theta_dot < 0.1 ? 0.0 : m_theta_dot / m_max_theta_dot < 0.2 ? -1.0 : -10.0;
    }

    if(!m_ignore_x) {
      reward += fabs(m_x) / m_max_x < 0.1 ? 10.0 : fabs(m_x) / m_max_x < 0.2 ? 5.0 : 0.0;
      if(m_x < 0.0) {
        if(m_x_dot < 0.0)
          reward += -m_x_dot / m_max_x_dot < 0.1 ? 0.0 : -m_x_dot / m_max_x_dot < 0.2 ? -1.0 : -10.0;
        else
          reward += 1.0;
      }
      else {
        if(m_x_dot < 0.0)
          reward += 1.0;
        else
          reward += m_x_dot / m_max_x_dot < 0.1 ? 0.0 : m_x_dot / m_max_x_dot < 0.2 ? -1.0 : -10.0;
      }
    }

    return reward;

    return success() ? 1.0 : failed() ? -1.0 : 0.0;
  }

  void Environment::print_impl(ostream &os) const {
    os << "Cart Pole:" << endl;
    os << " (" << m_x << ", " << m_x_dot << ", " << m_theta << ", " << m_theta_dot << ')' << endl;
  }

  Agent::Agent(const std::shared_ptr<Carli::Environment> &env)
   : Carli::Agent(env, [this](const Rete::Variable_Indices &variables, const Rete::WME_Token &token)->Carli::Action_Ptr_C {return std::make_shared<Move>(variables, token);}),
   m_action({{std::shared_ptr<const Carli::Action>(new Move(LEFT)),
              std::shared_ptr<const Carli::Action>(new Move(RIGHT))}})
  {
    const Rete::Symbol_Variable_Ptr_C m_first_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::First));
    const Rete::Symbol_Variable_Ptr_C m_third_var = Rete::Symbol_Variable_Ptr_C(new Rete::Symbol_Variable(Rete::Symbol_Variable::Third));

    const auto move_attr = std::make_shared<Rete::Symbol_Constant_String>("move");
    const auto x_attr = std::make_shared<Rete::Symbol_Constant_String>("x");
    const auto x_dot_attr = std::make_shared<Rete::Symbol_Constant_String>("x-dot");
    const auto theta_attr = std::make_shared<Rete::Symbol_Constant_String>("theta");
    const auto theta_dot_attr = std::make_shared<Rete::Symbol_Constant_String>("theta-dot");
    const std::array<Rete::Symbol_Constant_Int_Ptr_C, 2> move_values = {{std::make_shared<Rete::Symbol_Constant_Int>(LEFT),
                                                                         std::make_shared<Rete::Symbol_Constant_Int>(RIGHT)}};

    generate_rete();
    generate_features();

    m_x_wme = std::make_shared<Rete::WME>(m_s_id, x_attr, m_x_value);
    m_x_dot_wme = std::make_shared<Rete::WME>(m_s_id, x_dot_attr, m_x_dot_value);
    m_theta_wme = std::make_shared<Rete::WME>(m_s_id, theta_attr, m_theta_value);
    m_theta_dot_wme = std::make_shared<Rete::WME>(m_s_id, theta_dot_attr, m_theta_dot_value);
    insert_wme(m_x_wme);
    insert_wme(m_x_dot_wme);
    insert_wme(m_theta_wme);
    insert_wme(m_theta_dot_wme);
    for(const auto &move_value : move_values)
      insert_wme(std::make_shared<Rete::WME>(m_s_id, move_attr, move_value));
  }

  Agent::~Agent() {
    destroy();
  }

  void Agent::generate_rete() {
    std::string rules_in = dynamic_cast<const Option_String &>(Options::get_global()["rules"]).get_value();
    if(rules_in == "default")
      rules_in = "rules/cart-pole.carli";
    if(rete_parse_file(*this, rules_in))
      abort();
  }

  void Agent::generate_features() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    Rete::Agenda::Locker locker(agenda);
    CPU_Accumulator cpu_accumulator(*this);

    const bool flush_wmes = get_Option_Ranged<bool>(Options::get_global(), "rete-flush-wmes");

    if(flush_wmes || m_x_value->value != env->get_x()) {
      remove_wme(m_x_wme);
      m_x_wme->symbols[2] = m_x_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(env->get_x()));
      insert_wme(m_x_wme);
    }
    if(flush_wmes || m_x_dot_value->value != env->get_x_dot()) {
      remove_wme(m_x_dot_wme);
      m_x_dot_wme->symbols[2] = m_x_dot_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(env->get_x_dot()));
      insert_wme(m_x_dot_wme);
    }
    if(flush_wmes || m_theta_value->value != env->get_theta()) {
      remove_wme(m_theta_wme);
      m_theta_wme->symbols[2] = m_theta_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(env->get_theta()));
      insert_wme(m_theta_wme);
    }
    if(flush_wmes || m_theta_dot_value->value != env->get_theta_dot()) {
      remove_wme(m_theta_dot_wme);
      m_theta_dot_wme->symbols[2] = m_theta_dot_value = Rete::Symbol_Constant_Float_Ptr_C(new Rete::Symbol_Constant_Float(env->get_theta_dot()));
      insert_wme(m_theta_dot_wme);
    }
  }

  void Agent::update() {
    auto env = dynamic_pointer_cast<const Environment>(get_env());

    if(env->failed())
      m_metastate = Metastate::FAILURE;
    else if(env->has_goal() ? env->success() : get_step_count() > 9999)
      m_metastate = Metastate::SUCCESS;
    else
      m_metastate = Metastate::NON_TERMINAL;
  }

}
