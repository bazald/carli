#ifndef CARLI_AGENT_H
#define CARLI_AGENT_H

#include "utility/getopt.h"
#include "utility/tracked_ptr.h"
#include "utility/random.h"

#include "rete/rete_agent.h"
#include "rete/symbol.h"

#include "environment.h"
#include "feature.h"
#include "node.h"
#include "q_value.h"
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

  class CARLI_LINKAGE Carli_Data : public Rete::Rete_Data {
  public:
    Carli_Data(const std::function<Action_Ptr_C (const Rete::WME_Token &)> &get_action_, const Node_Ptr &node_)
      : get_action(get_action_),
      node(node_)
    {
    }

    Rete_Data * clone() const override {
      return new Carli_Data(get_action, Node_Ptr(node->clone()));
    }

    void action(Agent &agent, const Rete::WME_Token &token) {
      node->action(agent, token);
    }

    void retraction(Agent &agent, const Rete::WME_Token &token) {
      node->retraction(agent, token);
    }

    std::function<Action_Ptr_C (const Rete::WME_Token &)> get_action;
    Node_Ptr node;
  };

  class CARLI_LINKAGE Agent : public std::enable_shared_from_this<Agent>, public Rete::Rete_Agent {
    Agent(const Agent &) = delete;
    Agent & operator=(const Agent &) = delete;

  public:
    typedef Feature feature_type;
    typedef Action action_type;
    typedef double reward_type;
    typedef std::list<tracked_ptr<Q_Value>, Zeni::Pool_Allocator<tracked_ptr<Q_Value>>> Q_Value_List;

    bool specialize(Carli::Carli_Data &data, const Rete::WME_Token &token);
    void expand_fringe(Carli::Carli_Data &data, const Rete::WME_Token &token, const Feature * const &specialization);

    Agent(const std::shared_ptr<Environment> &environment);

    virtual ~Agent();

    void destroy();

    bool is_null_q_values() const {return m_null_q_values;}
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
    double get_split_cabe() const {return m_split_cabe;}
    double get_split_cabe_qmult() const {return m_split_cabe_qmult;}
    int64_t get_contribute_update_count() const {return m_contribute_update_count;}
    int64_t get_value_function_cap() const {return m_value_function_cap;}
    int64_t get_mean_cabe_queue_size() const {return m_mean_cabe_queue_size;}

    std::shared_ptr<const Environment> get_env() const {return m_environment;}
    const std::shared_ptr<Environment> & get_env() {return m_environment;}
    Metastate get_metastate() const {return m_metastate;}
    int64_t get_episode_number() const {return m_episode_number;}
    int64_t get_step_count() const {return m_step_count;}
    reward_type get_total_reward() const {return m_total_reward;}
  //  Mean get_mean_cabe() const {return m_mean_cabe;}
  //#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
  //  Mean get_mean_mabe() const {return m_mean_mabe;}
  //#endif

    void reset_statistics();

    void init();

    reward_type act();

    void purge_q_value(const tracked_ptr<Q_Value> &q_value);

    void insert_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value);
    void purge_q_value_next(const Action_Ptr_C &action, const tracked_ptr<Q_Value> &q_value);

    void purge_q_value_eligible(const tracked_ptr<Q_Value> &q_value);

    void print(std::ostream &os) const;

    void reset_update_counts();

    void print_value_function_grid(std::ostream &os) const;

    int64_t q_value_count = 0;

  protected:
    Action_Ptr_C choose_epsilon_greedy(const double &epsilon);
    Action_Ptr_C choose_greedy();
    Action_Ptr_C choose_randomly();

    void td_update(const Q_Value_List &current, const reward_type &reward, const Q_Value_List &next);

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

    Node_Fringe_Ptr split_test(const Rete::WME_Token &token, const Node_Unsplit_Ptr &general);

    static double sum_value(const action_type * const &action, const Q_Value_List &value_list);

    void clean_features();

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

    void print_value_function_grid_set(std::ostream &os, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &line_segments) const;

    void merge_value_function_grid_sets(std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &combination, const std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>> &additions) const;

    Metastate m_metastate = Metastate::NON_TERMINAL;
    Action_Ptr_C m_current;
    Q_Value_List m_current_q_value;
    Action_Ptr_C m_next;
    std::map<Action_Ptr_C, Q_Value_List, compare_deref_lt, Zeni::Pool_Allocator<std::pair<Action_Ptr_C, Q_Value_List>>> m_next_q_values;
    std::function<Action_Ptr_C ()> m_target_policy; ///< Sarsa/Q-Learning selector
    std::function<Action_Ptr_C ()> m_exploration_policy; ///< Exploration policy
    std::function<Node_Fringe_Ptr (const Rete::WME_Token &, const Node_Unsplit_Ptr &)> m_split_test; ///< true if too general, false if sufficiently general
    std::map<Action_Ptr_C, std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>>, std::less<Action_Ptr_C>, Zeni::Pool_Allocator<std::pair<Action_Ptr_C, std::set<typename Node_Ranged::Line, std::less<typename Node_Ranged::Line>, Zeni::Pool_Allocator<typename Node_Ranged::Line>>>>> m_lines;

    Rete::Symbol_Identifier_Ptr_C m_s_id = Rete::Symbol_Identifier_Ptr_C(new Rete::Symbol_Identifier("S1"));
    Rete::WME_Ptr_C m_wme_blink = Rete::WME_Ptr_C(new Rete::WME(m_s_id, m_s_id, m_s_id));

  private:
    virtual void generate_features() = 0;
    virtual void update() = 0;

    Mean m_mean_cabe;
    Value_Queue m_mean_cabe_queue;
    const int64_t m_mean_cabe_queue_size = get_Option_Ranged<int64_t>(Options::get_global(), "mean-cabe-queue-size");

  #ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
    Mean m_mean_mabe;
  #endif

  #ifdef WHITESON_ADAPTIVE_TILE
    size_t m_steps_since_minbe = 0;
  #endif

    std::shared_ptr<Environment> m_environment;

    Zeni::Random random;

    int64_t m_episode_number = 1;
    int64_t m_step_count = 0;
    reward_type m_total_reward = 0.0;

    const bool m_null_q_values = get_Option_Ranged<bool>(Options::get_global(), "null-q-values"); ///< insert nullptr instead of new Q_Values until reaching the leaf
    const int64_t m_value_function_cap = get_Option_Ranged<int64_t>(Options::get_global(), "value-function-cap"); ///< at this threshold, no more entries will be added to the value functions through refinement

    const double m_learning_rate = get_Option_Ranged<double>(Options::get_global(), "learning-rate"); ///< alpha
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
    const double m_epsilon = get_Option_Ranged<double>(Options::get_global(), "epsilon-greedy"); ///< for epsilon-greedy decision-making

    const int64_t m_split_min = get_Option_Ranged<int64_t>(Options::get_global(), "split-min");
    const int64_t m_split_max = get_Option_Ranged<int64_t>(Options::get_global(), "split-max");
    const double m_split_cabe = get_Option_Ranged<double>(Options::get_global(), "split-cabe");
    const double m_split_cabe_qmult = get_Option_Ranged<double>(Options::get_global(), "split-cabe-qmult");

    const int64_t m_pseudoepisode_threshold = get_Option_Ranged<int64_t>(Options::get_global(), "pseudoepisode-threshold"); ///< For deciding how many steps indicates a pseudoepisode
    const int64_t m_split_pseudoepisodes = get_Option_Ranged<int64_t>(Options::get_global(), "split-pseudoepisodes");
    const int64_t m_split_update_count = get_Option_Ranged<int64_t>(Options::get_global(), "split-update-count");

    const bool m_generate_line_segments = get_Option_Ranged<bool>(Options::get_global(), "generate-line-segments");
    const int64_t m_contribute_update_count = get_Option_Ranged<int64_t>(Options::get_global(), "contribute-update-count");
    const bool m_dynamic_midpoint = get_Option_Ranged<bool>(Options::get_global(), "dynamic-midpoint");
    const bool m_fringe = get_Option_Ranged<bool>(Options::get_global(), "fringe");
    const double m_fringe_learning_scale = get_Option_Ranged<double>(Options::get_global(), "fringe-learning-scale");

    Q_Value::List * m_eligible = nullptr;

  #ifndef NDEBUG
    void increment_badness() {++m_badness; assert(m_badness);}
    void decrement_badness() {assert(m_badness); --m_badness;}
    size_t m_badness = 0u;
  #endif

    std::string m_value_function_map_mode = dynamic_cast<const Option_Itemized &>(Options::get_global()["value-function-map-mode"]).get_value();
    std::string m_value_function_map_filename = dynamic_cast<const Option_String &>(Options::get_global()["value-function-map-filename"]).get_value();
    std::unordered_set<std::string> m_value_function_map;
    mutable std::ofstream m_value_function_out;
  };

}

CARLI_LINKAGE std::ostream & operator<<(std::ostream &os, const Carli::Agent &agent);

#endif
