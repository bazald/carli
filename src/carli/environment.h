#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <utility>

template <typename STATE, typename ACTION>
class Environment_Types {
public:
  typedef STATE state_type;
  typedef ACTION action_type;
  typedef double reward_type;

  struct result_type {
    result_type() {}

    result_type(const state_type &state_, const reward_type &reward_)
     : state(state_),
     reward(reward_)
    {
    }

    result_type(state_type &&state_, reward_type &&reward_)
     : state(std::move(state_)),
     reward(std::move(reward_))
    {
    }

    state_type state;
    reward_type reward;
  };

  enum meta_state_type {NON_TERMINAL, SUCCESS, FAILURE};
};

template <typename STATE, typename ACTION>
class Environment_Functions : public Environment_Types<STATE, ACTION> {
public:
  typedef Environment_Types<STATE, ACTION> base_type;
  typedef typename base_type::state_type state_type;
  typedef typename base_type::action_type action_type;
  typedef typename base_type::reward_type reward_type;
  typedef typename base_type::result_type result_type;
  typedef typename base_type::meta_state_type meta_state_type;

  state_type initial_state() const;
  result_type transition(const state_type &state, const action_type &action) const;
  meta_state_type meta_state(const state_type &state) const;
};

template <typename STATE, typename ACTION>
class Environment : public Environment_Functions<STATE, ACTION> {
public:
  typedef Environment_Types<STATE, ACTION> base_type;
  typedef typename base_type::state_type state_type;
  typedef typename base_type::action_type action_type;
  typedef typename base_type::reward_type reward_type;
  typedef typename base_type::result_type result_type;
  typedef typename base_type::meta_state_type meta_state_type;

  Environment() {
    init();
  }

  void init() {
    m_state = this->initial_state();
  }

  reward_type transition(const action_type &action) {
    const result_type result = this->transition(m_state, action);
    m_state = result.first;
    return result.second;
  }

  const state_type & state() const {return m_state;}

private:
  state_type m_state;
};

#endif
