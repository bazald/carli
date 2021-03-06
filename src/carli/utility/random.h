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

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <random>

#include "../linkage.h"

#ifdef RANDOM_LOG
#include <fstream>
#endif

namespace Zeni {

  class UTILITY_LINKAGE Random {
#ifdef RANDOM_LOG
    static std::ostream & get_os() {
      static std::ofstream os("random.txt");
      return os;
    }

    template <typename IDENT, typename VALUE1>
    static void log1(const IDENT &id, const VALUE1 &value1) {
      get_os() << id << '(' << value1 << ')' << std::endl;
    }

    template <typename IDENT, typename VALUE1, typename VALUE2>
    static void log2(const IDENT &id, const VALUE1 &value1, const VALUE2 &value2) {
      get_os() << id << '(' << value1 << ',' << value2 << ')' << std::endl;
    }
#else
    template <typename IDENT, typename VALUE1>
    static void log1(const IDENT &, const VALUE1 &) {
    }

    template <typename IDENT, typename VALUE1, typename VALUE2>
    static void log2(const IDENT &, const VALUE1 &, const VALUE2 &) {
    }
#endif

  public:
    /// Implement Concept: UniformRandomNumberGenerator
    typedef int32_t result_type;
    static result_type min() {return 0;}
    static result_type max() {return rand_max();}
    result_type operator()() {return rand();}

    Random(const uint32_t &seed = Random::get().rand())
     : m_random(seed),
       m_int_inclusive(0, rand_max()),
       m_real_exclusive(0.0, 1.0),
       m_real_inclusive(0.0, 1.0 + std::numeric_limits<double>::epsilon() / std::numeric_limits<double>::radix)
    {
      log1("Random", seed);
    }

    void seed(const uint32_t &seed) {
      m_random.seed(seed);
      log1("seed", seed);
    }

    /// Get the maximum size of a random integer returned from rand()
    static int32_t rand_max() {
      return std::numeric_limits<int32_t>::max();
    }

    /// Get a random integer in the range [0, rand_max()]
    int32_t rand() {
      const int32_t rv = m_int_inclusive(m_random);
      log1("rand", rv);
      return rv;
    }

    /// Get a random floating point number in the range [0.0, 1.0)
    double frand_lt() {
      const double rv = m_real_exclusive(m_random);
      log1("frand_lt", rv);
      return rv;
    }

    /// Get a random floating point number in the range [0.0, 1.0]
    double frand_lte() {
      double rv = m_real_inclusive(m_random);
      log1("frand_lte", rv);
      return rv;
    }

    /// Get a random integer in the range [0, mod)
    int32_t rand_lt(const int32_t &mod) {
      assert(mod > 0);
      const int32_t rv = std::uniform_int_distribution<int32_t>(0, mod - 1)(m_random);
      log2("rand_lt", mod, rv);
      return rv;
    }

    /// Get a random integer in the range [0, mod]
    int32_t rand_lte(const int32_t &mod) {
      assert(mod >= 0);
      const int32_t rv = std::uniform_int_distribution<int32_t>(0, mod)(m_random);
      log2("rand_lte", mod, rv);
      return rv;
    }

    /// Get a Gaussian floating point number in the range [-1.0, 1.0]
    double frand_gaussian() {
      // from http://docs.oracle.com/javase/1.4.2/docs/api/java/util/Random.html#nextGaussian%28%29
      if(m_have_next_gaussian) {
        m_have_next_gaussian = false;
        return m_next_gaussian;
      }
      else {
        double v1, v2, s;
        do {
          v1 = 2.0 * frand_lte() - 1.0; // between -1.0 and 1.0
          v2 = 2.0 * frand_lte() - 1.0; // between -1.0 and 1.0
          s = v1 * v1 + v2 * v2;
        } while (s >= 1 || s == 0);
        const double multiplier = std::sqrt(-2.0 * std::log(s) / s);
        m_have_next_gaussian = true;
        m_next_gaussian = v2 * multiplier;
        return v1 * multiplier;
      }
    }

    static Random & get();

  private:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
    std::mt19937 m_random;
    std::uniform_int_distribution<int32_t> m_int_inclusive;
    std::uniform_real_distribution<double> m_real_exclusive;
    std::uniform_real_distribution<double> m_real_inclusive;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    bool m_have_next_gaussian = false;
    double m_next_gaussian;
  };

}

#endif
