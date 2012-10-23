#ifndef MEAN_H
#define MEAN_H

class Value {
  friend class Mean;

public:
  Value(const double &value_ = double())
    : value(value_),
    contributor(false),
    value_contribution(double()),
    value_mark2(double())
  {
  }

  Value & operator=(const double &q_value_) {
    value = q_value_;
    return *this;
  }

  operator double () const {return value;}
  operator double & () {return value;}

private:
  double value;

  bool contributor;
  double value_contribution;

  double value_mark2;
};

class Mean {
public:
  Mean()
   : count(0lu),
   mean(double()),
   mean_mark2(double()),
   variance(double()),
   stddev(double())
  {
  }

  void contribute(Value &value) {
    if(!value.contributor) {
      const unsigned long new_count = count + 1lu;

      mean *= double(count) / new_count;

      value.contributor = true;
      count = new_count;
    }

    const double mean_old = mean;

    mean += (value.value - value.value_contribution) / count;

    const double mark2_contrib = (value.value - mean_old) * (value.value - mean);
    mean_mark2 += mark2_contrib - value.value_mark2;
    value.value_mark2 = mark2_contrib;
    if(count > 1) {
      variance = mean_mark2 / (count - 1);
      stddev = sqrt(variance);
    }

    value.value_contribution = value.value;
  }

  void uncontribute(Value &value) {
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

  operator double () const {return mean;}

  double get_mean() const {return mean;}
  double get_variance() const {return variance;}
  double get_stddev() const {return stddev;}

private:
  unsigned long count;

  double mean;

  double mean_mark2;
  double variance;
  double stddev;
};

#endif
