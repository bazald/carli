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
    Feature(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const std::string &axis_label_)
     : conditions(conditions_), bindings(bindings_), axis(axis_), axis_label(axis_label_)
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

    virtual int64_t compare(const Feature &rhs) const {
      const int64_t depth_comparison = get_depth() - rhs.get_depth();
      if(depth_comparison)
        return depth_comparison;
      const int64_t axis_comparison = compare_axis(rhs);
      if(axis_comparison)
        return axis_comparison;
      return compare_value(rhs);
    }

    int64_t compare_axis(const Feature &rhs) const {
      return conditions < rhs.conditions ? -1 : conditions > rhs.conditions ? 1 :
             bindings < rhs.bindings ? -1 : bindings > rhs.bindings ? 1 :
             strcmp(axis_label.c_str(), rhs.axis_label.c_str());
    }

    virtual int64_t get_depth() const = 0;
    virtual int64_t compare_value(const Feature &rhs) const = 0;

    virtual bool matches(const Rete::WME_Token &token) const = 0;

    virtual std::vector<Feature *> refined() const {return std::vector<Feature *>();}

    virtual void print(std::ostream &os) const = 0;

    void print_axis(std::ostream &os) const {
      os << '{';
      for(size_t i = 0; i != conditions.size(); ++i)
        os << (i ? "," : "") << conditions[i];
      os << '}' << bindings << axis << axis_label;
    }

    std::string to_string() const {
      std::ostringstream oss;
      print(oss);
      return oss.str();
    }

    std::vector<Rete::WME> conditions;
    Rete::WME_Bindings bindings;
    Rete::WME_Token_Index axis;
    std::string axis_label;
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
    Feature_Enumerated_Data(const int64_t &value_)
     : value(value_)
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

    int64_t value;
  };

  template <typename FEATURE = Feature>
  class Feature_Enumerated : public FEATURE, public Feature_Enumerated_Data {
    Feature_Enumerated(const Feature_Enumerated &) = delete;
    Feature_Enumerated & operator=(const Feature_Enumerated &) = delete;

  public:
    Feature_Enumerated(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const std::string &axis_label_, const int64_t &value_)
     : FEATURE(conditions_, bindings_, axis_, axis_label_),
     Feature_Enumerated_Data(value_)
    {
    }

    Feature_Enumerated * clone() const override {
      return new Feature_Enumerated(this->conditions, this->bindings, this->axis, this->axis_label, value);
    }

    int64_t get_depth() const override {return 1;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_Enumerated_Data::compare_value(debuggable_cast<const Feature_Enumerated &>(rhs));
    }

    bool matches(const Rete::WME_Token &token) const override {
      return *token[this->axis] == value;
    }

    void print(std::ostream &os) const override {
      this->print_axis(os);
      os << '=' << value;
    }
  };

  class CARLI_LINKAGE Feature_Ranged_Data {
  public:
    Feature_Ranged_Data(const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_, const bool &integer_locked_)
     : bound_lower(bound_lower_),
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

    int64_t compare_value(const Feature_Ranged_Data &rhs) const {
      return upper - rhs.upper;
    }

    Rete::Rete_Predicate::Predicate predicate() const {
      return upper ? Rete::Rete_Predicate::GTE : Rete::Rete_Predicate::LT;
    }

    Rete::Symbol_Ptr_C symbol_constant() const {
      return std::make_shared<Rete::Symbol_Constant_Float>(upper ? bound_lower : bound_upper);
    }

    double bound_lower; ///< inclusive
    double bound_upper; ///< exclusive

    int64_t depth; ///< 0 indicates unsplit

    bool upper; ///< Is this the upper half (same bound_upper) or lower half (same bound_lower) of a split?
    bool integer_locked; ///< Is this restricted to integer values?
  };

  template <typename FEATURE = Feature>
  class Feature_Ranged : public FEATURE, public Feature_Ranged_Data {
    Feature_Ranged(const Feature_Ranged &) = delete;
    Feature_Ranged & operator=(const Feature_Ranged &) = delete;

  public:
    Feature_Ranged(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const std::string &axis_label_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const bool &upper_, const bool &integer_locked_)
     : FEATURE(conditions_, bindings_, axis_, axis_label_),
     Feature_Ranged_Data(bound_lower_, bound_upper_, depth_, upper_, integer_locked_)
    {
    }

    Feature_Ranged * clone() const override {
      return new Feature_Ranged(this->conditions, this->bindings, this->axis, this->axis_label, bound_lower, bound_upper, depth, upper, integer_locked);
    }

    int64_t get_depth() const override {return depth;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_Ranged_Data::compare_value(debuggable_cast<const Feature_Ranged &>(rhs));
    }

    bool matches(const Rete::WME_Token &token) const override {
      const Rete::Symbol * const symbol = debuggable_cast<const Rete::Symbol * const>(token[this->axis].get());
      if(const auto symbol_cf = dynamic_cast<const Rete::Symbol_Constant_Float *>(symbol))
        return bound_lower <= symbol_cf->value && symbol_cf->value < bound_upper;
      else {
        const auto &symbol_ci = debuggable_cast<const Rete::Symbol_Constant_Int &>(*symbol);
        return bound_lower <= symbol_ci.value && symbol_ci.value < bound_upper;
      }
    }

    std::vector<Carli::Feature *> refined() const override {
      std::vector<Carli::Feature *> refined_features;

      const double mpt = midpt();

      if(bound_lower != mpt) {
        {
          const auto refined_feature = clone();
          refined_feature->bound_upper = mpt;
          ++refined_feature->depth;
          refined_feature->upper = false;
          refined_features.push_back(refined_feature);
        }

        {
          const auto refined_feature = clone();
          refined_feature->bound_lower = mpt;
          ++refined_feature->depth;
          refined_feature->upper = true;
          refined_features.push_back(refined_feature);
        }
      }

      return refined_features;
    }

    void print(std::ostream &os) const {
      this->print_axis(os);
      os << '=';
      if(integer_locked && bound_lower + 1 == bound_upper)
        os << bound_lower;
      else
        os << '[' << bound_lower << ',' << bound_upper << ')';
      os << ':' << depth;
    }
  };

}

#endif
