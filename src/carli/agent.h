#ifndef AGENT_H
#define AGENT_H

#include "clone.h"
#include "environment.h"
#include "trie.h"
#include "q_value.h"
#include "random.h"
#include "value_queue.h"

#include <functional>
#include <map>
#include <set>
#include <stack>
#include <string>

#include <iostream>

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature : public Zeni::Pool_Allocator<DERIVED2>, public Zeni::Cloneable<DERIVED> {
  Feature & operator=(const Feature &) = delete;

public:
  typedef typename Zeni::Linked_List<DERIVED> List;
  typedef typename List::iterator iterator;

  struct Compare {
    bool operator()(const Feature &lhs, const Feature &rhs) const {
      return lhs.compare(rhs) < 0;
    }

    bool operator()(const Feature * const &lhs, const Feature * const &rhs) const {
      return operator()(*lhs, *rhs);
    }

    bool operator()(const std::shared_ptr<const Feature> &lhs, const std::shared_ptr<const Feature> &rhs) const {
      return operator()(*lhs, *rhs);
    }

    bool operator()(const std::unique_ptr<const Feature> &lhs, const std::unique_ptr<const Feature> &rhs) const {
      return operator()(*lhs, *rhs);
    }
  };

  struct Compare_PI {
    bool operator()(const Feature &lhs, const Feature &rhs) const {
      return lhs.compare_pi(rhs) < 0;
    }

    bool operator()(const Feature * const &lhs, const Feature * const &rhs) const {
      return operator()(*lhs, *rhs);
    }

    bool operator()(const std::shared_ptr<const Feature> &lhs, const std::shared_ptr<const Feature> &rhs) const {
      return operator()(*lhs, *rhs);
    }

    bool operator()(const std::unique_ptr<const Feature> &lhs, const std::unique_ptr<const Feature> &rhs) const {
      return operator()(*lhs, *rhs);
    }
  };

  Feature(const bool &present_ = true)
    : features(static_cast<DERIVED *>(this)),
    present(present_)
  {
  }

  Feature(const Feature &rhs)
   : features(static_cast<DERIVED *>(this)),
   present(rhs.present)
  {
  }

  virtual ~Feature() {}

  bool operator<(const Feature &rhs) const {return compare(rhs) < 0;}
  bool operator<=(const Feature &rhs) const {return compare(rhs) <= 0;}
  bool operator>(const Feature &rhs) const {return compare(rhs) > 0;}
  bool operator>=(const Feature &rhs) const {return compare(rhs) >= 0;}
  bool operator==(const Feature &rhs) const {return compare(rhs) == 0;}
  bool operator!=(const Feature &rhs) const {return compare(rhs) != 0;}

  void print(std::ostream &os) const {
    if(!present)
      os << '!';
    print_impl(os);
  }

  int compare(const Feature &rhs) const {
    return present ^ rhs.present ? rhs.present - present : compare_pi(rhs);
  }

  int compare_pi(const Feature &rhs) const {
    return compare_pi(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual int compare_pi(const DERIVED &rhs) const = 0;

  virtual bool precedes(const DERIVED &rhs) const = 0;

  virtual void print_impl(std::ostream &os) const = 0;

  List features;

  bool present;
};

template <typename DERIVED, typename DERIVED2>
std::ostream & operator << (std::ostream &os, const Feature<DERIVED, DERIVED2> &feature) {
  feature.print(os);
  return os;
}

enum class Credit_Assignment : char {ALL, SPECIFIC, EVEN, INV_UPDATE_COUNT, INV_LOG_UPDATE_COUNT, INV_ROOT_UPDATE_COUNT, INV_DEPTH, EPSILON_EVEN_SPECIFIC, EPSILON_EVEN_DEPTH};
enum class Metastate : char {NON_TERMINAL, SUCCESS, FAILURE};

template <typename FEATURE, typename ACTION>
class Agent : public std::enable_shared_from_this<Agent<FEATURE, ACTION>> {
  Agent(const Agent &) = delete;
  Agent & operator=(const Agent &) = delete;

  class Again {};

public:
  typedef FEATURE feature_type;
  typedef typename FEATURE::List * feature_list;
  typedef ACTION action_type;
  typedef typename ACTION::List * action_list;
  typedef std::unique_ptr<const action_type> action_ptruc;
  typedef Zeni::Trie<std::shared_ptr<feature_type>, Q_Value, typename feature_type::Compare> feature_trie_type;
  typedef feature_trie_type * feature_trie;
  typedef Environment<action_type> environment_type;
  typedef std::map<action_type *, feature_trie, typename action_type::Compare> value_function_type;
  typedef double reward_type;

  Agent(const std::shared_ptr<environment_type> &environment)
    : m_target_policy([this]()->action_ptruc{return this->choose_greedy();}),
    m_exploration_policy([this]()->action_ptruc{return this->choose_epsilon_greedy(m_epsilon);}),
    m_split_test([this](Q_Value * const &q, const size_t &depth)->bool{return this->split_test(q, depth);}),
    m_environment(environment),
    m_credit_assignment(
      m_credit_assignment_code == "all" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_all(value_list);} :
      m_credit_assignment_code == "specific" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_specific(value_list);} :
      m_credit_assignment_code == "even" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_evenly(value_list);} :
      m_credit_assignment_code == "inv-update-count" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_update_count(value_list);} :
      m_credit_assignment_code == "inv-log-update-count" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_log_update_count(value_list);} :
      m_credit_assignment_code == "inv-root-update-count" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_root_update_count(value_list);} :
      m_credit_assignment_code == "inv-depth" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_depth(value_list);} :
      m_credit_assignment_code == "epsilon-even-specific" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_specific);} :
      m_credit_assignment_code == "epsilon-even-depth" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_inv_depth);} :
      std::function<void (Q_Value::List * const &)>()
    ),
    m_weight_assignment(
      m_weight_assignment_code == "all" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_all(value_list);} :
      m_weight_assignment_code == "specific" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_specific(value_list);} :
      m_weight_assignment_code == "even" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_evenly(value_list);} :
      m_weight_assignment_code == "inv-update-count" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_update_count(value_list);} :
      m_weight_assignment_code == "inv-log-update-count" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_log_update_count(value_list);} :
      m_weight_assignment_code == "inv-root-update-count" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_root_update_count(value_list);} :
      m_weight_assignment_code == "inv-depth" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_inv_depth(value_list);} :
      m_weight_assignment_code == "epsilon-even-specific" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_specific);} :
      m_weight_assignment_code == "epsilon-even-depth" ?
        [this](Q_Value::List * const &value_list){return this->assign_credit_epsilon(value_list, &Agent<FEATURE, ACTION>::assign_credit_evenly, &Agent<FEATURE, ACTION>::assign_credit_inv_depth);} :
      std::function<void (Q_Value::List * const &)>()
    )
  {
  }

  virtual ~Agent() {
    destroy_lists();

    for(typename value_function_type::iterator vt = m_value_function.begin(), vend = m_value_function.end(); vt != vend; ) {
      auto ptr = vt->first;
      vt->second->destroy(vt->second);
      m_value_function.erase(vt++);
      delete ptr;
    }
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
  Credit_Assignment get_weight_assignment() const {return m_weight_assignment_code;}
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

    regenerate_lists();

    if(m_metastate == Metastate::NON_TERMINAL)
      m_next = m_exploration_policy();
  }

  reward_type act() {
    m_current = std::move(m_next);

    Q_Value * const value_current = get_value(m_features, *m_current, Q_Value::current_offset());

    const reward_type reward = m_environment->transition(*m_current);

    update();

    if(m_metastate == Metastate::NON_TERMINAL) {
      regenerate_lists();

      m_next = m_target_policy();
#ifdef DEBUG_OUTPUT
      std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      Q_Value * const value_best = get_value(m_features, *m_next, Q_Value::next_offset());
      td_update(&value_current->current, reward, &value_best->next);

      if(!m_on_policy) {
        std::unique_ptr<const action_type> next = m_exploration_policy();

        if(*m_next != *next) {
          if(sum_value(nullptr, get_value(m_features, *next, Q_Value::current_offset())->current) < sum_value(nullptr, value_best->next))
            clear_eligibility_trace();
          m_next = std::move(next);
        }

#ifdef DEBUG_OUTPUT
        std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      }
    }
    else {
      destroy_lists();

      td_update(&value_current->current, reward, nullptr);
    }

    m_total_reward += reward;
    ++m_step_count;

    return reward;
  }

  void print(std::ostream &os) const {
    os << " Agent:\n";
    print_list(os, "  Features:\n  ", " ", m_features);
    print_list(os, "  Candidates:\n  ", " ", m_candidates);
#if defined(DEBUG_OUTPUT) && defined(DEBUG_OUTPUT_VALUE_FUNCTION)
    print_value_function(os);
#endif
  }

  void print_value_function(std::ostream &os) const {
    os << "  Value Function:";
    for(auto &vf : m_value_function) {
      os << std::endl << "  " << vf.first << " : ";
      print_value_function_trie(os, vf.second);
    }
    os << std::endl;
  }

  size_t get_value_function_size() const {
    return m_q_value_count;

//     size_t size = 0lu;
//     for(const auto &vf : m_value_function) {
//       size += get_trie_size(vf.second);
//     }
//     return size;
  }

  void reset_update_counts() {
    for(auto &vf : m_value_function) {
      reset_update_counts_for_trie(vf.second);
    }
  }

protected:
  Q_Value * get_value(const feature_list &features, const action_type &action, const size_t &offset, const size_t &depth = 0) {
    if(!features)
      return nullptr;

    for(;;) {
      try {
        feature_trie head = nullptr;

        auto it = features->begin();
        auto iend = features->end();
        if(it != iend) {
          head = new feature_trie_type(std::shared_ptr<feature_type>(it->clone()));
          auto tail = head;
          ++it;

          while(it != iend) {
            auto ptr = new feature_trie_type(std::shared_ptr<feature_type>(it->clone()));
            assert(it->present == ptr->get_key()->present);
            ptr->list_insert_after(tail);
            tail = ptr;
            ++it;
          }
        }

        Q_Value * const rv = get_value_from_function(head, get_trie(action), offset, depth)->get();

        assign_weight(value_to_Linked_List(rv, offset));

        return rv;
      }
      catch(Again &) {
      }
    }
  }

  feature_trie & get_trie(const action_type &action) {
    auto vf = m_value_function.find(const_cast<action_type *>(&action));
    if(vf == m_value_function.end())
      vf = m_value_function.insert(typename value_function_type::value_type(action.clone(), nullptr)).first;
    return vf->second;
  }

  void regenerate_lists() {
    destroy_lists();

    generate_features();
    generate_candidates();
  }

  void destroy_lists() {
    m_features->destroy(m_features);
    m_candidates->destroy(m_candidates);
  }

  action_ptruc choose_epsilon_greedy(const double &epsilon) {
    if(random.frand_lt() < epsilon)
      return choose_randomly();
    else
      return choose_greedy();
  }

  action_ptruc choose_greedy() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_greedy" << std::endl;
#endif

    double value = double();
    const action_type * action = nullptr;
    for(const action_type &action_ : *m_candidates) {
      const double value_ = sum_value(&action_, this->get_value(m_features, action_, Q_Value::next_offset())->next);

      if(!action || value_ > value) {
        action = &action_;
        value = value_;
      }
    }

    return action_ptruc(action->clone());
  }

  action_ptruc choose_randomly() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_randomly" << std::endl;
#endif

    int counter = 0;
    for(const action_type &action_ : *m_candidates) {
      ++counter;
      this->get_value(m_features, action_, Q_Value::next_offset()); ///< Trigger additional feature generation, as needed
    }

    counter = random.rand_lt(counter) + 1;
    const action_type * action = nullptr;
    for(const action_type &action_ : *m_candidates) {
      if(!--counter)
        action = &action_;
    }

    return action_ptruc(action->clone());
  }

  void td_update(Q_Value::List * const &current, const reward_type &reward, const Q_Value::List * const &next) {
    if(!current)
      return;

    const double target_next = next ? m_discount_rate * sum_value(nullptr, *next) : 0.0;
    const double target_value = reward + target_next;

    double q_old = double();
#ifdef DEBUG_OUTPUT
    std::cerr << " current :";
#endif
    for(Q_Value &q : *current) {
      if(q.type != Q_Value::Type::FRINGE) {
        q_old += q.value * q.weight;
#ifdef DEBUG_OUTPUT
        std::cerr << ' ' << &q;
#endif
      }
    }
#ifdef DEBUG_OUTPUT
    std::cerr << std::endl;
    std::cerr << " next    :";
    for(const Q_Value &q : *next) {
      if(q.type != Q_Value::Type::FRINGE)
        std::cerr << ' ' << &q;
    }
    std::cerr << std::endl;
#endif

    m_credit_assignment(current);

    for(Q_Value &q : *current) {
      ++q.update_count;

      const double credit = this->m_learning_rate * q.credit;
//       const double credit_accum = credit + (q.eligibility < 0.0 ? 0.0 : q.eligibility);

      if(credit >= q.eligibility) {
        if(q.eligibility < 0.0) {
          q.eligible.erase();
          q.eligible.insert_before(this->m_eligible);
        }

        q.eligibility_init = true;
        q.eligibility = credit;
      }
    }

#ifdef DEBUG_OUTPUT
    double q_new = 0.0;
#endif
    const double delta = target_value - q_old;
    for(Q_Value::List * q_ptr = m_eligible; q_ptr; ) {
      Q_Value &q = **q_ptr;
      q_ptr = q.eligible.next();

      const double ldelta = m_weight_assignment_code != "all" ? target_value - q.value : delta;
      const double edelta = q.eligibility * ldelta;

      q.value += edelta;
#ifdef DEBUG_OUTPUT
      q_new += q.value * q.weight;
#endif

      if(q.type == Q_Value::Type::SPLIT) {
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        this->m_mean_mabe.uncontribute(q.mabe);
#endif
        if(!m_mean_cabe_queue_size)
          this->m_mean_cabe.uncontribute(q.cabe);
      }
      else if(q.type == Q_Value::Type::UNSPLIT) {
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
          if(abs_edelta < q.minbe) {
            q.minbe = abs_edelta;
            this->m_steps_since_minbe = 0;
          }
          else
            ++this->m_steps_since_minbe;
#endif
        }

        q.cabe += abs_edelta;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        q.mabe = q.cabe / q.update_count;
#endif
        if(q.update_count > m_contribute_update_count) {
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
    std::cerr << " td_update: " << q_old << " <" << m_learning_rate << "= " << reward << " + " << m_discount_rate << " * " << target_next << std::endl;
    std::cerr << "            " << delta << " = " << target_value << " - " << q_old << std::endl;
    std::cerr << "            " << q_new << std::endl;

    for(Q_Value &q : *current) {
      if(q.type == Q_Value::Type::UNSPLIT) {
        std::cerr << " updates:  " << q.update_count << std::endl;
        if(m_mean_cabe_queue_size)
          std::cerr << " cabe q:   " << q.cabe << " of " << this->m_mean_cabe_queue.mean() << ':' << this->m_mean_cabe_queue.mean().get_stddev() << std::endl;
        else
          std::cerr << " cabe:     " << q.cabe << " of " << this->m_mean_cabe << ':' << this->m_mean_cabe.get_stddev() << std::endl;
#ifdef TRACK_MEAN_ABSOLUTE_BELLMAN_ERROR
        std::cerr << " mabe:     " << q.mabe << " of " << this->m_mean_mabe << ':' << this->m_mean_mabe.get_stddev() << std::endl;
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

  void assign_weight(Q_Value::List * const &value_list) {
    m_weight_assignment(value_list);

    for(Q_Value &q : *value_list)
      q.weight = q.type == Q_Value::Type::FRINGE ? 0.0 : q.credit;
  }

  void assign_credit_epsilon(Q_Value::List * const &value_list,
                             void (Agent::*exploration)(Q_Value::List * const &),
                             void (Agent::*target)(Q_Value::List * const &))
  {
    (this->*exploration)(value_list);

    for(Q_Value &q : *value_list)
      q.t0 = q.credit;

    (this->*target)(value_list);

    const double inverse = 1.0 - this->m_credit_assignment_epsilon;
    for(Q_Value &q : *value_list)
      q.credit = this->m_credit_assignment_epsilon * q.credit + inverse * q.t0;
  }

  void assign_credit_specific(Q_Value::List * const &value_list) {
    Q_Value * last = nullptr;
    for(Q_Value &q : *value_list) {
      q.credit = q.type == Q_Value::Type::FRINGE ? 1.0 : 0.0;
      last = &q;
    }

    if(last)
      last->credit = 1.0;
  }

  void assign_credit_evenly(Q_Value::List * const &value_list) {
    double count = double();
    for(Q_Value &q : *value_list) {
      if(q.type != Q_Value::Type::FRINGE)
        ++count;
    }

    for(Q_Value &q : *value_list)
      q.credit = q.type == Q_Value::Type::FRINGE ? 1.0 : 1.0 / count;
  }

  void assign_credit_all(Q_Value::List * const &value_list) {
    for(Q_Value &q : *value_list)
      q.credit = 1.0;
  }

  void assign_credit_inv_update_count(Q_Value::List * const &value_list) {
    double sum = double();
    for(Q_Value &q : *value_list) {
      if(q.type != Q_Value::Type::FRINGE) {
        q.credit = 1.0 / q.update_count;
        sum += q.credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_inv_log_update_count(Q_Value::List * const &value_list) {
    double sum = double();
    for(Q_Value &q : *value_list) {
      if(q.type != Q_Value::Type::FRINGE) {
        q.credit = 1.0 / (std::log(double(q.update_count)) / this->m_credit_assignment_log_base_value + 1.0);
        sum += q.credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_inv_root_update_count(Q_Value::List * const &value_list) {
    double sum = double();
    for(Q_Value &q : *value_list) {
      if(q.type != Q_Value::Type::FRINGE) {
        q.credit = 1.0 / std::pow(double(q.update_count), this->m_credit_assignment_root_value);
        sum += q.credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_inv_depth(Q_Value::List * const &value_list) {
    size_t depth = 0;
    for(Q_Value &q : *value_list) {
      if(q.type != Q_Value::Type::FRINGE)
        ++depth;
    }

    double sum = double();
    for(Q_Value &q : *value_list) {
      if(q.type != Q_Value::Type::FRINGE) {
        q.credit = 1.0 / std::pow(2.0, double(--depth));
        sum += q.credit;
      }
    }

    assign_credit_normalize(value_list, sum);
  }

  void assign_credit_normalize(Q_Value::List * const &value_list, const double &sum) {
    if(m_credit_assignment_normalize || sum > 1.0) {
      for(Q_Value &q : *value_list) {
        if(q.type == Q_Value::Type::FRINGE)
          q.credit = 1.0;
        else
          q.credit /= sum;
      }
    }
  }

  bool split_test(Q_Value * const &q, const size_t &depth) const {
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

  static double sum_value(const action_type * const &action, const Q_Value::List &value_list) {
#ifdef DEBUG_OUTPUT
    if(action)
      std::cerr << "   sum_value(" << *action << ") = {";
#endif

    double sum = double();
    for(const Q_Value &q : value_list) {
      if(q.type != Q_Value::Type::FRINGE) {
#ifdef DEBUG_OUTPUT
        if(action)
          std::cerr << ' ' << q.value * q.weight;
#endif

        sum += q.value * q.weight;
      }
    }

#ifdef DEBUG_OUTPUT
    if(action)
      std::cerr << " } = " << sum << std::endl;
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

  Metastate m_metastate = Metastate::NON_TERMINAL;
  feature_list m_features = nullptr;
  bool m_features_complete = true;
  action_list m_candidates = nullptr;
  value_function_type m_value_function;
  std::unique_ptr<const action_type> m_current;
  std::unique_ptr<const action_type> m_next;
  std::function<action_ptruc ()> m_target_policy; ///< Sarsa/Q-Learning selector
  std::function<action_ptruc ()> m_exploration_policy; ///< Exploration policy
  std::function<bool (Q_Value * const &, const size_t &)> m_split_test; ///< true if too general, false if sufficiently general

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
        trie2.get()->update_count = 1;
      reset_update_counts_for_trie(trie2.get_deeper());
    }
  }

  feature_trie get_value_from_function(const feature_trie &head, feature_trie &function, const size_t &offset, const size_t &depth, const double &value = 0.0) {
    /** Begin logic to ensure that features enter the trie in the same order, regardless of current ordering. **/
    auto match = std::find_first_of(head->begin(), head->end(),
                                    function->begin(), function->end(),
                                    [](const feature_trie_type &lhs, const feature_trie_type &rhs)->bool {
                                      return lhs.get_key()->compare_pi(*rhs.get_key()) == 0;
                                    });

    const bool use_value = m_weight_assignment_code != "all";

    if(match) {
      auto next = static_cast<feature_trie>(head == match ? head->next() : head);
      match->erase();
      feature_trie inserted = match.get();
      match = match->map_insert(function);
      if(match != inserted)
        inserted = nullptr; ///< now holds non-zero value if the match was actually inserted into the function
      if(!m_null_q_values && !match->get()) {
        match->get() = new Q_Value(use_value ? value : 0.0);
        ++m_q_value_count;
      }

      const double value_next = value + (match->get() ? match->get()->value * match->get()->weight : 0.0);

      feature_trie deeper = nullptr;
      if(next && m_split_test(match->get(), depth)) {
        collapse_fringe(match->get_deeper(), next);
        deeper = get_value_from_function(next, match->get_deeper(), offset, depth + 1, value_next);
      }
      else {
        try {
          generate_more_features(match->get(), depth, inserted != nullptr);
        }
        catch(Again &) {
          next->destroy(next);
          throw;
        }

        generate_fringe(match->get_deeper(), next, offset, value_next);
        deeper = match->get_deeper();

        if(m_null_q_values && !match->get()) {
          match->get() = new Q_Value(use_value ? value : 0.0);
          ++m_q_value_count;
        }
      }

      match->offset_erase(offset);
      auto rv = match->offset_insert_before(offset, deeper);
      assert(rv);
      return rv;
    }
    /** End logic to ensure that features enter the trie in the same order, regardless of current ordering. **/

    auto rv = head->insert(function, m_null_q_values, m_split_test, [this](Q_Value * const &q, const size_t &depth, const bool &force){this->generate_more_features(q, depth, force);}, generate_fringe, collapse_fringe, offset, depth, value, use_value, m_q_value_count);
    assert(rv);
    return rv;
  }

  void generate_more_features(Q_Value * const &q, const size_t &depth, const bool &
#ifdef ENABLE_FRINGE
                                                                                   force
#endif
  ) {
    if(!m_features_complete && (
#ifdef ENABLE_FRINGE
                                force ||
#endif
                                         m_split_test(q, depth))) {
      m_features->destroy(m_features);
      generate_features();
#ifdef DEBUG_OUTPUT
      std::cerr << "Again:" << std::endl << *this;
#endif
      throw Again();
    }
  }

#ifdef ENABLE_FRINGE
  /// Use up the rest of the features to generate a fringe
  static void generate_fringe(feature_trie &leaf_fringe, feature_trie head, const size_t &offset, const double &value) {
    assert(!leaf_fringe || !leaf_fringe->get() || leaf_fringe->get()->type == Q_Value::Type::FRINGE);

    while(head) {
      auto next = static_cast<feature_trie>(head->next());
      head->erase();

      auto predecessor = std::find_if(leaf_fringe->begin(), leaf_fringe->end(),
                                      [&head](const feature_trie_type &existing)->bool {
                                        return existing.get_key()->precedes(*head->get_key());
                                      });

      if(predecessor)
        delete head;
      else {
        auto inserted = head->map_insert(leaf_fringe);
        if(!inserted->get())
          inserted->get() = new Q_Value(value, Q_Value::Type::FRINGE);
      }

      head = next;
    }

    if(leaf_fringe) {
      leaf_fringe->offset_erase(offset);

      feature_trie fringe = leaf_fringe;
      for(;;) {
        auto next = static_cast<feature_trie>(fringe->next());
        if(next) {
          next->offset_erase(offset);
          fringe->offset_insert_before(offset, next);
          fringe = next;
        }
        else
          break;
      }
    }
  }

  static void collapse_fringe(feature_trie &leaf_fringe, feature_trie head) {
//     assert(!leaf_fringe || !leaf_fringe->get() || leaf_fringe->get()->type != Q_Value::Type::FRINGE); ///< TODO: Convert FRINGE to UNSPLIT

    if(leaf_fringe && leaf_fringe->get() && leaf_fringe->get()->type == Q_Value::Type::FRINGE)
      leaf_fringe->destroy(leaf_fringe);
  }
#else
  static void generate_fringe(feature_trie &, feature_trie head, const size_t &, const double &) {
    head->destroy(head); ///< Destroy the rest of the features instead of generating a fringe
  }

  static void collapse_fringe(feature_trie &, feature_trie) {
  }
#endif

  static void print_value_function_trie(std::ostream &os, const feature_trie_type * const &trie) {
    for(const auto &tt : *trie) {
      os << '<' << *tt.get_key() << ',';
      if(tt.get())
        os << tt->value;
      else
        os << "nullptr";
      os << ',';
      if(tt.get_deeper())
        print_value_function_trie(os, tt.get_deeper());
      else if(tt.get() && tt.get()->type == Q_Value::Type::FRINGE)
        os << 'f';
      os << '>';
    }
  }

  virtual void generate_features() = 0;
  virtual void generate_candidates() = 0;
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
  const std::function<void (Q_Value::List * const &)> m_credit_assignment; ///< How to assign credit to multiple Q-values
  const double m_credit_assignment_epsilon = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["credit-assignment-epsilon"]).get_value();
  const double m_credit_assignment_log_base = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["credit-assignment-log-base"]).get_value();
  const double m_credit_assignment_log_base_value = std::log(m_credit_assignment_log_base);
  const double m_credit_assignment_root = dynamic_cast<const Option_Ranged<double> &>(Options::get_global()["credit-assignment-root"]).get_value();
  const double m_credit_assignment_root_value = 1.0 / m_credit_assignment_root;
  const bool m_credit_assignment_normalize = dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["credit-assignment-normalize"]).get_value();

  const std::string m_weight_assignment_code = dynamic_cast<const Option_Itemized &>(Options::get_global()["weight-assignment"]).get_value();
  const std::function<void (Q_Value::List * const &)> m_weight_assignment; ///< How to assign weight to multiple Q-values at summation time

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

  Q_Value::List * m_eligible = nullptr;

  size_t m_q_value_count = 0;
};

template <typename FEATURE, typename ACTION>
std::ostream & operator << (std::ostream &os, const Agent<FEATURE, ACTION> &agent) {
  agent.print(os);
  return os;
}

#endif
