#include "t_test.h"

#ifdef ENABLE_T_TEST

#include <boost/math/distributions/students_t.hpp>

/** A Students t test applied to two sets of data.
  * See http://www.itl.nist.gov/div898/handbook/eda/section3/eda353.htm
  * and http://www.boost.org/doc/libs/1_43_0/libs/math/doc/sf_and_dist/html/math_toolkit/dist/stat_tut/weg/st_eg/two_sample_students_t.html
  * and http://www.boost.org/doc/libs/1_43_0/libs/math/example/students_t_two_samples.cpp
  */

namespace Carli {

  double degrees_of_freedom_min(const int64_t &lhs_update_count, const double &/*lhs_variance*/,
                                const int64_t &rhs_update_count, const double &/*rhs_variance*/) {
    return std::max(1.0, std::min(lhs_update_count, rhs_update_count) - 2.0);
  }

  /* The Welch-Satterthwaite approximation
   * See http://en.wikipedia.org/wiki/Welch-Satterthwaite_equation */
  double degrees_of_freedom_ws(const int64_t &lhs_update_count, const double &lhs_variance,
                               const int64_t &rhs_update_count, const double &rhs_variance) {
    if(lhs_update_count < 2 || rhs_update_count < 2)
      return lhs_update_count + rhs_update_count - 2.0; ///< Assume equal variances

    double t1 = lhs_variance / lhs_update_count;
    double t2 = rhs_variance / rhs_update_count;

    if(t1 + t2 <= 0.0)
      return lhs_update_count + rhs_update_count - 2.0; ///< Avoid divide by zero error

    double v = t1 + t2;

    t1 *= t1;
    t2 *= t2;
    v *= v;

    t1 /= lhs_update_count - 1.0;
    t2 /= rhs_update_count - 1.0;
    v /= t1 + t2;

    return v;
  }

  /* Welch's t test
   * See http://en.wikipedia.org/wiki/Welch%27s_t_test */
  double t_statistic(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                     const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    return (lhs_mean - rhs_mean) / sqrt(lhs_variance / lhs_update_count + rhs_variance / rhs_update_count);
  }

  double probability_eq(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                        const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    if(lhs_update_count && rhs_update_count && lhs_update_count + rhs_update_count > 2.0) {
      const double dof = degrees_of_freedom_min(lhs_update_count, lhs_variance, rhs_update_count, rhs_variance);
      const double ts = t_statistic(lhs_update_count, lhs_mean, lhs_variance, rhs_update_count, rhs_mean, rhs_variance);
      const boost::math::students_t_distribution<double> dist(dof);
      return 2 * cdf(complement(dist, fabs(ts)));
    }
    else
      return 0.5;
  }

  double probability_gt(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                        const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    if(lhs_update_count && rhs_update_count && lhs_update_count + rhs_update_count > 2.0) {
      const double dof = degrees_of_freedom_min(lhs_update_count, lhs_variance, rhs_update_count, rhs_variance);
      const double ts = t_statistic(lhs_update_count, lhs_mean, lhs_variance, rhs_update_count, rhs_mean, rhs_variance);
      const boost::math::students_t_distribution<double> dist(dof);
      return cdf(dist, ts);
    }
    else
      return 0.5;
  }

  double probability_gte(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                         const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    if(lhs_update_count && rhs_update_count && lhs_update_count + rhs_update_count > 2) {
      const double dof = degrees_of_freedom_min(lhs_update_count, lhs_variance, rhs_update_count, rhs_variance);
      const double ts = t_statistic(lhs_update_count, lhs_mean, lhs_variance, rhs_update_count, rhs_mean, rhs_variance);
      const boost::math::students_t_distribution<double> dist(dof);
      return std::max(cdf(dist, ts), 2.0 * cdf(complement(dist, fabs(ts))));
    }
    else
      return 0.5;
  }

  double probability_ne(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                        const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    return 1.0 - probability_eq(rhs_update_count, rhs_mean, rhs_variance, lhs_update_count, lhs_mean, lhs_variance);
  }

  double probability_lt(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                        const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    return probability_gte(rhs_update_count, rhs_mean, rhs_variance, lhs_update_count, lhs_mean, lhs_variance);
  }

  double probability_lte(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                         const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance) {
    return probability_gt(rhs_update_count, rhs_mean, rhs_variance, lhs_update_count, lhs_mean, lhs_variance);
  }

}

#endif
