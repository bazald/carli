#ifndef FEATURE_H
#define FEATURE_H

#include "clone.h"
#include "linked_list.h"
#include "memory_pool.h"

#include <iostream>
#include <memory>

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature : public Zeni::Pool_Allocator<DERIVED2> {
  Feature(const Feature &) = delete;
  Feature & operator=(const Feature &) = delete;

public:
  typedef typename Zeni::Linked_List<DERIVED> List;
  typedef typename List::iterator iterator;

  struct Compare {
    bool operator()(const Feature &lhs, const Feature &rhs) const {
      return lhs.compare(rhs) < 0;
    }

    bool operator()(const Feature * const &lhs, const Feature * const &rhs) const {
      return operator()(*lhs, *rhs);
    }

    bool operator()(const std::shared_ptr<const Feature> &lhs, const std::shared_ptr<const Feature> &rhs) const {
      return operator()(*lhs, *rhs);
    }

    bool operator()(const std::unique_ptr<const Feature> &lhs, const std::unique_ptr<const Feature> &rhs) const {
      return operator()(*lhs, *rhs);
    }
  };

  Feature()
    : features(static_cast<DERIVED *>(this))
  {
  }

  virtual ~Feature() {}

  virtual Feature * clone() const = 0;

  bool operator<(const Feature &rhs) const {return compare(rhs) < 0;}
  bool operator<=(const Feature &rhs) const {return compare(rhs) <= 0;}
  bool operator>(const Feature &rhs) const {return compare(rhs) > 0;}
  bool operator>=(const Feature &rhs) const {return compare(rhs) >= 0;}
  bool operator==(const Feature &rhs) const {return compare(rhs) == 0;}
  bool operator!=(const Feature &rhs) const {return compare(rhs) != 0;}

  int compare(const Feature &rhs) const {
    return compare(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual int compare(const DERIVED &rhs) const {
    const int axis_comparison = compare_axis(rhs);
    return axis_comparison ? axis_comparison : compare_value(rhs);
  }

  int compare_axis(const Feature &rhs) const {
    return compare_axis(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual int compare_axis(const DERIVED &rhs) const {return 0;}

  int compare_value(const Feature &rhs) const {
    return compare_value(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual int compare_value(const DERIVED &rhs) const = 0;

  virtual bool refinable() const {return false;}

  virtual bool precedes(const DERIVED &) const {return false;}

  virtual void print(std::ostream &os) const = 0;

  List features;
};

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature_Present : public Feature<DERIVED, DERIVED2> {
public:
  Feature_Present(const bool &present_)
   : present(present_)
  {
  }

  int compare_value(const DERIVED &rhs) const {
    return rhs.present - present;
  }

  bool present;
};

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Feature_Ranged : public Feature<DERIVED, DERIVED2> {
public:
  typedef int Axis;

  Feature_Ranged(const Axis &axis_, const double &bound_lower_, const double &bound_higher_, const size_t &depth_)
   : ::Feature<DERIVED, DERIVED2>(),
   axis(axis_),
   bound_lower(bound_lower_),
   bound_higher(bound_higher_),
   depth(depth_)
  {
  }

  int compare(const DERIVED &rhs) const {
    return depth != rhs.depth ? (depth < rhs.depth ? -1 : 1) :
           axis != rhs.axis ? (axis < rhs.axis ? -1 : 1) :
           bound_lower != rhs.bound_lower ? (bound_lower < rhs.bound_lower ? -1 : 1) :
           (bound_higher == rhs.bound_higher ? 0 : bound_higher < rhs.bound_higher ? -1 : 1);
  }

  int compare_axis(const DERIVED &rhs) const {
    return axis - rhs.axis;
  }

  int compare_value(const DERIVED &rhs) const {
    return depth != rhs.depth ? (depth < rhs.depth ? -1 : 1) :
           bound_lower != rhs.bound_lower ? (bound_lower < rhs.bound_lower ? -1 : 1) :
           (bound_higher == rhs.bound_higher ? 0 : bound_higher < rhs.bound_higher ? -1 : 1);
  }

  bool refinable() const {
    return true;
  }

  bool precedes(const DERIVED &rhs) const {
    const auto mdpt = midpt();
    return axis == rhs.axis && (rhs.bound_lower == mdpt || rhs.bound_higher == mdpt);
  }

  double midpt() const {
    return (bound_lower + bound_higher) / 2.0;
  }

  Feature_Ranged * clone() const {
    return new Feature_Ranged(this->axis, bound_lower, bound_higher, depth);
  }

  void print(std::ostream &os) const {
    os << this->axis << '(' << bound_lower << ',' << bound_higher << ':' << depth << ')';
  }

  Axis axis;

  double bound_lower; ///< inclusive
  double bound_higher; ///< exclusive

  size_t depth; ///< 0 indicates unsplit
};

#endif
