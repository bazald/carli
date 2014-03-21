#ifndef INFINITE_MARIO_H
#define INFINITE_MARIO_H

#include <array>

namespace Mario {

  typedef std::array<bool, 5> Action;
  struct State;

  void infinite_mario_ai(const State &prev, const State &current, Action &action);

}

#endif
