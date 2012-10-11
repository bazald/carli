#ifndef AGENT_H
#define AGENT_H

#include "clone.h"
#include "environment.h"
#include "trie.h"
#include "value.h"

#include <map>

template <typename DERIVED, typename DERIVED2 = DERIVED>
struct Feature : public Zeni::Pool_Allocator<DERIVED2>, public Zeni::Cloneable<DERIVED> {
  typedef typename Zeni::Linked_List<DERIVED> List;
  typedef typename List::iterator iterator;

  struct Compare {
    bool operator()(const Feature &lhs, const Feature &rhs) const {
      return lhs < rhs;
    }

    bool operator()(const Feature * const &lhs, const Feature * const &rhs) const {
      return *lhs < *rhs;
    }

    bool operator()(const std::shared_ptr<const Feature> &lhs, const std::shared_ptr<const Feature> &rhs) const {
      return *lhs < *rhs;
    }
  };

  Feature(const bool &present_ = true)
    : features(static_cast<DERIVED *>(this)),
    present(present_)
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
    return dynamic_cast<const DERIVED *>(this)->compare(dynamic_cast<const DERIVED &>(rhs));
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
  typedef Zeni::Trie<std::shared_ptr<feature_type>, Value, typename feature_type::Compare> feature_trie_type;
  typedef feature_trie_type * feature_trie;
  typedef Environment<action_type> environment_type;
  typedef std::map<std::shared_ptr<action_type>, feature_trie, typename action_type::Compare> value_function_type;
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
  }

  virtual ~Agent() {
    assert(!m_features);
    assert(!m_candidates);

    for_each(m_value_function.begin(), m_value_function.end(), [](const typename value_function_type::value_type &trie) {
      trie.second->destroy();
    });
  }

  std::shared_ptr<const environment_type> get_env() const {return m_environment.lock();}
  std::shared_ptr<environment_type> get_env() {return m_environment.lock();}
  metastate_type get_metastate() const {return m_metastate;}
  reward_type get_total_reward() const {return m_total_reward;}
  step_count_type get_step_count() const {return m_step_count;}

  void init() {
    m_metastate = NON_TERMINAL;
    m_total_reward = reward_type();
    m_step_count = step_count_type();

    destroy_lists();

    init_impl();
    
    generate_lists();
  }

  void act() {
    const reward_type reward = act_impl();

    m_total_reward += reward;
    ++m_step_count;

    destroy_lists();
    generate_lists();
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

protected:
  feature_trie get_value(const action_type &action) {
    feature_trie head = nullptr;

    if(m_features) {
      auto it = m_features->begin();
      auto iend = m_features->end();

      if(it != iend) {
        head = new feature_trie_type(std::shared_ptr<feature_type>(it->clone()));
        auto tail = head;
        ++it;

        while(it != iend) {
          auto ptr = new feature_trie_type(std::shared_ptr<feature_type>(it->clone()));
          ptr->insert_after(tail);
          tail = ptr;
          ++it;
        }
      }
    }

    return head->insert(m_value_function[std::shared_ptr<action_type>(action.clone())]);
  }

  metastate_type m_metastate;
  feature_list m_features;
  action_list m_candidates;

private:
  virtual void init_impl() = 0;
  virtual reward_type act_impl() = 0;
  virtual void print_impl(std::ostream &os) const = 0;
  virtual void generate_lists() = 0;
  virtual void destroy_lists() = 0;

  value_function_type m_value_function;

  std::weak_ptr<environment_type> m_environment;

  reward_type m_total_reward;
  step_count_type m_step_count;
};

template <typename FEATURE, typename ACTION>
std::ostream & operator << (std::ostream &os, const Agent<FEATURE, ACTION> &agent) {
  agent.print(os);
  return os;
}

#endif
