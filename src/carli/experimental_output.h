#ifndef CARLI_EXPERIMENTAL_OUTPUT
#define CARLI_EXPERIMENTAL_OUTPUT

#include <chrono>
#include <cstddef>
#include <functional>
#include <inttypes.h>
#include <map>

#include "linkage.h"

namespace Carli {

  class CARLI_LINKAGE Experimental_Output {
  public:
    Experimental_Output(const int64_t &print_every = 1);

    void print(const int64_t &total_steps, const int64_t &episode_number, const int64_t &step_count, const double &reward, const bool &done, const int64_t &q_value_count, const std::map<int64_t, int64_t> &unrefinements, const bool &evaluate_optimality, const double &optimal_reward);

  private:
    void reset_stats();

    double m_cumulative_reward = 0.0;
    double m_simple_reward = 0.0;
    double m_cumulative_optimal_reward = 0.0;
    double m_simple_optimal_reward = 0.0;

    int64_t m_print_every;

    int64_t m_print_count;

    double m_cumulative_min;
    double m_cumulative_mean;
    double m_cumulative_max;
    double m_cumulative_optimal;

    double m_simple_min;
    double m_simple_mean;
    double m_simple_max;
    double m_simple_optimal;

    std::chrono::high_resolution_clock::time_point m_start;
    std::chrono::high_resolution_clock::time_point m_current;
  };

}

#endif
