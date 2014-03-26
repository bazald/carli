#include "random.h"

namespace Zeni {

  Random & Random::get() {
    static Random g_random(0);
    return g_random;
  }

}
