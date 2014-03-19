#include "value.h"

namespace Carli {

  double Mean::outlier_above(const Value &value, const double &z) const {
    return count > 1 && value > mean + z * stddev;
  }

  void Mean::contribute(Value &value) {
    if(!value.contributor) {
      const unsigned long new_count = count + 1lu;

      mean *= double(count) / new_count;

      value.contributor = true;
      count = new_count;
    }

    mean += (value.value - value.value_contribution) / count;

    const double diff = value.value - mean;
    const double mark2_contrib = diff * diff;
    mean_mark2 += mark2_contrib - value.value_mark2;
    value.value_mark2 = mark2_contrib;
    if(count > 1) {
      variance = mean_mark2 / (count - 1);
      stddev = sqrt(variance);
    }

    value.value_contribution = value.value;
  }

  void Mean::uncontribute(Value &value) {
    if(value.contributor) {
      const unsigned long new_count = count - 1lu;

      if(new_count) {
        mean -= value.value_contribution / count;
        mean *= (double(count) / new_count);

        mean_mark2 -= value.value_mark2;
        if(new_count > 1) {
          variance = mean_mark2 / (new_count - 1);
          stddev = sqrt(variance);
        }
        else {
          variance = double();
          stddev = double();
        }
      }
      else {
        mean = double();

        mean_mark2 = double();
      }

      value.contributor = false;
      value.value_contribution = double();
      value.value_mark2 = double();
      count = new_count;
    }
  }

}
