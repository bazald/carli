#include "experimental_output.h"

#include <algorithm>
#include <cfloat>
#include <iostream>

namespace Carli {

  Experimental_Output::Experimental_Output(const int64_t &print_every)
   : m_print_every(print_every),
   m_start(std::chrono::high_resolution_clock::now()),
   m_current(m_start)
  {
    reset_stats();
  }

  void Experimental_Output::print(const int64_t &total_steps, const int64_t &episode_number, const int64_t &step_count, const double &reward, const bool &done, const int64_t &q_value_count) {
    m_cumulative_reward += reward;
    m_simple_reward += reward;

    if(done) {
      const double cumulative_reward_per_episode = m_cumulative_reward / episode_number;

      using dseconds = std::chrono::duration<double, std::ratio<1,1>>;
      dseconds prev_total = std::chrono::duration_cast<dseconds>(m_current - m_start);
      m_current = std::chrono::high_resolution_clock::now();
      dseconds current_total = std::chrono::duration_cast<dseconds>(m_current - m_start);
      const auto time_step = (current_total - prev_total).count() / step_count;

      int64_t steps = total_steps - step_count;
      while(steps != total_steps) {
        const int64_t s2 = std::min(m_print_every - m_print_count, total_steps - steps);
        steps += s2;

        m_cumulative_min = std::min(m_cumulative_min, cumulative_reward_per_episode);
        m_cumulative_mean += s2 * cumulative_reward_per_episode / m_print_every;
        m_cumulative_max = std::max(m_cumulative_max, cumulative_reward_per_episode);
        m_simple_min = std::min(m_simple_min, m_simple_reward);
        m_simple_mean += s2 * m_simple_reward / m_print_every;
        m_simple_max = std::max(m_simple_max, m_simple_reward);

        const auto time_passed = prev_total.count() + (current_total.count() - prev_total.count()) * (steps - total_steps + step_count) / step_count;

        m_print_count += s2;
        if(m_print_count == m_print_every) {
          std::cout << steps << ' '
                    << m_cumulative_min << ' ' << m_cumulative_mean << ' ' << m_cumulative_max << ' '
                    << m_simple_min << ' ' << m_simple_mean << ' ' << m_simple_max << ' '
                    << q_value_count << ' ' << time_passed << ' ' << time_step << std::endl;

          reset_stats();
        }
      }

      m_simple_reward = 0.0;
    }
  }

  void Experimental_Output::reset_stats() {
    m_print_count = 0;

    m_cumulative_min = DBL_MAX;
    m_cumulative_mean = 0.0;
    m_cumulative_max = -DBL_MAX;

    m_simple_min = DBL_MAX;
    m_simple_mean = 0.0;
    m_simple_max = -DBL_MAX;
  }

}
