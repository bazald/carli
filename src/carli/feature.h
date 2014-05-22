#ifndef CARLI_FEATURE_H
#define CARLI_FEATURE_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

#include "rete/rete.h"

#include <iostream>
#include <memory>
#include <sstream>

#include "linkage.h"

namespace Carli {

  class CARLI_LINKAGE Feature : public Zeni::Pool_Allocator<char> {
    Feature(const Feature &) = delete;
    Feature & operator=(const Feature &) = delete;

  public:
    Feature() {}

    virtual ~Feature() {}

    virtual Feature * clone() const = 0;

    bool operator<(const Feature &rhs) const {return compare(rhs) < 0;}
    bool operator<=(const Feature &rhs) const {return compare(rhs) <= 0;}
    bool operator>(const Feature &rhs) const {return compare(rhs) > 0;}
    bool operator>=(const Feature &rhs) const {return compare(rhs) >= 0;}
    bool operator==(const Feature &rhs) const {return compare(rhs) == 0;}
    bool operator!=(const Feature &rhs) const {return compare(rhs) != 0;}

    virtual int64_t compare(const Feature &rhs) const {
      const int64_t depth_comparison = get_depth() - rhs.get_depth();
      if(depth_comparison)
        return depth_comparison;
      const int64_t axis_comparison = compare_axis(rhs);
      if(axis_comparison)
        return axis_comparison;
      return compare_value(rhs);
    }

    virtual int64_t get_depth() const = 0;
    virtual int64_t compare_axis(const Feature &rhs) const = 0;
    virtual int64_t compare_value(const Feature &rhs) const = 0;

    virtual bool matches(const Rete::WME_Token &token) const = 0;

    virtual Rete::WME_Bindings bindings() const {return Rete::WME_Bindings();}

    virtual Rete::WME_Token_Index wme_token_index() const = 0;

    virtual std::vector<Feature *> refined() const {return std::vector<Feature *>();}

    virtual void print(std::ostream &os) const = 0;

    std::string to_string() const {
      std::ostringstream oss;
      print(oss);
      return oss.str();
    }

    bool detected = false;
  };

}

inline std::ostream & operator<<(std::ostream &os, const Carli::Feature &feature) {
  feature.print(os);
  return os;
}

namespace Carli {

  class CARLI_LINKAGE Feature_Enumerated_Data {
    Feature_Enumerated_Data(const Feature_Enumerated_Data &) = delete;
    Feature_Enumerated_Data & operator=(const Feature_Enumerated_Data &) = delete;

  public:
    Feature_Enumerated_Data(const Rete::WME_Token_Index &axis_, const int64_t &value_)
     : axis(axis_), value(value_)
    {
    }

    int64_t compare_value(const Feature_Enumerated_Data &rhs) const {
      return value > rhs.value ? 1 : value < rhs.value ? -1 : 0;
    }

    Rete::Rete_Predicate::Predicate predicate() const {
      return Rete::Rete_Predicate::EQ;
    }

    Rete::Symbol_Ptr_C symbol_constant() const {
      return std::make_shared<Rete::Symbol_Constant_Int>(value);
    }

    Rete::WME_Token_Index axis;

    int64_t value;
  };

  template <typename FEATURE>
  class Feature_Enumerated : public FEATURE, public Feature_Enumerated_Data {
    Feature_Enumerated(const Feature_Enumerated &) = delete;
    Feature_Enumerated & operator=(const Feature_Enumerated &) = delete;

  public:
    Feature_Enumerated(const Rete::WME_Token_Index &axis_, const int64_t &value_)
     : Feature_Enumerated_Data(axis_, value_)
    {
    }

    int64_t get_depth() const override {return 1;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_Enumerated_Data::compare_value(debuggable_cast<const Feature_Enumerated &>(rhs));
    }

    bool matches(const Rete::WME_Token &token) const override {
      return *token[this->wme_token_index()] == value;
    }

    Rete::WME_Token_Index wme_token_index() const override {
      return axis;
    }
  };

  class CARLI_LINKAGE Feature_Ranged_Data {
  public:
    Feature_Ranged_Data(const Rete::WME_Token_Index &axis_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_, const bool &integer_locked_)
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

    int64_t compare_axis(const Feature_Ranged_Data &rhs) const {
      return axis.first != rhs.axis.first ? axis.first - rhs.axis.first : axis.second - rhs.axis.second;
    }

    int64_t compare_value(const Feature_Ranged_Data &rhs) const {
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

    int64_t depth; ///< 0 indicates unsplit

    bool upper; ///< Is this the upper half (same bound_upper) or lower half (same bound_lower) of a split?
    bool integer_locked; ///< Is this restricted to integer values?
  };

  template <typename FEATURE>
  class Feature_Ranged : public FEATURE, public Feature_Ranged_Data {
    Feature_Ranged(const Feature_Ranged &) = delete;
    Feature_Ranged & operator=(const Feature_Ranged &) = delete;

  public:
    Feature_Ranged(const Rete::WME_Token_Index &axis_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_, const bool &integer_locked_)
     : Feature_Ranged_Data(axis_, bound_lower_, bound_upper_, depth_, upper_, integer_locked_)
    {
    }

    virtual Feature_Ranged * clone() const override = 0;

    int64_t get_depth() const override {return depth;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_Ranged_Data::compare_value(debuggable_cast<const Feature_Ranged &>(rhs));
    }

    bool matches(const Rete::WME_Token &token) const override {
      const Rete::Symbol &symbol = debuggable_cast<const Rete::Symbol &>(*token[wme_token_index()]);
      const Rete::Symbol_Constant_Float &symbol_cf = debuggable_cast<const Rete::Symbol_Constant_Float &>(symbol);
      return bound_lower <= symbol_cf.value && symbol_cf.value < bound_upper;
    }

    std::vector<Carli::Feature *> refined() const override {
      std::vector<Carli::Feature *> refined_features;

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

    Rete::WME_Token_Index wme_token_index() const override {
      return axis;
    }

    void print(std::ostream &os) const override {
      Feature_Ranged_Data::print(os);
    }
  };

}

#endif
