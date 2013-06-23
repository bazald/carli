#ifndef FEATURE_H
#define FEATURE_H

#include "linked_list.h"
#include "memory_pool.h"

#include <iostream>
#include <memory>

typedef int Feature_Axis;

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature : public Zeni::Pool_Allocator<DERIVED2> {
  Feature(const Feature &) = delete;
  Feature & operator=(const Feature &) = delete;

public:
  typedef typename Zeni::Linked_List<DERIVED> List;
  typedef typename List::iterator iterator;

  struct Compare {
    bool operator()(const Feature &lhs, const Feature &rhs) const {
      return static_cast<const DERIVED &>(lhs).compare(static_cast<const DERIVED &>(rhs)) < 0;
    }
    bool operator()(const Feature &lhs, const Feature * const &rhs) const {return operator()(lhs, *rhs);}
    bool operator()(const Feature &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(lhs, *rhs);}
    bool operator()(const Feature &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(lhs, *rhs);}

    bool operator()(const Feature * const &lhs, const Feature &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const Feature * const &lhs, const Feature * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const Feature * const &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const Feature * const &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}

    bool operator()(const std::shared_ptr<DERIVED> &lhs, const Feature &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const std::shared_ptr<DERIVED> &lhs, const Feature * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::shared_ptr<DERIVED> &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::shared_ptr<DERIVED> &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}

    bool operator()(const std::unique_ptr<DERIVED> &lhs, const Feature &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const std::unique_ptr<DERIVED> &lhs, const Feature * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::unique_ptr<DERIVED> &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::unique_ptr<DERIVED> &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
  };

  struct Compare_Axis {
    bool operator()(const Feature &lhs, const Feature &rhs) const {
      return static_cast<const DERIVED &>(lhs).compare_axis(static_cast<const DERIVED &>(rhs)) < 0;
    }
    bool operator()(const Feature &lhs, const Feature * const &rhs) const {return operator()(lhs, *rhs);}
    bool operator()(const Feature &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(lhs, *rhs);}
    bool operator()(const Feature &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(lhs, *rhs);}

    bool operator()(const Feature * const &lhs, const Feature &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const Feature * const &lhs, const Feature * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const Feature * const &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const Feature * const &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}

    bool operator()(const std::shared_ptr<DERIVED> &lhs, const Feature &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const std::shared_ptr<DERIVED> &lhs, const Feature * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::shared_ptr<DERIVED> &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::shared_ptr<DERIVED> &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}

    bool operator()(const std::unique_ptr<DERIVED> &lhs, const Feature &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const std::unique_ptr<DERIVED> &lhs, const Feature * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::unique_ptr<DERIVED> &lhs, const std::shared_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::unique_ptr<DERIVED> &lhs, const std::unique_ptr<DERIVED> &rhs) const {return operator()(*lhs, *rhs);}
  };

  Feature()
    : features(static_cast<DERIVED *>(this))
  {
  }

  virtual ~Feature() {}

  virtual Feature * clone() const = 0;

  bool operator<(const Feature &rhs) const {return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs)) < 0;}
  bool operator<=(const Feature &rhs) const {return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs)) <= 0;}
  bool operator>(const Feature &rhs) const {return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs)) > 0;}
  bool operator>=(const Feature &rhs) const {return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs)) >= 0;}
  bool operator==(const Feature &rhs) const {return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs)) == 0;}
  bool operator!=(const Feature &rhs) const {return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs)) != 0;}

  int compare(const Feature &rhs) const {
    return compare(static_cast<const DERIVED &>(rhs));
  }

  int compare(const DERIVED &rhs) const {
    const int axis_comparison = static_cast<const DERIVED *>(this)->compare_axis(rhs);
    return axis_comparison ? axis_comparison : static_cast<const DERIVED *>(this)->compare_value(rhs);
  }

  virtual bool refinable() const {return false;}

  virtual void print(std::ostream &os) const = 0;

  List features;
};

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature_Present : public Feature<DERIVED, DERIVED2> {
  Feature_Present(const Feature_Present &) = delete;
  Feature_Present & operator=(const Feature_Present &) = delete;

public:
  Feature_Present(const bool &present_)
   : present(present_)
  {
  }

  int compare_value(const DERIVED &rhs) const {
    return present - rhs.present;
  }

  bool present;
};

class Feature_Ranged_Data {
public:
  Feature_Ranged_Data(const Feature_Axis &axis_, const std::shared_ptr<double> &bound_lower_, const std::shared_ptr<double> &bound_upper_, const size_t &depth_, const bool &upper_, const double &midpt_, const bool &midpt_raw_, const size_t &midpt_update_count_ = 1u)
   : axis(axis_),
   bound_lower(bound_lower_),
   bound_upper(bound_upper_),
   midpt(std::make_shared<double>((*bound_lower + *bound_upper) / 2.0)),
   depth(depth_),
   midpt_update_count(midpt_update_count_),
   midpt_raw(midpt_raw_ ? (midpt_ - *bound_lower) / (*bound_upper - *bound_lower) : midpt_),
   upper(upper_)
  {
  }

  int compare(const Feature_Ranged_Data &rhs) const {
    return depth != rhs.depth ? depth - rhs.depth :
           axis != rhs.axis ? axis - rhs.axis :
           upper - rhs.upper;
  }

  int compare_axis(const Feature_Ranged_Data &rhs) const {
    return axis - rhs.axis;
  }

  int compare_value(const Feature_Ranged_Data &rhs) const {
    return depth != rhs.depth ? depth - rhs.depth :
           upper - rhs.upper;
  }

  void print(std::ostream &os) const {
    os << this->axis << '(' << *bound_lower << ',' << *bound_upper << ':' << depth << ')';
  }

  Feature_Axis axis;

  std::shared_ptr<double> bound_lower; ///< inclusive
  std::shared_ptr<double> bound_upper; ///< exclusive
  std::shared_ptr<double> midpt; ///< A point in (bound_lower, bound_upper)

  size_t depth; ///< 0 indicates unsplit
  size_t midpt_update_count; ///< Number of times the midpt has been modified
  double midpt_raw; ///< Raw, unmodified midpt data

  bool upper; ///< Is this the upper half (same bound_upper) or lower half (same bound_lower) of a split?
};

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature_Ranged : public Feature<DERIVED, DERIVED2>, public Feature_Ranged_Data {
  Feature_Ranged(const Feature_Ranged &) = delete;
  Feature_Ranged & operator=(const Feature_Ranged &) = delete;

public:
  Feature_Ranged(const Feature_Axis &axis_, const std::shared_ptr<double> &bound_lower_, const std::shared_ptr<double> &bound_upper_, const size_t &depth_, const bool &upper_, const double &midpt_, const bool &midpt_raw_, const size_t &midpt_update_count_ = 1u)
   : ::Feature<DERIVED, DERIVED2>(),
   Feature_Ranged_Data(axis_, bound_lower_, bound_upper_, depth_, upper_, midpt_, midpt_raw_, midpt_update_count_)
  {
  }

  int compare(const DERIVED &rhs) const {
    return Feature_Ranged_Data::compare(rhs);
  }

  int compare_axis(const DERIVED &rhs) const {
    return Feature_Ranged_Data::compare_axis(rhs);
  }

  int compare_value(const DERIVED &rhs) const {
    return Feature_Ranged_Data::compare_value(rhs);
  }

  bool refinable() const {
    return true;
  }

  Feature_Ranged * clone() const {
    return new Feature_Ranged(axis, bound_lower, bound_upper, depth, upper, midpt_raw, midpt_update_count);
  }

  void print(std::ostream &os) const {
    Feature_Ranged_Data::print(os);
  }
};

#endif
