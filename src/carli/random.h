/* This file is part of the Zenipex Library (zenilib).
 * Copyright (C) 2011 Mitchell Keith Bloch (bazald).
 *
 * zenilib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * zenilib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with zenilib.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \class Zeni::Random
 *
 * \ingroup zenilib
 *
 * \brief A Random Number Generator
 *
 * \author bazald
 *
 * Contact: bazald@zenipex.com
 */

#ifndef ZENI_RANDOM_H
#define ZENI_RANDOM_H

#include <cassert>
#include <cstdlib>
#include <stdint.h>

namespace Zeni {

  class Random {
  public:
    Random(const uint32_t &seed = uint32_t(std::rand()))
     : m_random_value(seed)
    {
    }

    /// Get the maximum size of a random integer returned from rand()
    int32_t rand_max() const {
      return 32767;
    }

    /// Get a random integer in the range [0, rand_max()]
    int32_t rand() {
      m_random_value = m_random_value * 1103515245 + 12345;
      return int32_t(static_cast<int32_t>(m_random_value / 65536) % (rand_max() + 1));
    }

    /// Get a random floating point number in the range [0.0f, 1.0f)
    float frand_lt() {
      return rand() / float(rand_max() + 1);
    }

    /// Get a random floating point number in the range [0.0f, 1.0f]
    float frand_lte() {
      return rand() / float(rand_max());
    }

    /// Get a random integer in the range [0, mod)
    int32_t rand_lt(const int32_t &mod) {
      assert(mod <= rand_max() + 1);
      return int32_t(frand_lt() * mod);
    }

    /// Get a random integer in the range [0, mod]
    int32_t rand_lte(const int32_t &mod) {
      assert(mod <= rand_max());
      return int32_t(frand_lt() * (mod + 1));
    }

  private:
    uint32_t m_random_value;
  };

}

#endif
