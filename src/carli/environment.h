#ifndef CARLI_ENVIRONMENT_H
#define CARLI_ENVIRONMENT_H

#include <utility>
#include <stdint.h>

#include "action.h"
#include "utility/getopt.h"

namespace Carli {

  class CARLI_LINKAGE Environment : public std::enable_shared_from_this<Environment> {
  public:
    typedef double reward_type;

    Environment()
    {
    }

    Environment & operator=(const Environment &rhs) {
      m_altered = rhs.m_altered;
      return *this;
    }

    virtual ~Environment() {}

    void init() {
      ++m_episode_count;
      m_step_count = 0;
      init_impl();
    }

    void alter() {
      if(!m_altered) {
        alter_impl();
        m_altered = true;
      }
    }

    std::pair<reward_type, reward_type> transition(const Action &action) {
      ++m_step_count;
      ++m_total_step_count;
      return transition_impl(action);
    }

    void print(std::ostream &os) const {
      print_impl(os);
    }

    int64_t get_scenario() const {return m_scenario;}
    int64_t get_episode_count() const {return m_episode_count;}
    int64_t get_step_count() const {return m_step_count;}
    int64_t get_total_step_count() const {return m_total_step_count;}

    virtual bool supports_optimal() const {return false;}
    virtual double optimal_reward() const {return 0.0;}

  private:
    virtual void init_impl() = 0;
    virtual void alter_impl() {}
    virtual std::pair<reward_type, reward_type> transition_impl(const Action &action) = 0;
    virtual void print_impl(std::ostream &os) const = 0;

    const int64_t m_scenario = get_Option_Ranged<int64_t>(Options::get_global(), "scenario");
    bool m_altered = false;

    int64_t m_episode_count = -1;
    int64_t m_step_count = 0;
    int64_t m_total_step_count = -dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["skip-steps"]).get_value();
  };

}

inline std::ostream & operator<<(std::ostream &os, const Carli::Environment &env) {
  env.print(os);
  return os;
}

#endif

