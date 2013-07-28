#ifndef AGENT_H
#define AGENT_H

#include "environment.h"
#include "feature.h"
#include "tracked_ptr.h"
#include "trie.h"
#include "q_value.h"
#include "random.h"
#include "value_queue.h"

#include "rete/agent.h"

#include <functional>
#include <map>
#include <set>
#include <stack>
#include <string>

#include <iostream>

template <typename DERIVED, typename DERIVED2>
std::ostream & operator << (std::ostream &os, const Feature<DERIVED, DERIVED2> &feature) {
  feature.print(os);
  return os;
}

enum class Credit_Assignment : char {ALL, SPECIFIC, EVEN, INV_UPDATE_COUNT, INV_LOG_UPDATE_COUNT, INV_ROOT_UPDATE_COUNT, INV_DEPTH, EPSILON_EVEN_SPECIFIC, EPSILON_EVEN_DEPTH};
enum class Metastate : char {NON_TERMINAL, SUCCESS, FAILURE};

template <typename FEATURE, typename ACTION>
class Agent : public std::enable_shared_from_this<Agent<FEATURE, ACTION>>, public Rete::Agent {
  Agent(const Agent &) = delete;
  Agent & operator=(const Agent &) = delete;

public:
  typedef FEATURE feature_type;
  typedef typename FEATURE::List * feature_list;
  typedef ACTION action_type;
  typedef typename ACTION::List * action_list;
  typedef std::shared_ptr<const typename action_type::derived_type> action_ptrsc;
  typedef Zeni::Trie<std::unique_ptr<feature_type>, Q_Value, typename feature_type::Compare> feature_trie_type;
  typedef feature_trie_type * feature_trie;
  typedef Environment<action_type> environment_type;
//  typedef std::map<action_type *, feature_list, typename action_type::Compare> feature_list_type;
//  typedef std::map<action_type *, feature_trie, typename action_type::Compare> value_function_type;
  typedef double reward_type;

  class RL : public std::enable_shared_from_this<RL> {
    RL(const RL &) = delete;
    RL & operator=(const RL &) = delete;

  public:
    typedef std::unordered_multimap<Rete::WME_Vector_Index, std::shared_ptr<RL>> Fringe_Values;

    RL(const size_t &depth_)
     : depth(depth_)
    {
    }

    ~RL() {
      q_value.delete_and_zero();
      fringe_values.delete_and_zero();
      feature.delete_and_zero();
    }

    size_t depth;
    tracked_ptr<Q_Value> q_value;
    std::weak_ptr<Rete::Rete_Action> action;

    /// Present only for Leaf nodes
    tracked_ptr<Fringe_Values> fringe_values;

    /// Present only for Fringe nodes
    tracked_ptr<feature_type> feature;
  };

  void specialize(const std::shared_ptr<typename action_type::derived_type> &action, RL &general) {
    if(general.fringe_values && split_test(general.q_value, general.depth)) {
      /// TODO: choose intelligently again
      generate_fringe(action, general, general.fringe_values->begin()->first);
    }
  }

  void generate_fringe(const std::shared_ptr<typename action_type::derived_type> &action, RL &general, const Rete::WME_Vector_Index &specialization) {
    assert(general.fringe_values);

    auto nexts = general.fringe_values->equal_range(specialization);
    typename RL::Fringe_Values leaves;
    while(nexts.first != nexts.second) {
      leaves.insert(*nexts.first);
      general.fringe_values->erase(nexts.first++);
    }

    for(auto &leaf : leaves) {
      auto &leaf_rl = *leaf.second;

      leaf_rl.q_value->eligible.erase();
      auto found = std::find(m_current_q_value.begin(), m_current_q_value.end(), leaf_rl.q_value);
      if(found != m_current_q_value.end())
        m_current_q_value.erase(found);
      found = std::find(m_next_q_values[action].begin(), m_next_q_values[action].end(), leaf_rl.q_value);
      if(found != m_next_q_values[action].end())
        m_next_q_values[action].erase(found);
//#ifdef DEBUG_OUTPUT
//      std::cerr << " delete  : " << leaf_rl.q_value << std::endl;
//#endif
      leaf_rl.q_value.delete_and_zero();
      leaf_rl.q_value = new Q_Value();
//#ifdef DEBUG_OUTPUT
//      std::cerr << " create  : " << leaf_rl.q_value << std::endl;
//#endif
      ++m_q_value_count;

      leaf_rl.fringe_values = new typename RL::Fringe_Values;

      auto refined = leaf_rl.feature->refined();
      for(auto &refined_feature : refined) {
        auto rl = std::make_shared<RL>(leaf_rl.depth + 1);
        rl->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE);
        rl->feature = refined_feature;
        auto predicate = make_predicate_vc(refined_feature->predicate(), leaf.first, refined_feature->symbol_constant(), leaf_rl.action.lock()->parent());
        rl->action = make_action([this,&action,rl](const Rete::Rete_Action &, const Rete::WME_Vector &) {
          this->specialize(action, *rl);
          this->m_next_q_values[action].push_back(rl->q_value);
        }, predicate);

        leaf_rl.fringe_values->insert(std::make_pair(leaf.first, rl));
      }
      leaf_rl.feature.delete_and_zero();

      for(auto &fringe : *general.fringe_values) {
        auto &fringe_rl = *fringe.second;

        auto rl = std::make_shared<RL>(leaf_rl.depth + 1);
        rl->q_value = new Q_Value(0.0, Q_Value::Type::FRINGE);
        rl->feature = fringe_rl.feature->clone();
        auto predicate = make_predicate_vc(rl->feature->predicate(), fringe.first, rl->feature->symbol_constant(), leaf_rl.action.lock()->parent());
        rl->action = make_action([this,&action,rl](const Rete::Rete_Action &, const Rete::WME_Vector &) {
          this->specialize(action, *rl);
          this->m_next_q_values[action].push_back(rl->q_value);
        }, predicate);

        leaf_rl.fringe_values->insert(std::make_pair(fringe.first, rl));
      }

      if(leaf_rl.fringe_values->empty())
        leaf_rl.fringe_values.delete_and_zero();
    }

    for(auto &fringe : *general.fringe_values) {
      auto found = std::find(m_current_q_value.begin(), m_current_q_value.end(), fringe.second->q_value);
      if(found != m_current_q_value.end())
        m_current_q_value.erase(found);
      found = std::find(m_next_q_values[action].begin(), m_next_q_values[action].end(), fringe.second->q_value);
      if(found != m_next_q_values[action].end())
        m_next_q_values[action].erase(found);
      fringe.second->q_value.delete_and_zero();
      excise_rule(fringe.second->action.lock());
    }
    general.fringe_values.delete_and_zero();
  }

  Agent(const std::shared_ptr<environment_type> &environment)
    : m_target_policy([this]()->action_ptrsc{return this->choose_greedy();}),
    m_exploration_policy([this]()->action_ptrsc{return this->choose_epsilon_greedy(m_epsilon);}),
    m_split_test([this](Q_Value * const &q, const size_t &depth)->bool{return this->split_test(q, depth);}),
    m_environment(environment),
    m_credit_assignment(
      m_credit_assignment_code == "all" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_all(value_list);} :
      m_credit_assignment_code == "specific" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_specific(value_list);} :
      m_credit_assignment_code == "even" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_evenly(value_list);} :
      m_credit_assignment_code == "inv-update-count" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_inv_update_count(value_list);} :
      m_credit_assignment_code == "inv-log-update-count" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_inv_log_update_count(value_list);} :
      m_credit_assignment_code == "inv-root-update-count" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_inv_root_update_count(value_list);} :
      m_credit_assignment_code == "inv-depth" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_inv_depth(value_list);} :
      m_credit_assignment_code == "epsilon-even-specific" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_specific);} :
      m_credit_assignment_code == "epsilon-even-depth" ?
        [this](const std::list<tracked_ptr<Q_Value>> &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_inv_depth);} :
      std::function<void (const std::list<tracked_ptr<Q_Value>> &)>()
    )
//#ifdef ENABLE_WEIGHT
//     ,
//    m_weight_assignment(
//      m_weight_assignment_code == "all" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_all(value_list);} :
//      m_weight_assignment_code == "specific" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_specific(value_list);} :
//      m_weight_assignment_code == "even" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_evenly(value_list);} :
//      m_weight_assignment_code == "inv-update-count" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_update_count(value_list);} :
//      m_weight_assignment_code == "inv-log-update-count" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_log_update_count(value_list);} :
//      m_weight_assignment_code == "inv-root-update-count" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_root_update_count(value_list);} :
//      m_weight_assignment_code == "inv-depth" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_depth(value_list);} :
//      m_weight_assignment_code == "epsilon-even-specific" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_specific);} :
//      m_weight_assignment_code == "epsilon-even-depth" ?
//        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_inv_depth);} :
//      std::function<void (Q_Value::List * const &)>()
//    )
//#endif
  {
    if(m_on_policy)
      m_target_policy = m_exploration_policy;
  }

  virtual ~Agent() {
//    for(typename value_function_type::iterator vt = m_value_function.begin(), vend = m_value_function.end(); vt != vend; ) {
//      auto ptr = vt->first;
//      vt->second->destroy(vt->second);
//      m_value_function.erase(vt++);
//      delete ptr;
//    }
  }

  bool is_null_q_values() const {return m_null_q_values;}
  double get_learning_rate() const {return m_learning_rate;}
  double get_discount_rate() const {return m_discount_rate;}
  double get_eligibility_trace_decay_rate() const {return m_eligibility_trace_decay_rate;}
  double get_eligibility_trace_decay_threshold() const {return m_eligibility_trace_decay_threshold;}
  Credit_Assignment get_credit_assignment() const {return m_credit_assignment_code;}
  double get_credit_assignment_epsilon() const {return m_credit_assignment_epsilon;}
  double get_credit_assignment_log_base() const {return m_credit_assignment_log_base;}
  double get_credit_assignment_root() const {return m_credit_assignment_root;}
  bool is_credit_assignment_normalize() const {return m_credit_assignment_normalize;}
#ifdef ENABLE_WEIGHT
  Credit_Assignment get_weight_assignment() const {return m_weight_assignment_code;}
#endif
  bool is_on_policy() const {return m_on_policy;}
  double get_epsilon() const {return m_epsilon;}
  size_t get_split_min() const {return m_split_min;}
  size_t get_split_max() const {return m_split_max;}
  size_t get_pseudoepisode_threshold() const {return m_pseudoepisode_threshold;}
  size_t get_split_pseudoepisodes() const {return m_split_pseudoepisodes;}
  size_t get_split_update_count() const {return m_split_update_count;}
  double get_split_cabe() const {return m_split_cabe;}
  double get_split_cabe_qmult() const {return m_split_cabe_qmult;}
  size_t get_contribute_update_count() const {return m_contribute_update_count;}
  size_t get_value_function_cap() const {return m_value_function_cap;}
  size_t get_mean_cabe_queue_size() const {return m_mean_cabe_queue_size;}

  const std::shared_ptr<const environment_type> & get_env() const {return m_environment;}
  std::shared_ptr<environment_type> & get_env() {return m_environment;}
  Metastate get_metastate() const {return m_metastate;}
  size_t get_episode_number() const {return m_episode_number;}
  size_t get_step_count() const {return m_step_count;}
  reward_type get_total_reward() const {return m_total_reward;}
  Mean get_mean_cabe() const {return m_mean_cabe;}
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
  Mean get_mean_mabe() const {return m_mean_mabe;}
#endif

  void reset_statistics() {
    m_episode_number = m_episode_number ? 1 : 0;
    m_step_count = 0;
    m_total_reward = 0.0;
  }

  void init() {
    if(m_metastate != Metastate::NON_TERMINAL)
      ++m_episode_number;
    m_metastate = Metastate::NON_TERMINAL;
    m_step_count = 0;
    m_total_reward = reward_type();

    clear_eligibility_trace();

    generate_features();

    if(m_metastate == Metastate::NON_TERMINAL)
      m_next = m_exploration_policy();

    m_current = m_next;
    m_current_q_value = m_next_q_values[m_next];
  }

  reward_type act() {
    generate_features();

    m_current = m_next;
    m_current_q_value = m_next_q_values[m_next];

    const reward_type reward = m_environment->transition(*m_current);

    update();

    if(m_metastate == Metastate::NON_TERMINAL) {
      generate_features();

      m_next = m_target_policy();
#ifdef DEBUG_OUTPUT
//      for(auto &next_q : m_next_q_values)
//        std::cerr << "   " << *next_q.first << " is an option." << std::endl;
      std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      auto &value_best = m_next_q_values[m_next];
      td_update(m_current_q_value, reward, value_best);

      if(!m_on_policy) {
        action_ptrsc next = m_exploration_policy();

        if(*m_next != *next) {
          if(sum_value(nullptr, m_current_q_value) < sum_value(nullptr, m_next_q_values[next]))
            clear_eligibility_trace();
          m_next = next;
        }

#ifdef DEBUG_OUTPUT
        std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      }
    }
    else {
      td_update(m_current_q_value, reward, std::list<tracked_ptr<Q_Value>>());
    }

    m_total_reward += reward;
    ++m_step_count;

    return reward;
  }

  void print(std::ostream &os) const {
    os << " Agent:\n";
//    print_feature_lists(os);
//    print_list(os, "  Candidates:\n  ", " ", m_candidates);
//#if defined(DEBUG_OUTPUT) && defined(DEBUG_OUTPUT_VALUE_FUNCTION)
//    print_value_function(os);
//#endif
  }

//  void print_feature_lists(std::ostream &os) const {
//    os.unsetf(std::ios_base::floatfield);
//
//    os << "  Features:" << std::endl;
//    for(auto &fl : m_feature_lists) {
//      os << "  " << *fl.first << " :";
//      print_list(os, "", " ", fl.second);
//    }
//
//    os.setf(std::ios_base::fixed, std::ios_base::floatfield);
//  }

//  void print_value_function(std::ostream &os) const {
//    os << "  Value Function:";
//    for(auto &vf : m_value_function) {
//      os << std::endl << "  " << *vf.first << " : ";
//      print_value_function_trie(os, vf.second);
//    }
//    os << std::endl;
//  }

  size_t get_value_function_size() const {
    return m_q_value_count;

//    size_t size = 0lu;
//    for(const auto &vf : m_value_function) {
//      size += get_trie_size(vf.second);
//    }
//    return size;
  }

  void reset_update_counts() {
//    for(auto &vf : m_value_function) {
//      reset_update_counts_for_trie(vf.second);
//    }
  }

//  void print_value_function_grid(std::ostream &os) const {
//    std::set<line_segment_type> line_segments;
//    for(auto &value : m_value_function) {
//      os << *value.first << ":" << std::endl;
//      const auto line_segments2 = generate_value_function_grid_sets(value.second);
//      merge_value_function_grid_sets(line_segments, line_segments2);
//      print_value_function_grid_set(os, line_segments2);
//    }
//    os << "all:" << std::endl;
//    print_value_function_grid_set(os, line_segments);
//  }

//  void print_update_count_grid(std::ostream &os) const {
//    std::map<line_segment_type, size_t> update_counts;
//    for(auto &value : m_value_function) {
//      os << *value.first << ":" << std::endl;
//      const auto update_counts2 = generate_update_count_maps(value.second);
//      merge_update_count_maps(update_counts, update_counts2);
//      print_update_count_map(os, update_counts2);
//    }
//    os << "all:" << std::endl;
//    print_update_count_map(os, update_counts);
//  }

protected:
  typedef std::pair<double, double> point_type;
  typedef std::pair<point_type, point_type> line_segment_type;

//  Q_Value * get_value(const action_ptrsc &action, const size_t &offset, const size_t &depth = 0) {
//#ifdef ENABLE_WEIGHT
//    const bool use_value = m_weight_assignment_code != "all";
//#else
//    const bool use_value = false;
//#endif
//
//    Q_Value * rv = nullptr; //= get_value_from_function(head, get_trie(action), offset, depth, use_value)->get();
//    if(offset == Q_Value::current_offset()) {
//      assert(*action == *m_current);
//      rv = m_current_q_value->get();
//    }
//    else
//      rv = m_next_q_values[action]->get();
//
//#ifdef ENABLE_WEIGHT
//    assign_weight(Zeni::value_to_Linked_List(rv, offset));
//#endif
//
//    return rv;
//  }

  action_ptrsc choose_epsilon_greedy(const double &epsilon) {
    if(random.frand_lt() < epsilon)
      return choose_randomly();
    else
      return choose_greedy();
  }

  action_ptrsc choose_greedy() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_greedy" << std::endl;
#endif

    double value = double();
    action_ptrsc action;
    for(const auto &action_q : m_next_q_values) {
      const double value_ = sum_value(action_q.first.get(), action_q.second);

      if(!action || value_ > value) {
        action = action_q.first;
        value = value_;
      }
    }

    return action;
  }

  action_ptrsc choose_randomly() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_randomly" << std::endl;
#endif

    int counter = m_next_q_values.size();
//    for(const auto &action_q : m_next_q_values) {
//      ++counter;
//      this->get_value(action_, Q_Value::next_offset()); ///< Trigger additional feature generation, as needed
//    }

    counter = random.rand_lt(counter) + 1;
    action_ptrsc action;
    for(const auto &action_q : m_next_q_values) {
      if(!--counter)
        action = action_q.first;
    }

    return action;
  }

  void td_update(const std::list<tracked_ptr<Q_Value>> &current, const reward_type &reward, const std::list<tracked_ptr<Q_Value>> &next) {
    const double target_next = m_discount_rate * sum_value(nullptr, next);
    const double target_value = reward + target_next;

    double q_old = double();
#ifdef DEBUG_OUTPUT
    std::cerr << " current :";
#endif
    for(const auto &q : current) {
      ++q->update_count;

      if(q->type != Q_Value::Type::FRINGE) {
        q_old += q->value /* * q.weight */;
#ifdef DEBUG_OUTPUT
        std::cerr << ' ' << q;
#endif
      }
    }
#ifdef DEBUG_OUTPUT
    std::cerr << std::endl;
    std::cerr << " fringe  :";
    for(const auto &q : current) {
      if(q->type == Q_Value::Type::FRINGE)
        std::cerr << ' ' << q;
    }
    std::cerr << std::endl;
    std::cerr << " next    :";
    for(const auto &q : next) {
      if(q->type != Q_Value::Type::FRINGE)
        std::cerr << ' ' << q;
    }
    std::cerr << std::endl;
#endif

    m_credit_assignment(current);

    for(const auto &q : current) {
      const double credit = this->m_learning_rate * q->credit;
//       const double credit_accum = credit + (q.eligibility < 0.0 ? 0.0 : q.eligibility);

      if(credit >= q->eligibility) {
        if(q->eligibility < 0.0) {
          q->eligible.erase_hard();
          q->eligible.insert_before(this->m_eligible);
        }

        q->eligibility_init = true;
        q->eligibility = credit;
      }
    }

#ifdef DEBUG_OUTPUT
    double q_new = 0.0;
#endif
    const double delta = target_value - q_old;
#ifdef ENABLE_WEIGHT
    const bool weight_assignment_all = m_weight_assignment_code == "all";
#else
    const bool weight_assignment_all = true;
#endif
    for(Q_Value::List * q_ptr = m_eligible; q_ptr; ) {
      Q_Value &q = **q_ptr;
      q_ptr = q.eligible.next();

      const double ldelta = weight_assignment_all && q.type != Q_Value::Type::FRINGE ? delta : target_value - q.value;
      const double edelta = q.eligibility * ldelta;

      q.value += edelta;
#ifdef DEBUG_OUTPUT
      q_new += q.value /* * q.weight */;
#endif

      if(q.type == Q_Value::Type::SPLIT) {
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        this->m_mean_mabe.uncontribute(q.mabe);
#endif
        if(!m_mean_cabe_queue_size)
          this->m_mean_cabe.uncontribute(q.cabe);
      }
      else {
        const double abs_edelta = std::abs(edelta);

        if(q.eligibility_init) {
          if(q.last_episode_fired != this->m_episode_number) {
            ++q.pseudoepisode_count;
            q.last_episode_fired = this->m_episode_number;
          }
          else if(this->m_step_count - q.last_step_fired > m_pseudoepisode_threshold)
            ++q.pseudoepisode_count;
          q.last_step_fired = this->m_step_count;

#ifdef WHITESON_ADAPTIVE_TILE
          if(q.type == Q_Value::Type::UNSPLIT) {
            if(abs_edelta < q.minbe) {
              q.minbe = abs_edelta;
              this->m_steps_since_minbe = 0;
            }
            else
              ++this->m_steps_since_minbe;
          }
#endif
        }

        q.cabe += abs_edelta;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        q.mabe.set_value(q.cabe / q.update_count);
#endif

        if(q.type == Q_Value::Type::UNSPLIT && q.update_count > m_contribute_update_count) {
          if(m_mean_cabe_queue_size) {
            if(this->m_mean_cabe_queue.size() == m_mean_cabe_queue_size)
              this->m_mean_cabe_queue.pop();
            this->m_mean_cabe_queue.push(q.cabe);
          }
          else
            this->m_mean_cabe.contribute(q.cabe);

#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
          this->m_mean_mabe.contribute(q.mabe);
#endif
        }
      }

      assert(q.eligibility >= 0.0);
      q.eligibility_init = false;
      q.eligibility *= this->m_eligibility_trace_decay_rate;
      if(q.eligibility < m_eligibility_trace_decay_threshold) {
        if(&q.eligible == this->m_eligible)
          this->m_eligible = this->m_eligible->next();
        q.eligible.erase();
        q.eligibility = -1.0;
      }
    }

#ifdef DEBUG_OUTPUT
    std::cerr.unsetf(std::ios_base::floatfield);
    std::cerr << " td_update: " << q_old << " <" << m_learning_rate << "= " << reward << " + " << m_discount_rate << " * " << target_next << std::endl;
    std::cerr << "            " << delta << " = " << target_value << " - " << q_old << std::endl;
    std::cerr << "            " << q_new << std::endl;
    std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);

    for(const auto &q : current) {
      if(q->type == Q_Value::Type::UNSPLIT) {
        std::cerr << " updates:  " << q->update_count << std::endl;
        if(m_mean_cabe_queue_size)
          std::cerr << " cabe q:   " << q->cabe << " of " << this->m_mean_cabe_queue.mean() << ':' << this->m_mean_cabe_queue.mean().get_stddev() << std::endl;
        else
          std::cerr << " cabe:     " << q->cabe << " of " << this->m_mean_cabe << ':' << this->m_mean_cabe.get_stddev() << std::endl;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        std::cerr << " mabe:     " << q->mabe << " of " << this->m_mean_mabe << ':' << this->m_mean_mabe.get_stddev() << std::endl;
#endif
      }
    }
#endif
  }

  void clear_eligibility_trace() {
    for(Q_Value &q : *m_eligible) {
      q.eligibility_init = false;
      q.eligibility = -1.0;
    }

    m_eligible = nullptr;
  }

//#ifdef ENABLE_WEIGHT
//  void assign_weight(Q_Value::List * const &value_list) {
//    m_weight_assignment(value_list);
//
//    for(Q_Value &q : *value_list)
//      q.weight = q.type == Q_Value::Type::FRINGE ? 0.0 : q.credit;
//  }
//#endif

  void assign_credit_epsilon(const std::list<tracked_ptr<Q_Value>> &value_list,
                             void (Agent::*exploration)(const std::list<tracked_ptr<Q_Value>> &),
                             void (Agent::*target)(const std::list<tracked_ptr<Q_Value>> &))
  {
    (this->*exploration)(value_list);

    for(const auto &q : value_list)
      q->t0 = q->credit;

    (this->*target)(value_list);

    const double inverse = 1.0 - this->m_credit_assignment_epsilon;
    for(const auto &q : value_list)
      q->credit = this->m_credit_assignment_epsilon * q->credit + inverse * q->t0;
  }

  void assign_credit_specific(const std::list<tracked_ptr<Q_Value>> &value_list) {
    tracked_ptr<Q_Value> last;
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 0.0;
        last = q;
      }
      else
        q->credit = m_fringe_learning_scale;
    }

    if(last)
      last->credit = 1.0;
  }

  void assign_credit_evenly(const std::list<tracked_ptr<Q_Value>> &value_list) {
    double count = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        ++count;
    }

    for(const auto &q : value_list)
      q->credit = q->type == Q_Value::Type::FRINGE ? m_fringe_learning_scale : 1.0 / count;
  }

  void assign_credit_all(const std::list<tracked_ptr<Q_Value>> &value_list) {
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        q->credit = 1.0;
      else
        q->credit = m_fringe_learning_scale;
    }
  }

  void assign_credit_inv_update_count(const std::list<tracked_ptr<Q_Value>> &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / q->update_count;
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_inv_log_update_count(const std::list<tracked_ptr<Q_Value>> &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / (std::log(double(q->update_count)) / this->m_credit_assignment_log_base_value + 1.0);
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_inv_root_update_count(const std::list<tracked_ptr<Q_Value>> &value_list) {
    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / std::pow(double(q->update_count), this->m_credit_assignment_root_value);
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_inv_depth(const std::list<tracked_ptr<Q_Value>> &value_list) {
    size_t depth = 0;
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE)
        ++depth;
    }

    double sum = double();
    for(const auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
        q->credit = 1.0 / std::pow(2.0, double(--depth));
        sum += q->credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_normalize(const std::list<tracked_ptr<Q_Value>> &value_list, const double &sum) {
    if(m_credit_assignment_normalize || sum > 1.0) {
      for(const auto &q : value_list) {
        if(q->type == Q_Value::Type::FRINGE)
          q->credit = m_fringe_learning_scale;
        else
          q->credit /= sum;
      }
    }
  }

  bool split_test(const tracked_ptr<Q_Value> &q, const size_t &depth) const {
    assert(!q || q->type != Q_Value::Type::FRINGE);

    if(depth < m_split_min) {
      if(q)
        q->type = Q_Value::Type::SPLIT;
      return true;
    }
    if(depth >= m_split_max)
      return false;

    if(!q)
      return false;
    if(q->type == Q_Value::Type::SPLIT)
      return true;

    if(m_value_function_cap && get_value_function_size() >= m_value_function_cap)
      return false;

    if(q->update_count > m_split_update_count &&
       q->pseudoepisode_count > m_split_pseudoepisodes &&
       (m_mean_cabe_queue_size ? m_mean_cabe_queue.mean().outlier_above(q->cabe, m_split_cabe + m_split_cabe_qmult * m_q_value_count)
                               : m_mean_cabe.outlier_above(q->cabe, m_split_cabe + m_split_cabe_qmult * m_q_value_count)))
    {
      q->type = Q_Value::Type::SPLIT;
      return true;
    }
    else
      return false;
  }

  static double sum_value(const action_type * const &
#ifdef DEBUG_OUTPUT
                                                     action
#endif
                                                           , const std::list<tracked_ptr<Q_Value>> &value_list) {
#ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr.unsetf(std::ios_base::floatfield);
      std::cerr << "   sum_value(" << *action << ") = {";
    }
#endif

//    assert((&value_list - static_cast<Q_Value::List *>(nullptr)) > ptrdiff_t(sizeof(Q_Value)));
    if(value_list.empty())
      return double();

    double sum = double();
    for(auto &q : value_list) {
      if(q->type != Q_Value::Type::FRINGE) {
#ifdef DEBUG_OUTPUT
        if(action)
          std::cerr << ' ' << q->value /* * q.weight */;
#endif

        sum += q->value /* * q->weight */;
      }
    }

#ifdef DEBUG_OUTPUT
    if(action) {
      std::cerr << " } = " << sum << std::endl;
      std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
    }
#endif

    return sum;
  }

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

  template <typename ENVIRONMENT>
  bool generate_feature_ranged(const std::shared_ptr<const ENVIRONMENT> &env, feature_list &list, feature_trie &trie, feature_list &tail, const double &value) {
    auto match = trie->find(**tail);

    if(match && (!match->get() || match->get()->type != Q_Value::Type::FRINGE)) {
      const auto &feature = match->get_key();
      const auto midpt = feature->midpt;
      if(env->get_value(feature->axis) < *midpt)
        tail = &(new FEATURE(typename FEATURE::Axis(feature->axis), feature->bound_lower, midpt, feature->depth + 1, false, value, true))->features;
      else
        tail = &(new FEATURE(typename FEATURE::Axis(feature->axis), midpt, feature->bound_upper, feature->depth + 1, true, value, true))->features;
      tail = tail->template insert_in_order<typename FEATURE::List::compare_default>(list, false);
      trie = match->get_deeper();
      return true;
    }

    return false;
  }

  virtual std::set<line_segment_type> generate_value_function_grid_sets(const feature_trie_type * const &) const {
    return std::set<line_segment_type>();
  }

  std::set<line_segment_type> generate_vfgs_for_axes(const feature_trie_type * const &trie, const Feature_Axis &axis_x, const Feature_Axis &axis_y, const line_segment_type &extents = line_segment_type(point_type(), point_type(1.0, 1.0))) const {
    std::set<line_segment_type> line_segments;

    if(trie) {
      for(const feature_trie_type &trie2 : *trie) {
        auto new_extents = extents;
        const auto &key = trie2.get_key();
        if(key->axis == axis_x) {
          new_extents.first.first = *key->bound_lower;
          new_extents.second.first = *key->bound_upper;
        }
        else if(key->axis == axis_y) {
          new_extents.first.second = *key->bound_lower;
          new_extents.second.second = *key->bound_upper;
        }

        if(key->axis == axis_x || key->axis == axis_y) {
          if(new_extents.first.first != extents.first.first)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.first.second), std::make_pair(new_extents.first.first, new_extents.second.second)));
          if(new_extents.first.second != extents.first.second)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.first.second), std::make_pair(new_extents.second.first, new_extents.first.second)));
          if(new_extents.second.first != extents.second.first)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.second.first, new_extents.first.second), std::make_pair(new_extents.second.first, new_extents.second.second)));
          if(new_extents.second.second != extents.second.second)
            line_segments.insert(std::make_pair(std::make_pair(new_extents.first.first, new_extents.second.second), std::make_pair(new_extents.second.first, new_extents.second.second)));
        }

        if(trie2.get_deeper()) {
          const auto line_segments2 = this->generate_vfgs_for_axes(trie2.get_deeper(), axis_x, axis_y, new_extents);
          this->merge_value_function_grid_sets(line_segments, line_segments2);
        }
      }
    }

    return line_segments;
  }

  virtual std::map<line_segment_type, size_t> generate_update_count_maps(const feature_trie_type * const &) const {
    return std::map<line_segment_type, size_t>();
  }

  std::map<line_segment_type, size_t> generate_ucm_for_axes(const feature_trie_type * const &trie, const Feature_Axis &axis_x, const Feature_Axis &axis_y, const line_segment_type &extents = line_segment_type(point_type(), point_type(1.0, 1.0)), const size_t &update_count = 0) const {
    std::map<line_segment_type, size_t> update_counts;

    if(trie) {
      for(const feature_trie_type &trie2 : *trie) {
        auto new_extents = extents;
        const auto &key = trie2.get_key();
        if(key->axis == axis_x) {
          new_extents.first.first = *key->bound_lower;
          new_extents.second.first = *key->bound_upper;
        }
        else if(key->axis == axis_y) {
          new_extents.first.second = *key->bound_lower;
          new_extents.second.second = *key->bound_upper;
        }

        const auto update_count2 = update_count + (trie2.get() ? trie2->update_count : 0);

        update_counts[new_extents] = update_count2;

        if(trie2.get_deeper()) {
          const auto update_counts2 = this->generate_ucm_for_axes(trie2.get_deeper(), axis_x, axis_y, new_extents, update_count2);
          this->merge_update_count_maps(update_counts, update_counts2);
        }
      }
    }

    return update_counts;
  }

  void print_value_function_grid_set(std::ostream &os, const std::set<line_segment_type> &line_segments) const {
    for(const line_segment_type &line_segment : line_segments)
      os << line_segment.first.first << ',' << line_segment.first.second << '/' << line_segment.second.first << ',' << line_segment.second.second << std::endl;
  }

  void print_update_count_map(std::ostream &os, const std::map<line_segment_type, size_t> &update_counts) const {
    for(const auto &rect : update_counts)
      os << rect.first.first.first << ',' << rect.first.first.second << '/' << rect.first.second.first << ',' << rect.first.second.second << '=' << rect.second << std::endl;
  }

  void merge_value_function_grid_sets(std::set<line_segment_type> &combination, const std::set<line_segment_type> &additions) const {
    for(const line_segment_type &line_segment : additions)
      combination.insert(line_segment);
  }

  void merge_update_count_maps(std::map<line_segment_type, size_t> &combination, const std::map<line_segment_type, size_t> &additions) const {
    for(const auto &rect : additions)
      combination[rect.first] += rect.second;
  }

  Metastate m_metastate = Metastate::NON_TERMINAL;
  action_ptrsc m_current;
  std::list<tracked_ptr<Q_Value>> m_current_q_value;
  action_ptrsc m_next;
  std::map<action_ptrsc, std::list<tracked_ptr<Q_Value>>, typename action_type::Compare> m_next_q_values;
  std::function<action_ptrsc ()> m_target_policy; ///< Sarsa/Q-Learning selector
  std::function<action_ptrsc ()> m_exploration_policy; ///< Exploration policy
  std::function<bool (Q_Value * const &, const size_t &)> m_split_test; ///< true if too general, false if sufficiently general
  size_t m_q_value_count = 0;

private:
  static size_t get_trie_size(const feature_trie_type * const &trie) {
    size_t size = 0lu;
    for(const feature_trie_type &trie2 : *trie) {
      if(trie2.get())
        ++size;
      size += get_trie_size(trie2.get_deeper());
    }
    return size;
  }

  static void reset_update_counts_for_trie(const feature_trie_type * const &trie) {
    for(const feature_trie_type &trie2 : *trie) {
      if(trie2.get())
        trie2.get()->update_count = 0;
      reset_update_counts_for_trie(trie2.get_deeper());
    }
  }

//  feature_trie get_value_from_function(const feature_trie &head, feature_trie &function, const size_t &offset, const size_t &depth, const bool &use_value, const double &value = 0.0) {
//    /** Begin logic to ensure that features enter the trie in the same order, regardless of current ordering. **/
//    feature_trie match, found;
//    for(match = head; match; match = match->list_next()) {
//      found = function->find(match->get_key(), typename feature_type::Compare_Axis());
//      if(found)
//        break;
//    }
//
//    if(match) {
//      assert(!found->get() || found->get()->type != Q_Value::Type::FRINGE);
//
//      /// Dynamic midpoint part 1 of 3
//      const feature_trie inserted = match;
//      Feature_Ranged_Data * ranged = dynamic_cast<Feature_Ranged_Data *>(inserted->get_key().get());
//      double inserted_midpt;
//      if(ranged)
//        inserted_midpt = ranged->midpt_raw;
//
//      /// Insertion
//      feature_trie next = static_cast<feature_trie>(head == match ? head->list_next() : head);
//      match->list_erase();
//      match = match->map_insert_into_unique(function);
//      if(!m_null_q_values && !match->get()) {
//        match->get() = new Q_Value(use_value ? value : 0.0);
//        ++m_q_value_count;
//      }
//
//      /// Dynamic midpoint part 2 of 3
////      if(m_dynamic_midpoint && ranged && inserted == match) {
////        Feature_Ranged_Data * const ranged_other = dynamic_cast<Feature_Ranged_Data *>(found->get_key().get());
////        if(ranged->upper)
////          ranged->bound_lower = ranged_other->bound_upper;
////        else
////          ranged->bound_upper = ranged_other->bound_lower;
////      }
//
//      const double value_next = value + (match->get() ? match->get()->value * match->get()->weight : 0.0);
//
//      feature_trie deeper = nullptr;
//      if(next && m_split_test(match->get(), depth)) {
//        /// Recursion
//        collapse_fringe(match->get_deeper(), next);
//        deeper = get_value_from_function(next, match->get_deeper(), offset, depth + 1, use_value, value_next);
//      }
//      else {
//        /// Done; repeat as needed
//        if(!found->get() || found->get()->type != Q_Value::Type::FRINGE) {
//          try {
//            generate_more_features(match->get(), depth, match == inserted);
//          }
//          catch(Again &) {
//            next->list_destroy(next);
//            throw;
//          }
//
//          if(m_null_q_values && !match->get()) {
//            match->get() = new Q_Value(use_value ? value : 0.0);
//            ++m_q_value_count;
//          }
//
//          deeper = generate_fringe(match->get_deeper(), next, offset, value_next);
//        }
//        else
//          next->list_destroy(next);
//      }
//
//      /// Dynamic midpoint part 3 of 3
//      if(m_dynamic_midpoint && offset == Q_Value::current_offset() && ranged && inserted != match) {
//        ranged = dynamic_cast<Feature_Ranged_Data *>(match->get_key().get());
//        const size_t next_update_count = ranged->midpt_update_count + 1;
//        const double next_update_count_d = double(next_update_count);
//
//        assert(ranged->midpt_raw >= 0.0);
//        assert(ranged->midpt_raw <= 1.0);
//        assert(inserted_midpt >= 0.0);
//        assert(inserted_midpt <= 1.0);
//        ranged->midpt_raw = ranged->midpt_raw * (ranged->midpt_update_count / next_update_count_d) + inserted_midpt / next_update_count_d;
//
//        ranged->midpt_update_count = next_update_count;
//
////        if(!deeper || deeper->get()->type == Q_Value::Type::FRINGE) {
////          double ranged_amt;
////          if(ranged->midpt_update_count < 10) { ///< 10 is a temporary magic number
////            ranged_amt = ranged->midpt_update_count / 10.0;
////            ranged_amt = ranged->midpt_raw * ranged_amt + 0.5 * (1.0 - ranged_amt);
////          }
////          else
////            ranged_amt = ranged->midpt_raw;
////          *ranged->midpt = *ranged->bound_lower + ranged_amt * (*ranged->bound_upper - *ranged->bound_lower);
////
////          assert(*ranged->midpt <= *ranged->bound_upper);
////          assert(*ranged->midpt >= *ranged->bound_lower);
////        }
//      }
//
//      /// Stitch together Q-Values and return
//      match->offset_erase_hard(offset);
//      auto rv = match->offset_insert_before(offset, deeper);
//      assert(rv);
//      return rv;
//    }
//    /** End logic to ensure that features enter the trie in the same order, regardless of current ordering. **/
//
//    auto rv = head->insert(function, m_null_q_values, m_split_test,
//                           [this](Q_Value * const &q, const size_t &depth, const bool &force){this->generate_more_features(q, depth, force);},
//                           [this](feature_trie &leaf_fringe, feature_trie head, const size_t &offset, const double &value)->feature_trie{return this->generate_fringe(leaf_fringe, head, offset, value);},
//                           [this](feature_trie &leaf_fringe, feature_trie head){this->collapse_fringe(leaf_fringe, head);},
//                           offset, depth, value, use_value, m_q_value_count);
//    assert(rv);
//    return rv;
//  }

//  void generate_more_features(Q_Value * const &q, const size_t &depth, const bool &force) {
//    if(!m_features_complete && (force || m_split_test(q, depth)))
//    {
//      destroy_features();
//      generate_features();
//      throw Again();
//    }
//  }

//  /// Use up the rest of the features to generate a fringe
//  feature_trie generate_fringe(feature_trie &leaf_fringe, feature_trie head, const size_t &offset, const double &value) {
//    if(m_fringe) {
//      assert(!leaf_fringe || !leaf_fringe->get() || leaf_fringe->get()->type == Q_Value::Type::FRINGE);
//
////  #ifdef DEBUG_OUTPUT
////      std::cerr.unsetf(std::ios_base::floatfield);
////      std::cerr << "   Current fringe:" << std::endl;
////  #endif
//
//      feature_trie fringe_head = nullptr;
//      while(head) {
//        auto next = static_cast<feature_trie>(head->list_next());
//        head->list_erase_hard();
//
//        auto inserted = head->map_insert_into_unique(leaf_fringe);
//        if(!inserted->get())
//          inserted->get() = new Q_Value(value, Q_Value::Type::FRINGE);
//
////  #ifdef DEBUG_OUTPUT
////        std::cerr << "    " << inserted->get() << " from " << *inserted->get_key() << std::endl;
////  #endif
//
//        inserted->offset_erase_hard(offset);
//        fringe_head = inserted->offset_insert_before(offset, fringe_head);
//
//        head = next;
//      }
//
////  #ifdef DEBUG_OUTPUT
////      leaf_fringe->map_debug_print_visit(std::cerr, [](const Q_Value * const q){std::cerr << q;});
////      std::cerr << std::endl;
////
////      for(feature_trie_type &node : *leaf_fringe) {
////        std::cerr << "    " << node.get() << ' ' << *node.get_key() << " = " << node->value;
////        std::cerr << "; update_count = " << node->update_count;
////        std::cerr << "; cabe = " << node->cabe;
////        std::cerr << "; mabe = " << node->cabe / node->update_count;
////        if(auto ranged = dynamic_cast<Feature_Ranged_Data *>(node.get_key().get()))
////          std::cerr << "; split = " << fabs(ranged->midpt_raw - 0.5);
////        std::cerr << std::endl;
////      }
////      std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
////  #endif
//
//      return fringe_head;
//    }
//    else {
//      head->list_destroy(head); ///< Destroy the rest of the features instead of generating a fringe
//      return nullptr;
//    }
//  }

//  void collapse_fringe(feature_trie &leaf_fringe, feature_trie head) {
//    if(m_fringe) {
//      purge_from_eligibility_trace(leaf_fringe);
//
//      feature_trie choice = nullptr;
//
//      if(leaf_fringe && leaf_fringe->get() && leaf_fringe->get()->type == Q_Value::Type::FRINGE) {
//#ifdef DEBUG_OUTPUT
//        std::cerr.unsetf(std::ios_base::floatfield);
//        std::cerr << " Collapsing fringe:" << std::endl;
//        for(feature_trie_type &node : *leaf_fringe) {
//          std::cerr << "         " << node.get() << ' ' << *node.get_key() << " = " << node->value;
//          std::cerr << "; update_count = " << node->update_count;
//          std::cerr << "; cabe = " << node->cabe;
//          std::cerr << "; mabe = " << node->cabe / node->update_count;
//          if(auto ranged = dynamic_cast<Feature_Ranged_Data *>(node.get_key().get()))
//            std::cerr << "; split = " << fabs(ranged->midpt_raw - 0.5);
//          std::cerr << std::endl;
//        }
//#endif
//
//        size_t min_depth = std::numeric_limits<size_t>::max();
//        for(feature_trie_type &node : *leaf_fringe) {
//          const auto feature = node.get_key().get();
//          const auto ranged = dynamic_cast<Feature_Ranged_Data *>(feature);
//          if(ranged->depth < min_depth)
//            min_depth = ranged->depth;
//        }
//
//        feature_trie_type * prev = nullptr;
//        Feature_Ranged_Data * ranged_prev = nullptr;
////        size_t choice_update_count_min = 0u;
////        size_t ranged_update_count_min = 0u;
////        size_t choice_axis_in_a_row = 0u;
////        size_t axis_in_a_row = 0u;
//        double choice_delta = 0.0;
////        size_t choice_depth = 0;
//        for(feature_trie_type &node : *leaf_fringe) {
//          const auto feature = node.get_key().get();
//          const auto ranged = dynamic_cast<Feature_Ranged_Data *>(feature);
//
////          if(!choice) {
////            choice = &node;
////            choice_update_count_min = ranged ? ranged->midpt_update_count : 0u;
////            choice_axis_in_a_row = ranged ? 1u : 0u;
////          }
//
//          if(ranged_prev && ranged_prev->depth == min_depth) {
//            if(ranged && ranged_prev && ranged->axis == ranged_prev->axis &&
//               ranged->midpt_update_count && ranged_prev->midpt_update_count)
//            {
//              const double delta = fabs(prev->get()->value - node.get()->value);
//              if(delta > choice_delta) {
//                choice_delta = delta;
////                choice_depth = ranged->depth;
//                choice = &node;
//              }
//            }
//
////            if(!ranged || ranged->axis != ranged_prev->axis) {
////              if(axis_in_a_row >  choice_axis_in_a_row ||
////                (axis_in_a_row == choice_axis_in_a_row && ranged_update_count_min > choice_update_count_min)) {
////                choice = prev;
////                choice_update_count_min = ranged_update_count_min;
////              }
////
////              if(ranged) {
////                ranged_update_count_min = ranged->midpt_update_count;
////                axis_in_a_row = 1u;
////              }
////            }
////            else {
////              ranged_update_count_min = std::min(ranged_update_count_min, ranged->midpt_update_count);
////              ++axis_in_a_row;
////            }
//          }
////          else if(ranged) {
////            ranged_update_count_min = ranged->midpt_update_count;
////            axis_in_a_row = 1u;
////          }
//
//          prev = &node;
//          ranged_prev = ranged;
//        }
////        if(axis_in_a_row >  choice_axis_in_a_row ||
////          (axis_in_a_row == choice_axis_in_a_row && ranged_update_count_min > choice_update_count_min)) {
////          choice = prev;
////          choice_update_count_min = ranged_update_count_min;
////        }
//
//#ifdef DEBUG_OUTPUT
//        std::cerr << "  Choice " << *choice->get_key() << std::endl;
//        std::cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
//#endif
//
//        if(choice)
//          choice = new feature_trie_type(std::unique_ptr<feature_type>(choice->get_key()->clone()));
//
//        leaf_fringe->destroy(leaf_fringe);
//
//        if(choice)
//          choice->map_insert_into_unique(leaf_fringe);
//      }
//    }
//    /// Only necessary when lesioning codebase for debugging purposes (i.e. disabling previous block of code)
//    else if(leaf_fringe && leaf_fringe->get() && leaf_fringe->get()->type == Q_Value::Type::FRINGE) {
//      purge_from_eligibility_trace(leaf_fringe);
//      leaf_fringe->destroy(leaf_fringe);
//    }
//  }

//  void purge_from_eligibility_trace(const feature_trie leaf_fringe) {
//    for(feature_trie_type &node : *leaf_fringe) {
//      if(Q_Value * const q = node.get()) {
//        if(&q->eligible == m_eligible && m_eligible->next())
//          m_eligible = m_eligible->next();
//
//        q->eligibility_init = false;
//        q->eligibility = -1.0;
//
//        q->eligible.erase();
//      }
//    }
//  }

//  static void print_value_function_trie(std::ostream &os, const feature_trie_type * const &trie) {
//    for(const auto &tt : *trie) {
//      os << '<' << *tt.get_key() << ',';
//      if(tt.get())
//        os << tt->value;
//      else
//        os << "nullptr";
//      os << ',';
//      if(tt.get_deeper())
//        print_value_function_trie(os, tt.get_deeper());
//      else if(tt.get() && tt.get()->type == Q_Value::Type::FRINGE)
//        os << 'f';
//      os << '>';
//    }
//  }

  virtual void generate_features() = 0;
  virtual void update() = 0;

  Mean m_mean_cabe;
  Value_Queue m_mean_cabe_queue;
  const size_t m_mean_cabe_queue_size = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["mean-cabe-queue-size"]).get_value();

#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
  Mean m_mean_mabe;
#endif

#ifdef WHITESON_ADAPTIVE_TILE
  size_t m_steps_since_minbe = 0;
#endif

  std::shared_ptr<environment_type> m_environment;

  Zeni::Random random;

  size_t m_episode_number = 1;
  size_t m_step_count = 0;
  reward_type m_total_reward = 0.0;

  const bool m_null_q_values = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["null-q-values"]).get_value(); ///< insert nullptr instead of new Q_Values until reaching the leaf
  const size_t m_value_function_cap = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["value-function-cap"]).get_value(); ///< at this threshold, no more entries will be added to the value functions through refinement

  const double m_learning_rate = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["learning-rate"]).get_value(); ///< alpha
  const double m_discount_rate = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["discount-rate"]).get_value(); ///< gamma
  const double m_eligibility_trace_decay_rate = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["eligibility-trace-decay-rate"]).get_value(); ///< lambda
  const double m_eligibility_trace_decay_threshold = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["eligibility-trace-decay-threshold"]).get_value();

  const std::string m_credit_assignment_code = dynamic_cast<const Option_Itemized &>(Options::get_global()["credit-assignment"]).get_value();
  const std::function<void (const std::list<tracked_ptr<Q_Value>> &)> m_credit_assignment; ///< How to assign credit to multiple Q-values
  const double m_credit_assignment_epsilon = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["credit-assignment-epsilon"]).get_value();
  const double m_credit_assignment_log_base = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["credit-assignment-log-base"]).get_value();
  const double m_credit_assignment_log_base_value = std::log(m_credit_assignment_log_base);
  const double m_credit_assignment_root = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["credit-assignment-root"]).get_value();
  const double m_credit_assignment_root_value = 1.0 / m_credit_assignment_root;
  const bool m_credit_assignment_normalize = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["credit-assignment-normalize"]).get_value();

//#ifdef ENABLE_WEIGHT
//  const std::string m_weight_assignment_code = dynamic_cast<const Option_Itemized &>(Options::get_global()["weight-assignment"]).get_value();
//  const std::function<void (Q_Value::List * const &)> m_weight_assignment; ///< How to assign weight to multiple Q-values at summation time
//#endif

  const bool m_on_policy = dynamic_cast<const Option_Itemized &>(Options::get_global()["policy"]).get_value() == "on-policy"; ///< for Sarsa/Q-learning selection
  const double m_epsilon = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["epsilon-greedy"]).get_value(); ///< for epsilon-greedy decision-making

  const size_t m_split_min = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["split-min"]).get_value();
  const size_t m_split_max = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["split-max"]).get_value();
  const double m_split_cabe = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["split-cabe"]).get_value();
  const double m_split_cabe_qmult = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["split-cabe-qmult"]).get_value();

  const size_t m_pseudoepisode_threshold = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["pseudoepisode-threshold"]).get_value(); ///< For deciding how many steps indicates a pseudoepisode
  const size_t m_split_pseudoepisodes = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["split-pseudoepisodes"]).get_value();
  const size_t m_split_update_count = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["split-update-count"]).get_value();

  const size_t m_contribute_update_count = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["contribute-update-count"]).get_value();
  const bool m_dynamic_midpoint = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["dynamic-midpoint"]).get_value();
  const bool m_fringe = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["fringe"]).get_value();
  const double m_fringe_learning_scale = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["fringe-learning-scale"]).get_value();

  Q_Value::List * m_eligible = nullptr;
};

template <typename FEATURE, typename ACTION>
std::ostream & operator << (std::ostream &os, const Agent<FEATURE, ACTION> &agent) {
  agent.print(os);
  return os;
}

#endif
