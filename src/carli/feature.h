#ifndef FEATURE_H
#define FEATURE_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

#include "rete/rete.h"

#include <iostream>
#include <memory>

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
    const int axis_comparison = compare_axis(rhs);
    return axis_comparison ? axis_comparison : compare_value(rhs);
  }

  virtual int compare_axis(const Feature &rhs) const = 0;
  virtual int compare_value(const Feature &rhs) const = 0;

  virtual std::vector<Feature *> refined() const {return std::vector<Feature *>();}
  virtual Rete::Rete_Predicate::Predicate predicate() const = 0;
  virtual Rete::Symbol_Ptr_C symbol_constant() const = 0;

  virtual void print(std::ostream &os) const = 0;

  List features;
};

inline std::ostream & operator<<(std::ostream &os, const Feature &feature) {
  feature.print(os);
  return os;
}

class Feature_Present : public Feature {
  Feature_Present(const Feature_Present &) = delete;
  Feature_Present & operator=(const Feature_Present &) = delete;

public:
  Feature_Present(const bool &present_)
   : present(present_)
  {
  }

  int compare_value(const Feature &rhs) const {
    return present - debuggable_cast<const Feature_Present &>(rhs).present;
  }

  Rete::Rete_Predicate::Predicate predicate() const {
    return Rete::Rete_Predicate::EQ;
  }

  bool present;
};

class Feature_Ranged_Data {
public:
  Feature_Ranged_Data(const Rete::WME_Token_Index &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
   : axis(axis_),
   bound_lower(bound_lower_),
   bound_upper(bound_upper_),
   depth(depth_),
   upper(upper_)
  {
  }

  double midpt() const {
    return (bound_lower + bound_upper) / 2.0;
  }

  int compare(const Feature_Ranged_Data &rhs) const {
    return depth != rhs.depth ? depth - rhs.depth :
           axis.first != rhs.axis.first ? int(axis.first) - int(rhs.axis.first) :
           axis.second != rhs.axis.second ? int(axis.second) - int(rhs.axis.second) :
           upper - rhs.upper;
  }

  int compare_axis(const Feature_Ranged_Data &rhs) const {
    return axis.first != rhs.axis.first ? int(axis.first) - int(rhs.axis.first) : int(axis.second) - int(rhs.axis.second);
  }

  int compare_value(const Feature_Ranged_Data &rhs) const {
    return depth != rhs.depth ? depth - rhs.depth :
           upper - rhs.upper;
  }

  void print(std::ostream &os) const {
    os << axis << '(' << bound_lower << ',' << bound_upper << ':' << depth << ')';
  }

  Rete::WME_Token_Index axis;

  double bound_lower; ///< inclusive
  double bound_upper; ///< exclusive

  size_t depth; ///< 0 indicates unsplit

  bool upper; ///< Is this the upper half (same bound_upper) or lower half (same bound_lower) of a split?
};

class Feature_Ranged : public Feature, public Feature_Ranged_Data {
  Feature_Ranged(const Feature_Ranged &) = delete;
  Feature_Ranged & operator=(const Feature_Ranged &) = delete;

public:
  Feature_Ranged(const Rete::WME_Token_Index &axis_, const double &bound_lower_, const double &bound_upper_, const size_t &depth_, const bool &upper_)
   : Feature_Ranged_Data(axis_, bound_lower_, bound_upper_, depth_, upper_)
  {
  }

  virtual Feature_Ranged * clone() const = 0;

  int compare(const Feature &rhs) const {
    return Feature_Ranged_Data::compare(debuggable_cast<const Feature_Ranged &>(rhs));
  }

  int compare_axis(const Feature &rhs) const {
    return Feature_Ranged_Data::compare_axis(debuggable_cast<const Feature_Ranged &>(rhs));
  }

  int compare_value(const Feature &rhs) const {
    return Feature_Ranged_Data::compare_value(debuggable_cast<const Feature_Ranged &>(rhs));
  }

  std::vector<Feature *> refined() const {
    std::vector<Feature *> refined_features;

    auto refined_feature = clone();
    refined_feature->bound_upper = refined_feature->midpt();
    ++refined_feature->depth;
    refined_feature->upper = false;
    refined_features.push_back(refined_feature);

    refined_feature = clone();
    refined_feature->bound_lower = refined_feature->midpt();
    ++refined_feature->depth;
    refined_feature->upper = true;
    refined_features.push_back(refined_feature);

    return refined_features;
  }

  void print(std::ostream &os) const {
    Feature_Ranged_Data::print(os);
  }

  Rete::Rete_Predicate::Predicate predicate() const {
    return upper ? Rete::Rete_Predicate::GTE : Rete::Rete_Predicate::LT;
  }

  Rete::Symbol_Ptr_C symbol_constant() const {
    return std::make_shared<Rete::Symbol_Constant_Float>(upper ? bound_lower : bound_upper);
  }
};

#endif
