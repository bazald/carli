#ifndef AGENT_H
#define AGENT_H

#include "clone.h"
#include "environment.h"
#include "trie.h"
#include "q_value.h"
#include "random.h"

#include <functional>
#include <map>

#define MAX_DEPTH 1

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature : public Zeni::Pool_Allocator<DERIVED2>, public Zeni::Cloneable<DERIVED> {
  Feature & operator=(const Feature &);

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
    return dynamic_cast<const DERIVED *>(this)->compare_pi(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual void print_impl(std::ostream &os) const = 0;

  List features;

  bool present;
};

template <typename DERIVED, typename DERIVED2>
std::ostream & operator << (std::ostream &os, const Feature<DERIVED, DERIVED2> &feature) {
  feature.print(os);
  return os;
}

template <typename FEATURE, typename ACTION>
class Agent : public std::enable_shared_from_this<Agent<FEATURE, ACTION> > {
  Agent(const Agent &);
  Agent operator=(const Agent &);

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
  typedef size_t step_count_type;
  enum metastate_type {NON_TERMINAL, SUCCESS, FAILURE};

  Agent(const std::shared_ptr<environment_type> &environment)
   : m_metastate(NON_TERMINAL),
   m_features(nullptr),
   m_candidates(nullptr),
   m_environment(environment),
   m_total_reward(reward_type()),
   m_step_count(step_count_type())
  {
    using namespace std::placeholders;

    m_target_policy = std::bind(&Agent::choose_greedy, this);
    m_exploration_policy = std::bind(&Agent::choose_epsilon_greedy, this, 0.1);
  }

  virtual ~Agent() {
    destroy_lists();

    for(typename value_function_type::iterator vt = m_value_function.begin(), vend = m_value_function.end(); vt != vend; ) {
      auto ptr = vt->first;
      vt->second->destroy();
      m_value_function.erase(vt++);
      delete ptr;
    }
  }

  const std::shared_ptr<const environment_type> & get_env() const {return m_environment;}
  std::shared_ptr<environment_type> & get_env() {return m_environment;}
  metastate_type get_metastate() const {return m_metastate;}
  reward_type get_total_reward() const {return m_total_reward;}
  step_count_type get_step_count() const {return m_step_count;}

  void init() {
    m_metastate = NON_TERMINAL;
    m_total_reward = reward_type();
    m_step_count = step_count_type();

    regenerate_lists();

    if(m_metastate == NON_TERMINAL)
      m_next = m_exploration_policy();
  }

  void act() {
    m_current = std::move(m_next);

    Q_Value * const value_current = get_value(m_features, *m_current, Q_Value::current_offset(), MAX_DEPTH);

    const reward_type reward = m_environment->transition(*m_current);

    update();

    if(m_metastate == NON_TERMINAL) {
      regenerate_lists();

      m_next = m_target_policy();
      Q_Value * const value_best = get_value(m_features, *m_next, Q_Value::next_offset(), MAX_DEPTH);
      td_update(&value_current->current, reward, &value_best->next, 1.0, 1.0);

      m_next = m_exploration_policy();
    }
    else {
      destroy_lists();

      td_update(&value_current->current, reward, nullptr, 1.0, 1.0);
    }

    m_total_reward += reward;
    ++m_step_count;
  }

  void print(std::ostream &os) const {
    os << " Agent:\n";
    print_list(os, "  Features:\n  ", " ", m_features);
    print_list(os, "  Candidates:\n  ", " ", m_candidates);
#ifdef DEBUG_OUTPUT
    print_value_function(os);
#endif
  }

protected:
  Q_Value * get_value(const feature_list &features, const action_type &action, const size_t &offset, const size_t &depth = size_t(-1)) {
    if(!features)
      return nullptr;

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
        ptr->insert_after(tail);
        tail = ptr;
        ++it;
      }
    }

    auto vf = m_value_function.find(const_cast<action_type *>(&action));
    if(vf == m_value_function.end())
      vf = m_value_function.insert(typename value_function_type::value_type(action.clone(), nullptr)).first;

    return get_value_from_function(head, vf->second, offset, depth);
  }

  void regenerate_lists() {
    destroy_lists();

    generate_features();
    generate_candidates();
  }

  void destroy_lists() {
    m_features->destroy();
    m_features = nullptr;
    m_candidates->destroy();
    m_candidates = nullptr;
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
    std::for_each(m_candidates->begin(), m_candidates->end(), [this,&action,&value](const action_type &action_) {
      const double value_ = sum_value(&action_, this->get_value(m_features, action_, Q_Value::next_offset(), MAX_DEPTH)->next);

      if(!action || value_ > value) {
        action = &action_;
        value = value_;
      }
    });

    return action_ptruc(action->clone());
  }

  action_ptruc choose_randomly() {
#ifdef DEBUG_OUTPUT
    std::cerr << "  choose_randomly" << std::endl;
#endif

    int counter = 0;
    std::for_each(m_candidates->begin(), m_candidates->end(), [&counter](const action_type &) {
      ++counter;
    });

    counter = random.rand_lt(counter) + 1;
    const action_type * action = nullptr;
    std::for_each(m_candidates->begin(), m_candidates->end(), [&counter,&action](const action_type &action_) {
      if(!--counter)
        action = &action_;
    });

    return action_ptruc(action->clone());
  }

  void td_update(Q_Value::List * const &current, const reward_type &reward, const Q_Value::List * const &next, const double &alpha, const double &gamma) {
    if(!current)
      return;

    const double target_value = reward + (next ? gamma * sum_value(nullptr, *next) : 0.0);

    double count = double();
    double q_old = double();
    std::for_each(current->begin(), current->end(), [&count,&q_old](const Q_Value &q) {
      ++count;
      q_old += q.value;
    });

    std::for_each(current->begin(), current->end(), [&count](Q_Value &q) {
      q.credit = 1.0 / count;
    });

    double variance_total_next = double();
    if(next) {
      std::for_each(next->begin(), next->end(), [&variance_total_next](const Q_Value &q) {
        variance_total_next += q.variance_total;
      });
    }

    const double &alpha_ = alpha;
    const double delta = target_value - q_old;
    const double &gamma_ = gamma;
    double q_new = double();
    std::for_each(current->begin(), current->end(), [this,&alpha_,&delta,gamma_,&q_new,&variance_total_next](Q_Value &q) {
      const double local_old = q.value;
      const double local_alpha = alpha_ * q.credit;
      const double local_delta = local_alpha * delta;

      ++q.update_count;

      q.value += local_delta;
      q_new += q.value;

      q.cabe += local_delta < 0.0 ? -local_delta : local_delta;
      this->m_mean_cabe.contribute(q.cabe);

      if(q.update_count > 1) {
        const double x = local_old + delta;
        const double mdelta = (x - local_old) * (x - q.value);

        q.mean2 += mdelta / q.credit; ///< divide by q.credit to prevent shrinking of estimated variance due to credit assignment
        q.variance_0 = q.mean2 / (q.update_count - 1);
        q.variance_rest += local_alpha * (q.credit * gamma_ * variance_total_next - q.variance_rest);
        q.variance_total = q.variance_0 + q.variance_rest;
        this->m_mean_variance.contribute(q.variance_total);
      }
    });

#ifdef DEBUG_OUTPUT
    std::cerr << " td_update: " << q_old << " <" << alpha << "= " << reward << " + " << gamma << " * " << (next ? sum_value(nullptr, *next) : 0.0) << std::endl;
    std::cerr << "            " << delta << " = " << alpha << " * (" << target_value << " - " << q_old << ") / " << count << std::endl
              << "            " << q_new << std::endl;

    std::for_each(current->begin(), current->end(), [this](const Q_Value &q) {
      std::cerr << " cabe:     " << q.cabe << " of " << this->m_mean_cabe << std::endl
                << " variance: " << q.variance_total << " of " << this->m_mean_variance << std::endl;
    });
#endif
  }

  static double sum_value(const action_type * const &action, const Q_Value::List &value_list) {
#ifdef DEBUG_OUTPUT
    if(action)
      std::cerr << "   sum_value(" << *action << ") = {";
#endif

    double sum = double();
    std::for_each(value_list.begin(), value_list.end(), [&action,&sum](const Q_Value &q) {
#ifdef DEBUG_OUTPUT
      if(action)
        std::cerr << ' ' << q.value;
#endif

      sum += q.value;
    });

#ifdef DEBUG_OUTPUT
    if(action)
      std::cerr << " }" << std::endl;
#endif

    return sum;
  }

#ifdef DEBUG_OUTPUT
  template <typename LIST>
  static void print_list(std::ostream &os, const std::string &head, const std::string &pre, const LIST &list) {
    if(list) {
      os << head;
      std::for_each(list->begin(), list->end(), [&os,&pre](decltype(*list->begin()) &value) {
        os << pre << value;
      });
      os << std::endl;
    }
  }

  void print_value_function(std::ostream &os) const {
    os << "  Value Function:";
    std::for_each(m_value_function.begin(), m_value_function.end(), [this,&os](decltype(*m_value_function.begin()) &value) {
      os << std::endl << "  " << *value.first << " : ";
      this->print_value_function_trie(os, value.second);
    });
    os << std::endl;
  }
#endif

  metastate_type m_metastate;
  feature_list m_features;
  action_list m_candidates;
  std::unique_ptr<const action_type> m_current;
  std::unique_ptr<const action_type> m_next;
  std::function<action_ptruc ()> m_target_policy; ///< Sarsa/Q-Learning selector
  std::function<action_ptruc ()> m_exploration_policy; ///< Exploration policy

private:
  Q_Value * get_value_from_function(feature_trie &head, feature_trie &function, const size_t &offset, const size_t &depth) {
    /** Begin logic to ensure that features enter the trie in the same order, regardless of current ordering. **/
    auto match = std::find_first_of(head->begin(), head->end(),
                                    function ? function->begin() : typename feature_trie_type::iterator(), function ? function->end() : typename feature_trie_type::iterator(),
                                    [](const feature_trie_type &lhs, const feature_trie_type &rhs)->bool {
                                      return lhs.get_key()->compare_pi(*rhs.get_key()) == 0;
                                    });

    if(match) {
      auto next = static_cast<feature_trie>(head == match ? head->next() : head);
      match->erase();
      auto mp = match->insert(function, offset, depth);
      if(next && depth)
        return next->insert(mp->get_deeper(), offset, depth - 1)->get();
      else
        return mp->get();
    }
    else
    /** End logic to ensure that features enter the trie in the same order, regardless of current ordering. **/
      return head->insert(function, offset, depth)->get();
  }

#ifdef DEBUG_OUTPUT
  static void print_value_function_trie(std::ostream &os, const feature_trie_type * const &trie) {
    if(trie) {
      for(auto tt = trie->begin(), tend = trie->end(); tt != tend; ++tt) {
        os << '<' << *tt->get_key() << ',';
        if(tt->get())
          os << (*tt)->value;
        else
          os << "nullptr";
        os << ',';
        if(tt->get_deeper())
          print_value_function_trie(os, tt->get_deeper());
        os << '>';
      }
    }
  }
#endif

  virtual void generate_features() = 0;
  virtual void generate_candidates() = 0;
  virtual void update() = 0;

  value_function_type m_value_function;
  Mean m_mean_cabe;
  Mean m_mean_variance;

  std::shared_ptr<environment_type> m_environment;

  Zeni::Random random;

  reward_type m_total_reward;
  step_count_type m_step_count;
};

template <typename FEATURE, typename ACTION>
std::ostream & operator << (std::ostream &os, const Agent<FEATURE, ACTION> &agent) {
  agent.print(os);
  return os;
}

#endif
