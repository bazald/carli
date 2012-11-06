#ifndef AGENT_H
#define AGENT_H

#include "clone.h"
#include "environment.h"
#include "trie.h"
#include "q_value.h"
#include "random.h"

#include <functional>
#include <map>
#include <set>
#include <stack>
#include <string>

#include <iostream>

template <size_t VALUE>
struct Binary_Log {
  enum Value {value = Binary_Log<VALUE / 2>::value + 1};
};
template <>
struct Binary_Log<1> {
  enum Value {value = 0};
};

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
    return compare_pi(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual int compare_pi(const DERIVED &rhs) const = 0;

  virtual void print_impl(std::ostream &os) const = 0;

  List features;

  bool present;
};

template <typename DERIVED, typename DERIVED2>
std::ostream & operator << (std::ostream &os, const Feature<DERIVED, DERIVED2> &feature) {
  feature.print(os);
  return os;
}

enum Metastate {NON_TERMINAL, SUCCESS, FAILURE};

template <typename FEATURE, typename ACTION>
class Agent : public std::enable_shared_from_this<Agent<FEATURE, ACTION> > {
  Agent(const Agent &);
  Agent operator=(const Agent &);

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
  enum Credit_Assignment {EVEN, INV_UPDATE_COUNT, INV_LOG_UPDATE_COUNT};

  Agent(const std::shared_ptr<environment_type> &environment)
   : m_metastate(NON_TERMINAL),
   m_features(nullptr),
   m_features_complete(true),
   m_candidates(nullptr),
   m_environment(environment),
   m_episode_number(1),
   m_step_count(0),
   m_total_reward(reward_type()),
   m_learning_rate(0.3),
   m_discount_rate(0.9),
   m_credit_assignment_code(EVEN),
   m_on_policy(false),
   m_epsilon(0.1),
   m_pseudoepisode_threshold(20),
   m_split_min(0),
   m_split_max(size_t(-1)),
   m_split_pseudoepisodes(0),
   m_split_cabe(0.84155)
  {
    m_target_policy = [this]()->action_ptruc{return this->choose_greedy();};
    m_exploration_policy = [this]()->action_ptruc{return this->choose_epsilon_greedy(m_epsilon);};
    m_credit_assignment = [this](Q_Value::List * const &value_list){return this->assign_credit_evenly(value_list);};

    m_split_test = [this](Q_Value * const &q, const size_t &depth)->bool{
      if(depth < m_split_min) {
        if(q)
          q->split = true;
        return true;
      }
      if(depth >= m_split_max)
        return false;

      if(!q)
        return false;
      if(q->split)
        return true;

      q->split |= q->pseudoepisode_count > m_split_pseudoepisodes &&
                  this->get_mean_cabe().outlier_above(q->cabe, m_split_cabe);

      return q->split;
    };
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

  double get_learning_rate() const {return m_learning_rate;}
  void set_learning_rate(const double &learning_rate) {
    if(learning_rate <= 0.0 || learning_rate > 1.0)
      throw std::range_error("Illegal learning rate.");
    m_learning_rate = learning_rate;
  }

  double get_discount_rate() const {return m_discount_rate;}
  void set_discount_rate(const double &discount_rate) {
    if(discount_rate < 0.0 || discount_rate > 1.0)
      throw std::range_error("Illegal discount rate.");
    m_discount_rate = discount_rate;
  }

  Credit_Assignment get_credit_assignment() const {return m_credit_assignment_code;}
  void set_credit_assignment(const Credit_Assignment &credit_assignment) {
    switch(credit_assignment) {
      case EVEN:
        m_credit_assignment = [this](Q_Value::List * const &value_list){return this->assign_credit_evenly(value_list);};
        break;
        
      case INV_UPDATE_COUNT:
        m_credit_assignment = [this](Q_Value::List * const &value_list){return this->assign_credit_inv_update_count(value_list);};
        break;
        
      case INV_LOG_UPDATE_COUNT:
        m_credit_assignment = [this](Q_Value::List * const &value_list){return this->assign_credit_inv_log_update_count(value_list);};
        break;
    }

    m_credit_assignment_code = credit_assignment;
  }

  bool get_on_policy() const {return m_on_policy;}
  void set_on_policy(const bool &on_policy) {
    if(on_policy)
      m_target_policy = [this]()->action_ptruc{return this->m_exploration_policy();};
    else
      m_target_policy = [this]()->action_ptruc{return this->choose_greedy();};

    m_on_policy = on_policy;
  }

  double get_epsilon() const {return m_epsilon;}
  void set_epsilon(const double &epsilon) {
    if(epsilon < 0.0 || epsilon > 1.0)
      throw std::range_error("Illegal epsilon.");
    m_epsilon = epsilon;
  }

  size_t get_pseudoepisode_threshold() const {return m_pseudoepisode_threshold;}
  void set_pseudoepisode_threshold(const size_t &pseudoepisode_threshold) {
    m_pseudoepisode_threshold = pseudoepisode_threshold;
  }

  size_t get_split_min() const {return m_split_min;}
  void set_split_min(const size_t &split_min) {
    m_split_min = split_min;
  }

  size_t get_split_max() const {return m_split_max;}
  void set_split_max(const size_t &split_max) {
    m_split_max = split_max;
  }

  size_t get_split_pseudoepisodes() const {return m_split_pseudoepisodes;}
  void set_split_pseudoepisodes(const size_t &split_pseudoepisodes) {
    m_split_pseudoepisodes = split_pseudoepisodes;
  }

  double get_split_cabe() const {return m_split_cabe;}
  void set_split_cabe(const double &split_cabe) {
    m_split_cabe = split_cabe;
  }

  const std::shared_ptr<const environment_type> & get_env() const {return m_environment;}
  std::shared_ptr<environment_type> & get_env() {return m_environment;}
  Metastate get_metastate() const {return m_metastate;}
  size_t get_episode_number() const {return m_episode_number;}
  size_t get_step_count() const {return m_step_count;}
  reward_type get_total_reward() const {return m_total_reward;}
  Mean get_mean_cabe() const {return m_mean_cabe;}

  void init() {
    if(m_metastate != NON_TERMINAL)
      ++m_episode_number;
    m_metastate = NON_TERMINAL;
    m_step_count = 0;
    m_total_reward = reward_type();

    regenerate_lists();

    if(m_metastate == NON_TERMINAL)
      m_next = m_exploration_policy();
  }

  reward_type act() {
    m_current = std::move(m_next);

    Q_Value * const value_current = get_value(m_features, *m_current, Q_Value::current_offset());

    const reward_type reward = m_environment->transition(*m_current);

    update();

    if(m_metastate == NON_TERMINAL) {
      regenerate_lists();

      m_next = m_target_policy();
#ifdef DEBUG_OUTPUT
      std::cerr << "   " << *m_next << " is next." << std::endl;
#endif
      Q_Value * const value_best = get_value(m_features, *m_next, Q_Value::next_offset());
      td_update(&value_current->current, reward, &value_best->next);

      if(!m_on_policy) {
        m_next = m_exploration_policy();
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
    std::for_each(m_value_function.begin(), m_value_function.end(), [this,&os](decltype(*m_value_function.begin()) &value) {
      os << std::endl << "  " << *value.first << " : ";
      this->print_value_function_trie(os, value.second);
    });
    os << std::endl;
  }

  size_t get_value_function_size() const {
    size_t size = 0lu;
    std::for_each(m_value_function.begin(), m_value_function.end(), [this,&size](const typename value_function_type::value_type &vf) {
      size += this->get_trie_size(vf.second);
    });
    return size;
  }

protected:
  Q_Value * get_value(const feature_list &features, const action_type &action, const size_t &offset, const size_t &depth = size_t()) {
    if(!features)
      return nullptr;

    for(;;) {
      try {
        feature_trie head = nullptr;

        auto it = features->begin(features);
        auto iend = features->end(features);
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

        return get_value_from_function(head, get_trie(action), offset, depth)->get();
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
    std::for_each(m_candidates->begin(m_candidates), m_candidates->end(m_candidates), [this,&action,&value](const action_type &action_) {
      const double value_ = sum_value(&action_, this->get_value(m_features, action_, Q_Value::next_offset())->next);

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
    std::for_each(m_candidates->begin(m_candidates), m_candidates->end(m_candidates), [this,&counter](const action_type &action_) {
      ++counter;
      this->get_value(m_features, action_, Q_Value::next_offset()); ///< Trigger additional feature generation, as needed
    });

    counter = random.rand_lt(counter) + 1;
    const action_type * action = nullptr;
    std::for_each(m_candidates->begin(m_candidates), m_candidates->end(m_candidates), [&counter,&action](const action_type &action_) {
      if(!--counter)
        action = &action_;
    });

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
    std::for_each(current->begin(current), current->end(current), [&q_old](Q_Value &q) {
      q_old += q.value;
      ++q.update_count;
#ifdef DEBUG_OUTPUT
      std::cerr << ' ' << &q;
#endif
    });
#ifdef DEBUG_OUTPUT
    std::cerr << std::endl;
    std::cerr << " next    :";
    std::for_each(next->begin(next), next->end(next), [](const Q_Value &q) {
      std::cerr << ' ' << &q;
    });
    std::cerr << std::endl;
#endif

#ifdef TRACK_Q_VALUE_VARIANCE
    double variance_total_next = double();
    if(next) {
      std::for_each(next->begin(next), next->end(next), [&variance_total_next](const Q_Value &q) {
        variance_total_next += q.variance_total;
      });
    }
#endif

    m_credit_assignment(current);

    const double delta = target_value - q_old;
    double q_new = double();
    std::for_each(current->begin(current), current->end(current), [this,&delta,&q_new
#ifdef TRACK_Q_VALUE_VARIANCE
                                                                   ,&variance_total_next
#endif
                                                                   ](Q_Value &q) {
#ifdef TRACK_Q_VALUE_VARIANCE
      const double local_old = q.value;
#endif

      q.value += this->m_learning_rate * q.credit * delta;
      q_new += q.value;

      if(q.split) {
        this->m_mean_cabe.uncontribute(q.cabe);
#ifdef TRACK_Q_VALUE_VARIANCE
        this->m_mean_variance.uncontribute(q.variance_total);
#endif
      }
      else {
        if(q.last_episode_fired != this->m_episode_number) {
          ++q.pseudoepisode_count;
          q.last_episode_fired = this->m_episode_number;
        }
        else if(this->m_step_count - q.last_step_fired > m_pseudoepisode_threshold)
          ++q.pseudoepisode_count;
        q.last_step_fired = this->m_step_count;

        q.cabe += std::abs(delta);
        this->m_mean_cabe.contribute(q.cabe);

#ifdef TRACK_Q_VALUE_VARIANCE
        if(q.update_count > 1) {
          const double x = local_old + delta;
          const double mdelta = (x - local_old) * (x - q.value);

          q.mean2 += mdelta / q.credit; ///< divide by q.credit to prevent shrinking of estimated variance due to credit assignment
          q.variance_0 = q.mean2 / (q.update_count - 1);
          q.variance_rest += local_learning_rate * (q.credit * this->m_discount_rate * variance_total_next - q.variance_rest);
          q.variance_total = q.variance_0 + q.variance_rest;
          this->m_mean_variance.contribute(q.variance_total);
        }
#endif
      }
    });

#ifdef DEBUG_OUTPUT
    std::cerr << " td_update: " << q_old << " <" << m_learning_rate << "= " << reward << " + " << m_discount_rate << " * " << target_next << std::endl;
    std::cerr << "            " << delta << " = " << target_value << " - " << q_old << std::endl
              << "            " << q_new << std::endl;

    std::for_each(current->begin(current), current->end(current), [this](const Q_Value &q) {
      if(!q.split) {
        std::cerr << " updates:  " << q.update_count << std::endl
                  << " cabe:     " << q.cabe << " of " << this->m_mean_cabe << ':' << this->m_mean_cabe.get_stddev() << std::endl;
#ifdef TRACK_Q_VALUE_VARIANCE
        std::cerr << " variance: " << q.variance_total << " of " << this->m_mean_variance << ':' << this->m_mean_variance.get_stddev() << std::endl;
#endif
      }
    });
#endif
  }

  void assign_credit_evenly(Q_Value::List * const &value_list) {
    double count = double();
    std::for_each(value_list->begin(value_list), value_list->end(value_list), [&count](const Q_Value &q) {
      ++count;
    });

    std::for_each(value_list->begin(value_list), value_list->end(value_list), [&count](Q_Value &q) {
      q.credit = 1.0 / count;
    });
  }

  void assign_credit_inv_update_count(Q_Value::List * const &value_list) {
    double sum = double();
    std::for_each(value_list->begin(value_list), value_list->end(value_list), [&sum](Q_Value &q) {
      q.credit = 1.0 / q.update_count;
      sum += q.credit;
    });

    std::for_each(value_list->begin(value_list), value_list->end(value_list), [&sum](Q_Value &q) {
      q.credit /= sum;
    });
  }

  void assign_credit_inv_log_update_count(Q_Value::List * const &value_list) {
    double sum = double();
    std::for_each(value_list->begin(value_list), value_list->end(value_list), [&sum](Q_Value &q) {
      q.credit = 1.0 / (log(double(q.update_count)) + 1.0);
      sum += q.credit;
    });

    std::for_each(value_list->begin(value_list), value_list->end(value_list), [&sum](Q_Value &q) {
      q.credit /= sum;
    });
  }

  static double sum_value(const action_type * const &action, const Q_Value::List &value_list) {
#ifdef DEBUG_OUTPUT
    if(action)
      std::cerr << "   sum_value(" << *action << ") = {";
#endif

    double sum = double();
    std::for_each(value_list.begin(&value_list), value_list.end(&value_list), [&action,&sum](const Q_Value &q) {
#ifdef DEBUG_OUTPUT
      if(action)
        std::cerr << ' ' << q.value;
#endif

      sum += q.value;
    });

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
      std::for_each(list->begin(list), list->end(list), [&os,&pre](decltype(*list->begin(list)) &value) {
        os << pre << value;
      });
      os << std::endl;
    }
  }
#endif

  Metastate m_metastate;
  feature_list m_features;
  bool m_features_complete;
  action_list m_candidates;
  value_function_type m_value_function;
  std::unique_ptr<const action_type> m_current;
  std::unique_ptr<const action_type> m_next;
  std::function<action_ptruc ()> m_target_policy; ///< Sarsa/Q-Learning selector
  std::function<action_ptruc ()> m_exploration_policy; ///< Exploration policy
  std::function<bool (Q_Value * const &, const size_t &)> m_split_test; ///< true if too general, false if sufficiently general

private:
  static size_t get_trie_size(const feature_trie_type * const &trie) {
    size_t size = 0lu;
    std::for_each(trie->begin(trie), trie->end(trie), [&size](const feature_trie_type &trie2) {
      if(trie2.get())
        ++size;
      size += get_trie_size(trie2.get_deeper());
    });
    return size;
  }

  feature_trie get_value_from_function(const feature_trie &head, feature_trie &function, const size_t &offset, const size_t &depth) {
    /** Begin logic to ensure that features enter the trie in the same order, regardless of current ordering. **/
    auto match = std::find_first_of(head->begin(head), head->end(head),
                                    function->begin(function), function->end(function),
                                    [](const feature_trie_type &lhs, const feature_trie_type &rhs)->bool {
                                      return lhs.get_key()->compare_pi(*rhs.get_key()) == 0;
                                    });

    if(match) {
      auto next = static_cast<feature_trie>(head == match ? head->next() : head);
      match->erase();
      match = match->map_insert(function);
#ifndef NULL_Q_VALUES
      if(!match->get())
        match->get() = new Q_Value;
#endif

      feature_trie deeper = nullptr;
      if(next && m_split_test(match->get(), depth))
        deeper = get_value_from_function(next, match->get_deeper(), offset, depth + 1);
      else {
        next->destroy(next);
        generate_more_features(match->get(), depth);
#ifdef NULL_Q_VALUES
        if(!match->get())
          match->get() = new Q_Value;
#endif
      }

      match->offset_erase(offset);
      auto rv = match->offset_insert_before(offset, deeper);
      assert(rv);
      return rv;
    }
    /** End logic to ensure that features enter the trie in the same order, regardless of current ordering. **/

    auto rv = head->insert(function, m_split_test, [this](Q_Value * const &q, const size_t &depth){this->generate_more_features(q, depth);}, offset, depth);
    assert(rv);
    return rv;
  }

  void generate_more_features(Q_Value * const &q, const size_t &depth) {
    if(!m_features_complete && m_split_test(q, depth)) {
      m_features->destroy(m_features);
      generate_features();
#ifdef DEBUG_OUTPUT
      std::cerr << "Again:" << std::endl << *this;
#endif
      throw Again();
    }
  }

  static void print_value_function_trie(std::ostream &os, const feature_trie_type * const &trie) {
    if(trie) {
      for(auto tt = trie->begin(trie), tend = trie->end(trie); tt != tend; ++tt) {
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

  virtual void generate_features() = 0;
  virtual void generate_candidates() = 0;
  virtual void update() = 0;

  Mean m_mean_cabe;
#ifdef TRACK_Q_VALUE_VARIANCE
  Mean m_mean_variance;
#endif

  std::shared_ptr<environment_type> m_environment;

  Zeni::Random random;

  size_t m_episode_number;
  size_t m_step_count;
  reward_type m_total_reward;

  double m_learning_rate; ///< alpha
  double m_discount_rate; ///< gamma

  Credit_Assignment m_credit_assignment_code;
  std::function<void (Q_Value::List * const &)> m_credit_assignment; ///< How to assign credit to multiple Q-values

  bool m_on_policy; ///< for Sarsa/Q-learning selection
  double m_epsilon; ///< for epsilon-greedy decision-making
  size_t m_pseudoepisode_threshold; ///< For deciding how many steps indicates a pseudoepisode

  size_t m_split_min;
  size_t m_split_max;
  size_t m_split_pseudoepisodes;
  double m_split_cabe;
};

template <typename FEATURE, typename ACTION>
std::ostream & operator << (std::ostream &os, const Agent<FEATURE, ACTION> &agent) {
  agent.print(os);
  return os;
}

#endif
