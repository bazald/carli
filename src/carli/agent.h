#ifndef CARLI_AGENT_H
#define CARLI_AGENT_H

#include "utility/tracked_ptr.h"
#include "utility/random.h"

#include "rete/rete_agent.h"

#include "environment.h"
#include "feature.h"
#include "node.h"
#include "q_value.h"
#include "t_test.h"
#include "value_queue.h"

#include <functional>
#include <map>
#include <set>
#include <stack>
#include <string>

#include <iostream>
#include <fstream>

namespace Carli {

  enum class Metastate : char {NON_TERMINAL, SUCCESS, FAILURE};

  class CARLI_LINKAGE Agent : public std::enable_shared_from_this<Agent>, public Rete::Rete_Agent {
    Agent(const Agent &) = delete;
    Agent & operator=(const Agent &) = delete;

  public:
    typedef Feature feature_type;
    typedef Action action_type;
    typedef double reward_type;
    typedef std::unordered_map<tracked_ptr<Q_Value>, int64_t> Q_Value_List;

    bool respecialize(Rete::Rete_Action &rete_action);
    bool specialize(Rete::Rete_Action &rete_action);
    void expand_fringe(Rete::Rete_Action &rete_action, const Rete::Rete_Action_Ptr &parent_action, const Fringe_Values::iterator &specialization);
    bool collapse_rete(Rete::Rete_Action &rete_action); ///< Collapses and returns true unless there exist no nodes to collapse into a new fringe

    Agent(const std::shared_ptr<Environment> &environment, const std::function<Carli::Action_Ptr_C (const Rete::Variable_Indices &variables, const Rete::WME_Token &token)> &get_action_);

    virtual ~Agent();

    void destroy();

    double get_learning_rate() const {return m_learning_rate;}
    double get_discount_rate() const {return m_discount_rate;}
    double get_eligibility_trace_decay_rate() const {return m_eligibility_trace_decay_rate;}
    double get_eligibility_trace_decay_threshold() const {return m_eligibility_trace_decay_threshold;}
    std::string get_credit_assignment() const {return m_credit_assignment_code;}
    double get_credit_assignment_epsilon() const {return m_credit_assignment_epsilon;}
    double get_credit_assignment_log_base() const {return m_credit_assignment_log_base;}
    double get_credit_assignment_root() const {return m_credit_assignment_root;}
    bool is_credit_assignment_normalize() const {return m_credit_assignment_normalize;}
  #ifdef ENABLE_WEIGHT
    std::string get_weight_assignment() const {return m_weight_assignment_code;}
  #endif
    bool is_on_policy() const {return m_on_policy;}
    double get_epsilon() const {return m_epsilon;}
    int64_t get_split_min() const {return m_split_min;}
    int64_t get_split_max() const {return m_split_max;}
    int64_t get_pseudoepisode_threshold() const {return m_pseudoepisode_threshold;}
    int64_t get_split_pseudoepisodes() const {return m_split_pseudoepisodes;}
    int64_t get_split_update_count() const {return m_split_update_count;}
    double get_split_catde() const {return m_split_catde;}
    double get_split_catde_qmult() const {return m_split_catde_qmult;}
    int64_t get_contribute_update_count() const {return m_contribute_update_count;}
    int64_t get_value_function_cap() const {return m_value_function_cap;}
    int64_t get_mean_catde_queue_size() const {return m_mean_catde_queue_size;}

    std::shared_ptr<const Environment> get_env() const {return m_environment;}
    const std::shared_ptr<Environment> & get_env() {return m_environment;}
    Metastate get_metastate() const {return m_metastate;}
    int64_t get_episode_number() const {return m_episode_number;}
    int64_t get_total_step_count() const {return m_total_step_count;}
    int64_t get_step_count() const {return m_step_count;}
    reward_type get_total_reward() const {return m_total_reward;}
  //  Mean get_mean_catde() const {return m_mean_catde;}
  //#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
  //  Mean get_mean_matde() const {return m_mean_matde;}
  //#endif
    const std::map<int64_t, int64_t> & get_unrefinements() const {return m_unrefinements;}

    void set_total_step_count(const int64_t &total_step_count_) {m_total_step_count = total_step_count_;}

    void reset_statistics();

    void init();

    reward_type act();

    Rete::Rete_Action_Ptr make_standard_action(const Rete::Rete_Node_Ptr &parent, const std::string &name, const bool &user_command, const Rete::Variable_Indices_Ptr_C &variables);
    Rete::Rete_Action_Ptr make_standard_fringe(const Rete::Rete_Node_Ptr &parent, const std::string &name, const bool &user_command, const Node_Unsplit_Ptr &root_action_data, const tracked_ptr<Feature> &feature, const Rete::Variable_Indices_Ptr_C &variables);

    void purge_q_value(const tracked_ptr<Q_Value> &q_value);

    void insert_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value);
    void purge_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value);

    void purge_q_value_eligible(const tracked_ptr<Q_Value> &q_value);

    void print(std::ostream &os) const;

    //void print_value_function_grid(std::ostream &os) const;

    void visit_increment_depth();
    void visit_reset_update_count();

    const std::function<Carli::Action_Ptr_C (const Rete::Variable_Indices &variables, const Rete::WME_Token &token)> get_action;
    int64_t q_value_count = 0;

    std::unordered_map<Node_Ptr, int64_t> m_nodes_active;
    std::unordered_map<Node_Ptr, int64_t> m_nodes_activating;

    const bool terse_out = get_Option_Ranged<bool>(Options::get_global(), "terse-out");

  protected:
    Action_Ptr_C choose_boltzmann(const Node_Fringe * const &fringe, const int64_t &fringe_depth);
    Action_Ptr_C choose_epsilon_greedy(const Node_Fringe * const &fringe, const int64_t &fringe_depth);
#ifdef ENABLE_T_TEST
    Action_Ptr_C choose_t_test(const Node_Fringe * const &fringe, const int64_t &fringe_depth);
#endif
    Action_Ptr_C choose_greedy(const Node_Fringe * const &fringe, const int64_t &fringe_depth);
    std::list<Action_Ptr_C, Zeni::Pool_Allocator<Action_Ptr_C>> choose_greedies(const Node_Fringe * const &fringe, const int64_t &fringe_depth);
    Action_Ptr_C choose_randomly();

    double probability_epsilon_greedy(const Action_Ptr_C &action, const double &epsilon, const Node_Fringe * const &fringe, const int64_t &fringe_depth);
    double probability_greedy(const Action_Ptr_C &action, const Node_Fringe * const &fringe, const int64_t &fringe_depth);
    double probability_random();

    void td_update(const Q_Value_List &current, const reward_type &reward, const Q_Value_List &next, const double &rho, const double &I);

    void clear_eligibility_trace();

    void assign_credit_epsilon(const Q_Value_List &value_list,
                               void (Agent::*exploration)(const Q_Value_List &),
                               void (Agent::*target)(const Q_Value_List &));

    void assign_credit_random(const Q_Value_List &value_list);
    void assign_credit_specific(const Q_Value_List &value_list);
    void assign_credit_evenly(const Q_Value_List &value_list);
    void assign_credit_all(const Q_Value_List &value_list);
    void assign_credit_inv_update_count(const Q_Value_List &value_list);
    void assign_credit_inv_log_update_count(const Q_Value_List &value_list);
    void assign_credit_inv_root_update_count(const Q_Value_List &value_list);
    void assign_credit_inv_depth(const Q_Value_List &value_list);

    void assign_credit_normalize(const Q_Value_List &value_list, const double &sum);

    Fringe_Values::iterator split_test_catde(Node_Unsplit &general);
    Fringe_Values::iterator split_test_policy(Node_Unsplit &general);
    Fringe_Values::iterator split_test_value(Node_Unsplit &general);
    Fringe_Values::iterator split_test_utile(Node_Unsplit &general);

    bool unsplit_test_catde(Node_Split &general);
    bool unsplit_test_policy(Node_Split &general);
    bool unsplit_test_value(Node_Split &general);
    bool unsplit_test_utile(Node_Split &general);

    /// Get the sample mean and standard deviation
    std::tuple<double, double, int64_t> sum_value(const action_type * const &action, const Q_Value_List &value_list, const Feature * const &axis, const int64_t &fringe_depth) const;

    void generate_all_features();

  #ifdef DEBUG_OUTPUT
    template <typename LIST>
    static void print_list(std::ostream &os, const std::string &head, const std::string &pre, const LIST &list) {
      if(list) {
        os << head;
        for(const auto &value : *list)
          os << pre << value;
        os << std::endl;
      }
    }
  #endif

    //void print_value_function_grid_set(std::ostream &os, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &line_segments) const;

    //void merge_value_function_grid_sets(std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &combination, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &additions) const;

    Metastate m_metastate = Metastate::NON_TERMINAL;
    Action_Ptr_C m_current;
    Q_Value_List m_current_q_value;
    Action_Ptr_C m_next;
    std::map<Action_Ptr_C, Q_Value_List, Rete::compare_deref_lt, Zeni::Pool_Allocator<std::pair<Action_Ptr_C, Q_Value_List>>> m_next_q_values;
    std::function<Action_Ptr_C ()> m_target_policy; ///< Sarsa/Q-Learning selector
    std::function<Action_Ptr_C ()> m_exploration_policy; ///< Exploration policy
    std::function<Fringe_Values::iterator (Node_Unsplit &)> m_split_criterion; ///< true if too general, false if sufficiently general
    std::function<bool (Node_Split &)> m_unsplit_criterion; ///< true if too general, false if sufficiently general
    //std::map<Action_Ptr_C, std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>>, std::less<Action_Ptr_C>, Zeni::Pool_Allocator<std::pair<Action_Ptr_C, std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>>>>> m_lines;

    Rete::Symbol_Identifier_Ptr_C m_s_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("S1"));

  private:
    virtual void generate_features() = 0;
    virtual void update() = 0;

    Mean m_mean_catde;
    Value_Queue m_mean_catde_queue;
    const int64_t m_mean_catde_queue_size = get_Option_Ranged<int64_t>(Options::get_global(), "mean-catde-queue-size");

  #ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
    Mean m_mean_matde;
  #endif

  #ifdef WHITESON_ADAPTIVE_TILE
    size_t m_steps_since_minbe = 0;
  #endif

    std::shared_ptr<Environment> m_environment;

    Zeni::Random random;

    int64_t m_episode_number = 1;
    int64_t m_step_count = 0;
    int64_t m_total_step_count = 0;
    reward_type m_total_reward = 0.0;
    const int64_t m_step_cutoff = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["step-cutoff"]).get_value();

    const int64_t m_value_function_cap = get_Option_Ranged<int64_t>(Options::get_global(), "value-function-cap"); ///< at this threshold, no more entries will be added to the value functions through refinement

    const double m_learning_rate = get_Option_Ranged<double>(Options::get_global(), "learning-rate"); ///< alpha
    const double m_secondary_learning_rate = get_Option_Ranged<double>(Options::get_global(), "secondary-learning-rate"); ///< eta
    const double m_discount_rate = get_Option_Ranged<double>(Options::get_global(), "discount-rate"); ///< gamma
    const double m_eligibility_trace_decay_rate = get_Option_Ranged<double>(Options::get_global(), "eligibility-trace-decay-rate"); ///< lambda
    const double m_eligibility_trace_decay_threshold = get_Option_Ranged<double>(Options::get_global(), "eligibility-trace-decay-threshold");

    const std::string m_credit_assignment_code = dynamic_cast<const Option_Itemized &>(Options::get_global()["credit-assignment"]).get_value();
    const std::function<void (const Q_Value_List &)> m_credit_assignment; ///< How to assign credit to multiple Q-values
    const double m_credit_assignment_epsilon = get_Option_Ranged<double>(Options::get_global(), "credit-assignment-epsilon");
    const double m_credit_assignment_log_base = get_Option_Ranged<double>(Options::get_global(), "credit-assignment-log-base");
    const double m_credit_assignment_log_base_value = std::log(m_credit_assignment_log_base);
    const double m_credit_assignment_root = get_Option_Ranged<double>(Options::get_global(), "credit-assignment-root");
    const double m_credit_assignment_root_value = 1.0 / m_credit_assignment_root;
    const bool m_credit_assignment_normalize = get_Option_Ranged<bool>(Options::get_global(), "credit-assignment-normalize");

  //#ifdef ENABLE_WEIGHT
  //  const std::string m_weight_assignment_code = dynamic_cast<const Option_Itemized &>(Options::get_global()["weight-assignment"]).get_value();
  //  const std::function<void (Q_Value::List * const &)> m_weight_assignment; ///< How to assign weight to multiple Q-values at summation time
  //#endif

    const bool m_on_policy = dynamic_cast<const Option_Itemized &>(Options::get_global()["policy"]).get_value() == "on-policy"; ///< for Sarsa/Q-learning selection
    const double m_inverse_epsilon_episodic_increment = get_Option_Ranged<double>(Options::get_global(), "inverse-epsilon-episodic-increment"); ///< for epsilon-greedy decision-making
    const double m_epsilon_initial = get_Option_Ranged<double>(Options::get_global(), "epsilon-greedy"); ///< for epsilon-greedy decision-making
    double m_epsilon = get_Option_Ranged<double>(Options::get_global(), "epsilon-greedy"); ///< for epsilon-greedy decision-making
#ifdef ENABLE_T_TEST
    const double m_t_test = get_Option_Ranged<double>(Options::get_global(), "t-test"); ///< for t-test decision-making
#endif
    const double m_inverse_temperature_episodic_increment = get_Option_Ranged<double>(Options::get_global(), "inverse-temperature-episodic-increment"); ///< for Boltzmann decision-making
    const double m_inverse_temperature_initial = get_Option_Ranged<double>(Options::get_global(), "inverse-temperature"); ///< for Boltzmann decision-making
    double m_inverse_temperature = get_Option_Ranged<double>(Options::get_global(), "inverse-temperature"); ///< for Boltzmann decision-making

    const int64_t m_pseudoepisode_threshold = get_Option_Ranged<int64_t>(Options::get_global(), "pseudoepisode-threshold"); ///< For deciding how many steps indicates a pseudoepisode
    const double m_split_catde = get_Option_Ranged<double>(Options::get_global(), "split-catde");
    const double m_split_catde_qmult = get_Option_Ranged<double>(Options::get_global(), "split-catde-qmult");
    const int64_t m_split_max = get_Option_Ranged<int64_t>(Options::get_global(), "split-max");
    const int64_t m_split_min = get_Option_Ranged<int64_t>(Options::get_global(), "split-min");
    const int64_t m_split_pseudoepisodes = get_Option_Ranged<int64_t>(Options::get_global(), "split-pseudoepisodes");
    const std::string m_split_test = dynamic_cast<const Option_Itemized &>(Options::get_global()["split-test"]).get_value();
    const std::string m_unsplit_test = dynamic_cast<const Option_Itemized &>(Options::get_global()["unsplit-test"]).get_value();
    const int64_t m_split_update_count = get_Option_Ranged<int64_t>(Options::get_global(), "split-update-count");
    const int64_t m_unsplit_update_count = get_Option_Ranged<int64_t>(Options::get_global(), "unsplit-update-count");
    const int64_t m_concrete_update_count = get_Option_Ranged<int64_t>(Options::get_global(), "concrete-update-count");
    const std::string m_resplit_bias = dynamic_cast<const Option_Itemized &>(Options::get_global()["resplit-bias"]).get_value();
    const double m_resplit_boost_scale = get_Option_Ranged<double>(Options::get_global(), "resplit-boost-scale");
    const double m_split_probability = get_Option_Ranged<double>(Options::get_global(), "split-probability");
    const double m_unsplit_probability = get_Option_Ranged<double>(Options::get_global(), "unsplit-probability");

    const int64_t m_contribute_update_count = get_Option_Ranged<int64_t>(Options::get_global(), "contribute-update-count");
    const bool m_dynamic_midpoint = get_Option_Ranged<bool>(Options::get_global(), "dynamic-midpoint");
    const double m_fringe_learning_scale = get_Option_Ranged<double>(Options::get_global(), "fringe-learning-scale");

    Q_Value::List::list_pointer_type m_eligible;

  #ifndef NDEBUG
    void increment_badness() {++m_badness; assert(m_badness);}
    void decrement_badness() {assert(m_badness); --m_badness;}
    size_t m_badness = 0u;
  #endif

    const std::string m_value_function_map_mode = dynamic_cast<const Option_Itemized &>(Options::get_global()["value-function-map-mode"]).get_value();
    const std::string m_value_function_map_filename = dynamic_cast<const Option_String &>(Options::get_global()["value-function-map-filename"]).get_value();
    std::unordered_set<std::string> m_value_function_map;
    mutable std::ofstream m_value_function_out;

    const bool m_output_dot = get_Option_Ranged<bool>(Options::get_global(), "output-dot");
    size_t g_output_dot_exp_count = 0u;
    size_t g_output_dot_col_count = 0u;

#ifndef NO_COLLAPSE_DETECTION_HACK
  protected:
    size_t m_positive_rewards_in_a_row = 0;
    bool m_experienced_n_positive_rewards_in_a_row = false;
#endif

    std::map<int64_t, int64_t> m_unrefinements;
  };

}

CARLI_LINKAGE std::ostream & operator<<(std::ostream &os, const Carli::Agent &agent);

#endif
