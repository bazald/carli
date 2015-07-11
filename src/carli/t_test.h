#ifndef T_TEST_H
#define T_TEST_H

/** A Students t test applied to two sets of data.
  * See http://www.itl.nist.gov/div898/handbook/eda/section3/eda353.htm
  * and http://www.boost.org/doc/libs/1_43_0/libs/math/doc/sf_and_dist/html/math_toolkit/dist/stat_tut/weg/st_eg/two_sample_students_t.html
  * and http://www.boost.org/doc/libs/1_43_0/libs/math/example/students_t_two_samples.cpp
  */

namespace Carli {

  CARLI_LINKAGE double degrees_of_freedom_min(const int64_t &lhs_update_count, const double &/*lhs_variance*/,
                                              const int64_t &rhs_update_count, const double &/*rhs_variance*/);

  /* The Welch-Satterthwaite approximation
   * See http://en.wikipedia.org/wiki/Welch-Satterthwaite_equation */
  CARLI_LINKAGE double degrees_of_freedom_ws(const int64_t &lhs_update_count, const double &lhs_variance,
                                             const int64_t &rhs_update_count, const double &rhs_variance);

  /* Welch's t test
   * See http://en.wikipedia.org/wiki/Welch%27s_t_test */
  CARLI_LINKAGE double t_statistic(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                   const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

  CARLI_LINKAGE double probability_eq(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                      const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

  CARLI_LINKAGE double probability_gt(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                      const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

  CARLI_LINKAGE double probability_gte(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                       const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

  CARLI_LINKAGE double probability_ne(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                      const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

  CARLI_LINKAGE double probability_lt(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                      const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

  CARLI_LINKAGE double probability_lte(const int64_t &lhs_update_count, const double &lhs_mean, const double &lhs_variance,
                                       const int64_t &rhs_update_count, const double &rhs_mean, const double &rhs_variance);

}

#endif
