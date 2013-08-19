#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <utility>
#include <stdint.h>

#include "action.h"
#include "getopt.h"

class Environment : public std::enable_shared_from_this<Environment> {
  Environment(const Environment &) = delete;
  Environment & operator=(const Environment &) = delete;

public:
  typedef double reward_type;

  Environment()
  {
  }

  virtual ~Environment() {}

  void init() {
    init_impl();
  }

  void alter() {
    if(!m_altered) {
      alter_impl();
      m_altered = true;
    }
  }

  reward_type transition(const Action &action) {
    return transition_impl(action);
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

  uint32_t get_scenario() const {return m_scenario;}

private:
  virtual void init_impl() = 0;
  virtual void alter_impl() {}
  virtual reward_type transition_impl(const Action &action) = 0;
  virtual void print_impl(std::ostream &os) const = 0;

  const uint32_t m_scenario = dynamic_cast<const Option_Ranged<int> &>(Options::get_global()["scenario"]).get_value();
  bool m_altered = false;
};

inline std::ostream & operator<<(std::ostream &os, const Environment &env) {
  env.print(os);
  return os;
}

#endif

