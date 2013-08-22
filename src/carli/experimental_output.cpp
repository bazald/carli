#include "experimental_output.h"

#include <cfloat>
#include <iostream>

Experimental_Output::Experimental_Output(const size_t &print_every)
 : m_print_every(print_every)
{
  reset_stats();
}

void Experimental_Output::print(const size_t &total_steps, const size_t &episode_number, const size_t &step_count, const double &reward, const bool &done, const size_t &q_value_count) {
  m_cumulative_reward += reward;
  m_simple_reward += reward;

  if(done) {
    const double cumulative_reward_per_episode = m_cumulative_reward / episode_number;

    size_t steps = total_steps - step_count;
    while(steps != total_steps) {
      const size_t s2 = std::min(m_print_every - m_print_count, total_steps - steps);
      steps += s2;

      m_cumulative_min = std::min(m_cumulative_min, cumulative_reward_per_episode);
      m_cumulative_mean += s2 * cumulative_reward_per_episode / m_print_every;
      m_cumulative_max = std::max(m_cumulative_max, cumulative_reward_per_episode);
      m_simple_min = std::min(m_simple_min, m_simple_reward);
      m_simple_mean += s2 * m_simple_reward / m_print_every;
      m_simple_max = std::max(m_simple_max, m_simple_reward);

      m_print_count += s2;
      if(m_print_count == m_print_every) {
        std::cout << steps << ' '
                  << m_cumulative_min << ' ' << m_cumulative_mean << ' ' << m_cumulative_max << ' '
                  << m_simple_min << ' ' << m_simple_mean << ' ' << m_simple_max << ' '
                  << q_value_count << std::endl;

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
