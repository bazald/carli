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

  void print(const size_t &total_steps, const size_t &episode_number, const size_t &step_count, const double &reward, const bool &done, const std::function<size_t ()> get_value_function_size) {
    m_cumulative_reward += reward;

    if(done) {
      const double cumulative_reward_per_episode = m_cumulative_reward / episode_number;
      const size_t value_function_size = get_value_function_size();

      size_t steps = total_steps - step_count;
      while(steps != total_steps) {
        const size_t s2 = std::min(m_print_every - m_print_count, total_steps - steps);
        steps += s2;

        m_min = std::min(m_min, cumulative_reward_per_episode);
        m_mean += s2 * cumulative_reward_per_episode / m_print_every;
        m_max = std::max(m_max, cumulative_reward_per_episode);

        m_print_count += s2;
        if(m_print_count == m_print_every) {
          std::cout << steps << ' ' << m_min << ' ' << m_mean << ' ' << m_max << ' ' << value_function_size << std::endl;
          m_print_count = 0;
          m_min = DBL_MAX;
          m_mean = 0.0;
          m_max = -DBL_MAX;
        }
      }
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
