#ifndef MEAN_H
#define MEAN_H

class Value {
  friend class Mean;

public:
  Value(const double &value_ = double())
    : contributor(false),
    value(value_),
    value_contribution(double())
  {
  }

  Value & operator=(const double &q_value_) {
    value = q_value_;
    return *this;
  }

  operator double () const {return value;}
  operator double & () {return value;}

private:
  bool contributor;
  double value;
  double value_contribution;
};

class Mean {
public:
  Mean()
   : mean(double()),
   count(double())
  {
  }

  void contribute(Value &value) {
    if(value.contributor) {
      mean += (value.value - value.value_contribution) / count;
    }
    else {
      const double new_count = count + 1.0;
      mean = mean * (count / new_count) + value.value / new_count;
      value.contributor = true;
      count = new_count;
    }

    value.value_contribution = value.value;
  }

  void uncontribute(Value &value) {
    if(value.contributor) {
      const double new_count = count - 1.0;
      if(new_count)
        mean = (value.value_contribution / count) * (count / new_count);
      else
        mean = double();
      value.contributor = false;
      count = new_count;
    }
  }

  operator double () const {return mean;}

private:
  double mean;
  double count;
};

#endif
