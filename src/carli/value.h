#ifndef CARLI_VALUE_H
#define CARLI_VALUE_H

#include <cmath>

#include "linkage.h"

namespace Carli {

  class CARLI_LINKAGE Value {
    friend class Mean;

    Value(const Value &) = delete;

  public:
    Value(const double &value_ = double())
      : value(value_)
    {
    }

    void set_value(const double &rhs) {
      value = rhs;
    }

    operator double () const {return value;}
    operator double & () {return value;}

  private:
    double value;

    bool contributor = false;
    double value_contribution = 0.0;

    double value_mark2 = 0.0;
  };

  class CARLI_LINKAGE Mean {
    Mean(const Mean &) = delete;
    Mean operator=(const Mean &) = delete;

  public:
    Mean() {}

    bool outlier_above(const Value &value, const double &z = 0.84155) const;

    void contribute(Value &value);

    void uncontribute(Value &value);

    operator double () const {return mean;}

    double get_mean() const {return mean;}
    double get_variance() const {return variance;}
    double get_stddev() const {return stddev;}

  private:
    unsigned long count = 0lu;

    double mean = 0.0;

    double mean_mark2 = 0.0;
    double variance = 0.0;
    double stddev = 0.0;
  };

}

#endif
