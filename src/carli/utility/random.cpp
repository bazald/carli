#include "random.h"

namespace Zeni {

  Random & Random::get() {
    static Random g_random(std::random_device{}());
    return g_random;
  }

}
