#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <utility>

template <typename ACTION>
class Environment {
public:
  typedef ACTION action_type;
  typedef double reward_type;
  typedef size_t step_count_type;
  enum metastate_type {NON_TERMINAL, SUCCESS, FAILURE};
  typedef action_type * candidate_type;

  Environment()
   : m_metastate(NON_TERMINAL),
   m_candidates(nullptr),
   m_total_reward(reward_type()),
   m_step_count(step_count_type())
  {
  }

  Environment(const Environment &rhs)
   : m_metastate(rhs.m_metastate),
   m_candidates(nullptr),
   m_total_reward(rhs.m_total_reward),
   m_step_count(rhs.m_step_count)
  {
    if(rhs.m_candidates)
      m_candidates = generate_candidates();
  }

  Environment(Environment &&rhs)
   : m_metastate(rhs.m_metastate),
   m_candidates(rhs.m_candidates),
   m_total_reward(rhs.m_total_reward),
   m_step_count(rhs.m_step_count)
  {
    rhs.m_candidates = 0;
  }

  Environment & operator= (const Environment &rhs) {
    Environment temp(rhs);

    std::swap(temp.m_metastate, m_metastate);
    std::swap(temp.m_candidates, m_candidates);
    std::swap(temp.m_total_reward, m_total_reward);
    std::swap(temp.m_step_count, m_step_count);

    return *this;
  }

  Environment & operator= (Environment &&rhs) {
    Environment temp(std::move(rhs));

    std::swap(temp.m_metastate, m_metastate);
    std::swap(temp.m_candidates, m_candidates);
    std::swap(temp.m_total_reward, m_total_reward);
    std::swap(temp.m_step_count, m_step_count);

    return *this;
  }

  metastate_type get_metastate() const {return m_metastate;}
  reward_type get_total_reward() const {return m_total_reward;}
  step_count_type get_step_count() const {return m_step_count;}
  const action_type * get_candidates() const {return m_candidates;}

  void init() {
    m_metastate = NON_TERMINAL;
    m_total_reward = reward_type();
    m_step_count = step_count_type();

    destroy_candidates(m_candidates);

    init_impl();

    m_candidates = generate_candidates();
  }

  reward_type transition(const action_type &action) {
    const reward_type reward = transition_impl(action);

    m_total_reward += reward;
    ++m_step_count;

    destroy_candidates(m_candidates);
    m_candidates = generate_candidates();

    return reward;
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

protected:
  metastate_type m_metastate;
  candidate_type m_candidates;

private:
  virtual void init_impl() = 0;
  virtual reward_type transition_impl(const action_type &action) = 0;
  virtual void print_impl(std::ostream &os) const = 0;
  virtual candidate_type generate_candidates() = 0;
  virtual void destroy_candidates(candidate_type candidates) = 0;

  reward_type m_total_reward;
  step_count_type m_step_count;
};

template <typename ACTION>
std::ostream & operator << (std::ostream &os, const Environment<ACTION> &env) {
  env.print(os);
  return os;
}

#endif

