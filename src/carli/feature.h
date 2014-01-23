#ifndef FEATURE_H
#define FEATURE_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

#include "rete/rete.h"

#include <iostream>
#include <memory>
#include <sstream>

class Feature : public Zeni::Pool_Allocator<char> {
  Feature(const Feature &) = delete;
  Feature & operator=(const Feature &) = delete;

public:
  typedef typename Zeni::Linked_List<Feature> List;
  typedef typename List::iterator iterator;

  Feature()
    : features(this)
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

  virtual int compare(const Feature &rhs) const {
    const int depth_comparison = get_depth() - rhs.get_depth();
    if(depth_comparison)
      return depth_comparison;
    const int axis_comparison = compare_axis(rhs);
    if(axis_comparison)
      return axis_comparison;
    return compare_value(rhs);
  }

  virtual int get_depth() const = 0;
  virtual int compare_axis(const Feature &rhs) const = 0;
  virtual int compare_value(const Feature &rhs) const = 0;

  virtual std::vector<Feature *> refined() const {return std::vector<Feature *>();}

  virtual void print(std::ostream &os) const = 0;

  std::string to_string() const {
    std::ostringstream oss;
    print(oss);
    return oss.str();
  }

  List features;
};

inline std::ostream & operator<<(std::ostream &os, const Feature &feature) {
  feature.print(os);
  return os;
}

class Feature_Enumerated_Data {
  Feature_Enumerated_Data(const Feature_Enumerated_Data &) = delete;
  Feature_Enumerated_Data & operator=(const Feature_Enumerated_Data &) = delete;

public:
  Feature_Enumerated_Data(const size_t &value_)
   : value(value_)
  {
  }

  int compare_value(const Feature_Enumerated_Data &rhs) const {
    return value > rhs.value ? 1 : value < rhs.value ? -1 : 0;
  }

  Rete::Rete_Predicate::Predicate predicate() const {
    return Rete::Rete_Predicate::EQ;
  }

  Rete::Symbol_Ptr_C symbol_constant() const {
    return std::make_shared<Rete::Symbol_Constant_Int>(value);
  }

  size_t value;
};

template <typename FEATURE>
class Feature_Enumerated : public FEATURE, public Feature_Enumerated_Data {
  Feature_Enumerated(const Feature_Enumerated &) = delete;
  Feature_Enumerated & operator=(const Feature_Enumerated &) = delete;

public:
  Feature_Enumerated(const size_t &value_)
   : Feature_Enumerated_Data(value_)
  {
  }

  int get_depth() const {return 1;}

  int compare_value(const Feature &rhs) const {
    return Feature_Enumerated_Data::compare_value(debuggable_cast<const Feature_Enumerated &>(rhs));
  }
};

class Feature_Ranged_Data {
public:
  Feature_Ranged_Data(const Rete::WME_Token_Index &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_, const bool &integer_locked_)
   : axis(axis_),
   bound_lower(bound_lower_),
   bound_upper(bound_upper_),
   depth(depth_),
   upper(upper_),
   integer_locked(integer_locked_)
  {
  }

  double midpt() const {
    const double mpt = (bound_lower + bound_upper) / 2.0;
    return integer_locked ? floor(mpt) : mpt;
  }

  int compare_axis(const Feature_Ranged_Data &rhs) const {
    return axis.first != rhs.axis.first ? int(axis.first) - int(rhs.axis.first) : int(axis.second) - int(rhs.axis.second);
  }

  int compare_value(const Feature_Ranged_Data &rhs) const {
    return upper - rhs.upper;
  }

  void print(std::ostream &os) const {
    os << axis << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
  }

  Rete::Rete_Predicate::Predicate predicate() const {
    return upper ? Rete::Rete_Predicate::GTE : Rete::Rete_Predicate::LT;
  }

  Rete::Symbol_Ptr_C symbol_constant() const {
    return std::make_shared<Rete::Symbol_Constant_Float>(upper ? bound_lower : bound_upper);
  }

  Rete::WME_Token_Index axis;

  double bound_lower; ///< inclusive
  double bound_upper; ///< exclusive

  size_t depth; ///< 0 indicates unsplit

  bool upper; ///< Is this the upper half (same bound_upper) or lower half (same bound_lower) of a split?
  bool integer_locked; ///< Is this restricted to integer values?
};

template <typename FEATURE>
class Feature_Ranged : public FEATURE, public Feature_Ranged_Data {
  Feature_Ranged(const Feature_Ranged &) = delete;
  Feature_Ranged & operator=(const Feature_Ranged &) = delete;

public:
  Feature_Ranged(const Rete::WME_Token_Index &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_, const bool &integer_locked_)
   : Feature_Ranged_Data(axis_, bound_lower_, bound_upper_, depth_, upper_, integer_locked_)
  {
  }

  virtual Feature_Ranged * clone() const = 0;

  int get_depth() const {return depth;}

  int compare_value(const Feature &rhs) const {
    return Feature_Ranged_Data::compare_value(debuggable_cast<const Feature_Ranged &>(rhs));
  }

  std::vector<Feature *> refined() const {
    std::vector<Feature *> refined_features;

    const double mpt = midpt();

    if(bound_lower != mpt) {
      const auto refined_feature = clone();
      refined_feature->bound_upper = mpt;
      ++refined_feature->depth;
      refined_feature->upper = false;
      refined_features.push_back(refined_feature);
    }

    if(mpt != bound_upper) {
      const auto refined_feature = clone();
      refined_feature->bound_lower = mpt;
      ++refined_feature->depth;
      refined_feature->upper = true;
      refined_features.push_back(refined_feature);
    }

    return refined_features;
  }

  void print(std::ostream &os) const {
    Feature_Ranged_Data::print(os);
  }
};

#endif
