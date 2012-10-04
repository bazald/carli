#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <memory>
#include <utility>

#include "linked_list.h"
#include "memory_pool.h"

template <typename TYPE>
struct Feature : public Zeni::Pool_Allocator<TYPE> {
  typedef typename Zeni::Linked_List<Feature> List;
  typedef typename List::iterator iterator;

  Feature(const bool &present_ = true)
    : features(this),
    present(present_)
  {
  }

  virtual ~Feature() {}

  void print(std::ostream &os) const {
    if(!present)
      os << '!';
    print_impl(os);
  }

  virtual void print_impl(std::ostream &os) const = 0;

  List features;

  bool present;
};

template <typename TYPE>
std::ostream & operator << (std::ostream &os, const Feature<TYPE> &feature) {
  feature.print(os);
  return os;
}

template <typename TYPE>
struct Action : public Zeni::Pool_Allocator<TYPE> {
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;

  Action()
    : candidates(this)
  {
  }

  virtual ~Action() {}

  void print(std::ostream &os) const {
    print_impl(os);
  }

  virtual void print_impl(std::ostream &os) const = 0;

  List candidates;
};

template <typename TYPE>
std::ostream & operator << (std::ostream &os, const Action<TYPE> &action) {
  action.print(os);
  return os;
}

template <typename ACTION>
class Environment : public std::enable_shared_from_this<Environment<ACTION> > {
  Environment(const Environment &);
  Environment operator=(const Environment &);

public:
  typedef ACTION action_type;
  typedef double reward_type;
  typedef size_t step_count_type;
  enum metastate_type {NON_TERMINAL, SUCCESS, FAILURE};

  Environment()
   : m_metastate(NON_TERMINAL),
   m_total_reward(reward_type()),
   m_step_count(step_count_type())
  {
  }

  metastate_type get_metastate() const {return m_metastate;}
  reward_type get_total_reward() const {return m_total_reward;}
  step_count_type get_step_count() const {return m_step_count;}

  void init() {
    m_metastate = NON_TERMINAL;
    m_total_reward = reward_type();
    m_step_count = step_count_type();

    init_impl();
  }

  reward_type transition(const action_type &action) {
    const reward_type reward = transition_impl(action);

    m_total_reward += reward;
    ++m_step_count;

    return reward;
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

protected:
  metastate_type m_metastate;

private:
  virtual void init_impl() = 0;
  virtual reward_type transition_impl(const action_type &action) = 0;
  virtual void print_impl(std::ostream &os) const = 0;

  reward_type m_total_reward;
  step_count_type m_step_count;
};

template <typename ACTION>
std::ostream & operator << (std::ostream &os, const Environment<ACTION> &env) {
  env.print(os);
  return os;
}

#endif

