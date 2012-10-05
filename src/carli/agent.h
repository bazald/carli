#ifndef AGENT_H
#define AGENT_H

#include "environment.h"

template <typename DERIVED, typename DERIVED2>
struct Feature : public Zeni::Pool_Allocator<DERIVED2> {
  typedef typename Zeni::Linked_List<DERIVED> List;
  typedef typename List::iterator iterator;

  Feature(const bool &present_ = true)
    : features(static_cast<DERIVED *>(this)),
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
  typedef ACTION action_type;
  typedef Environment<action_type> environment_type;
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
  metastate_type m_metastate;
  feature_type * m_features;
  action_type * m_candidates;

private:
  virtual void init_impl() = 0;
  virtual reward_type act_impl() = 0;
  virtual void print_impl(std::ostream &os) const = 0;
  virtual void generate_lists() = 0;
  virtual void destroy_lists() = 0;

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
