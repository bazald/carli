#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <utility>
#include <stdint.h>

#include "action.h"
#include "getopt.h"

template <typename ACTION>
class Environment : public std::enable_shared_from_this<Environment<ACTION>> {
  Environment(const Environment &) = delete;
  Environment & operator=(const Environment &) = delete;

public:
  typedef ACTION action_type;
  typedef double reward_type;

  Environment()
  {
  }

  void init() {
    init_impl();
  }

  void alter() {
    if(!m_altered) {
      alter_impl();
      m_altered = true;
    }
  }

  reward_type transition(const action_type &action) {
    return transition_impl(action);
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

  uint32_t get_scenario() const {return m_scenario;}
  void set_scenario(const uint32_t &scenario) {m_scenario = scenario;}

private:
  virtual void init_impl() = 0;
  virtual void alter_impl() {}
  virtual reward_type transition_impl(const action_type &action) = 0;
  virtual void print_impl(std::ostream &os) const = 0;

  const uint32_t m_scenario = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["scenario"]).get_value();
  bool m_altered = false;
};

template <typename ACTION>
std::ostream & operator << (std::ostream &os, const Environment<ACTION> &env) {
  env.print(os);
  return os;
}

#endif

