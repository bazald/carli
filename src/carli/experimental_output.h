#ifndef EXPERIMENTAL_OUTPUT
#define EXPERIMENTAL_OUTPUT

#include <cfloat>
#include <iostream>

class Experimental_Output {
public:
  Experimental_Output(const size_t &print_every = 1)
   : m_print_every(print_every),
   m_print_count(0),
   m_cumulative_reward(0.0),
   m_min(DBL_MAX),
   m_mean(0.0),
   m_max(-DBL_MAX)
  {
  }

  void print(const size_t &total_steps, const size_t &episode_number, const size_t &/*step_count*/, const double &reward) {
    m_cumulative_reward += reward;

    const double cumulative_reward_per_episode = m_cumulative_reward / episode_number;

    m_min = std::min(m_min, cumulative_reward_per_episode);
    m_mean += cumulative_reward_per_episode / m_print_every;
    m_max = std::max(m_max, cumulative_reward_per_episode);

    if(++m_print_count == m_print_every) {
      std::cout << total_steps << ' ' << m_min << ' ' << m_mean << ' ' << m_max << std::endl;
      m_print_count = 0;
      m_min = DBL_MAX;
      m_mean = 0.0;
      m_max = -DBL_MAX;
    }
  }

private:
  size_t m_print_every;
  size_t m_print_count;

  double m_cumulative_reward;

  double m_min;
  double m_mean;
  double m_max;
};

#endif
