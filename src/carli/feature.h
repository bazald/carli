#ifndef CARLI_FEATURE_H
#define CARLI_FEATURE_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

#include "rete/rete.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>

#include "linkage.h"

namespace Carli {

  class CARLI_LINKAGE Feature : public Zeni::Pool_Allocator<char> {
    Feature(const Feature &) = delete;
    Feature & operator=(const Feature &) = delete;

  public:
    Feature(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const Rete::Variable_Indices_Ptr_C &indices_, const int64_t &arity_)
     : conditions(conditions_), bindings(bindings_), axis(axis_), indices(indices_), arity(arity_)
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
//      if(conditions < rhs.conditions)
//        return -1;
//      if(conditions > rhs.conditions)
//        return 1;
//
//      if(bindings.size() != rhs.bindings.size())
//        return bindings.size() < rhs.bindings.size() ? -1 : 1;
//      for(auto lt = bindings.begin(), lend = bindings.end(), rt = rhs.bindings.begin(); lt != lend; ++lt, ++rt) {
//        auto llv = Rete::get_Variable_name(indices, lt->first);
//        auto lrv = Rete::get_Variable_name(rhs.indices, rt->first);
//        if(llv != lrv)
//          return std::strcmp(llv.c_str(), lrv.c_str());
//        if(llv.empty()) {
//          if(lt->first < rt->first)
//            return -1;
//          if(lt->first > rt->first)
//            return 1;
//        }
////        if(lt->first.column != rt->first.column)
////          return lt->first.column - rt->first.column;
////        const auto offset_l = lt->first.rete_row - rt->first.rete_row;
////        const auto offset_r = lt->second.rete_row - rt->second.rete_row;
////        if(offset_l != offset_r)
////          return offset_l - offset_r;
//
//        auto rlv = Rete::get_Variable_name(indices, lt->second);
//        auto rrv = Rete::get_Variable_name(rhs.indices, rt->second);
//        if(rlv != rrv)
//          return std::strcmp(rlv.c_str(), rrv.c_str());
//        if(rlv.empty()) {
//          if(lt->second < rt->second)
//            return -1;
//          if(lt->second > rt->second)
//            return 1;
//        }
//      }
//
//      return 0;

      return conditions < rhs.conditions ? -1 : conditions > rhs.conditions ? 1 :
             bindings < rhs.bindings ? -1 : bindings > rhs.bindings ? 1 : 0;
    }

    virtual int64_t get_depth() const = 0;
    virtual int64_t compare_value(const Feature &rhs) const = 0;

    virtual std::vector<Feature *> refined() const {return std::vector<Feature *>();}

    virtual void print(std::ostream &os) const = 0;

    void print_axis(std::ostream &os) const {
//      os << '{';
//      for(size_t i = 0; i != conditions.size(); ++i)
//        os << (i ? "," : "") << conditions[i];
//      os << '}';

//      os << bindings;

      if(axis.rete_row == -1) {
        for(const auto &binding : bindings) {
          if(binding.second.rete_row != -1)
            os << Rete::get_Variable_name(indices, binding.first) << '.';
        }
        os << *conditions.rbegin()->symbols[1];
      }
      else
        os << Rete::get_Variable_name(indices, axis);
    }

    std::string to_string() const {
      std::ostringstream oss;
      print(oss);
      return oss.str();
    }

    std::vector<Rete::WME> conditions;
    Rete::WME_Bindings bindings;
    Rete::WME_Token_Index axis;
    Rete::Variable_Indices_Ptr_C indices;
    int64_t arity = -1;
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

    Rete::Rete_Predicate::Predicate get_predicate() const {
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
    Feature_Enumerated(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const Rete::Variable_Indices_Ptr_C &indices_, const int &arity_, const int64_t &value_)
     : FEATURE(conditions_, bindings_, axis_, indices_, arity_),
     Feature_Enumerated_Data(value_)
    {
    }

    Feature_Enumerated * clone() const override {
      return new Feature_Enumerated(this->conditions, this->bindings, this->axis, this->indices, this->arity, value);
    }

    int64_t get_depth() const override {return 1;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_Enumerated_Data::compare_value(debuggable_cast<const Feature_Enumerated &>(rhs));
    }

    void print(std::ostream &os) const override {
      this->print_axis(os);
      os << '=' << value;
    }
  };

  class CARLI_LINKAGE Feature_Ranged_Data {
  public:
    Feature_Ranged_Data(const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const Rete::Rete_Predicate::Predicate &predicate_, const bool &integer_locked_)
     : bound_lower(bound_lower_),
     bound_upper(bound_upper_),
     depth(depth_),
     predicate(predicate_),
     integer_locked(integer_locked_)
    {
    }

    double midpt() const {
      const double mpt = (bound_lower + bound_upper) / 2.0;
      return integer_locked ? std::floor(mpt) : mpt;
    }

    int64_t compare_value(const Feature_Ranged_Data &rhs) const {
      return predicate - rhs.predicate;
    }

    Rete::Rete_Predicate::Predicate get_predicate() const {
      return predicate;
    }

    Rete::Symbol_Ptr_C symbol_constant() const {
      double value;

      switch(predicate) {
        case Rete::Rete_Predicate::GT:
        case Rete::Rete_Predicate::GTE:
          value = bound_lower;
          break;

        case Rete::Rete_Predicate::LT:
        case Rete::Rete_Predicate::LTE:
          value = bound_upper;
          break;

        default:
          abort();
      }

      if(integer_locked)
        return std::make_shared<Rete::Symbol_Constant_Int>(int64_t(value));
      else
        return std::make_shared<Rete::Symbol_Constant_Float>(value);
    }

    double bound_lower; ///< inclusive
    double bound_upper; ///< exclusive

    int64_t depth; ///< 0 indicates unsplit

    Rete::Rete_Predicate::Predicate predicate; ///< Is this the upper half (same bound_upper) or lower half (same bound_lower) of a split?
    bool integer_locked; ///< Is this restricted to integer values?
  };

  template <typename FEATURE = Feature>
  class Feature_Ranged : public FEATURE, public Feature_Ranged_Data {
    Feature_Ranged(const Feature_Ranged &) = delete;
    Feature_Ranged & operator=(const Feature_Ranged &) = delete;

  public:
    Feature_Ranged(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const Rete::Variable_Indices_Ptr_C &indices_, const int &arity_, const double &bound_lower_, const double &bound_upper_, const int64_t &depth_, const Rete::Rete_Predicate::Predicate &predicate_, const bool &integer_locked_)
     : FEATURE(conditions_, bindings_, axis_, indices_, arity_),
     Feature_Ranged_Data(bound_lower_, bound_upper_, depth_, predicate_, integer_locked_)
    {
    }

    Feature_Ranged * clone() const override {
      return new Feature_Ranged(this->conditions, this->bindings, this->axis, this->indices, this->arity, bound_lower, bound_upper, depth, predicate, integer_locked);
    }

    int64_t get_depth() const override {return depth;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_Ranged_Data::compare_value(debuggable_cast<const Feature_Ranged &>(rhs));
    }

    std::vector<Carli::Feature *> refined() const override {
      std::vector<Carli::Feature *> refined_features;

      const double mpt = midpt();

      Rete::Rete_Predicate::Predicate upper_predicate, lower_predicate;
      switch(predicate) {
        case Rete::Rete_Predicate::GT:
        case Rete::Rete_Predicate::LTE:
          upper_predicate = Rete::Rete_Predicate::GT;
          lower_predicate = Rete::Rete_Predicate::LTE;
          break;

        case Rete::Rete_Predicate::GTE:
        case Rete::Rete_Predicate::LT:
          upper_predicate = Rete::Rete_Predicate::GTE;
          lower_predicate = Rete::Rete_Predicate::LT;
          break;

        default:
          abort();
      }

      if(bound_lower != mpt) {
        {
          const auto refined_feature = clone();
          refined_feature->bound_upper = mpt;
          ++refined_feature->depth;
          refined_feature->predicate = lower_predicate;
          refined_features.push_back(refined_feature);
        }

        {
          const auto refined_feature = clone();
          refined_feature->bound_lower = mpt;
          ++refined_feature->depth;
          refined_feature->predicate = upper_predicate;
          refined_features.push_back(refined_feature);
        }
      }

      return refined_features;
    }

    void print(std::ostream &os) const override {
      this->print_axis(os);
      os << '=';
      if(integer_locked && bound_lower + 1 == bound_upper)
        os << bound_lower;
      else
        os << '[' << bound_lower << ',' << bound_upper << ')';
      os << ':' << depth;
    }
  };

  class CARLI_LINKAGE Feature_NullHOG_Data {
    Feature_NullHOG_Data(const Feature_NullHOG_Data &) = delete;
    Feature_NullHOG_Data & operator=(const Feature_NullHOG_Data &) = delete;

  public:
    Feature_NullHOG_Data(const std::string &value_)
     : value(value_)
    {
    }

    int64_t compare_value(const Feature_NullHOG_Data &rhs) const {
      return std::strcmp(value.c_str(), rhs.value.c_str());
    }

    Rete::Rete_Predicate::Predicate get_predicate() const {
      return Rete::Rete_Predicate::NEQ;
    }

    Rete::Symbol_Ptr_C symbol_constant() const {
      return std::make_shared<Rete::Symbol_Constant_String>(value);
    }

    std::string value;
  };

  template <typename FEATURE = Feature>
  class Feature_NullHOG : public FEATURE, public Feature_NullHOG_Data {
    Feature_NullHOG(const Feature_NullHOG &) = delete;
    Feature_NullHOG & operator=(const Feature_NullHOG &) = delete;

  public:
    Feature_NullHOG(const std::vector<Rete::WME> &conditions_, const Rete::WME_Bindings &bindings_, const Rete::WME_Token_Index &axis_, const Rete::Variable_Indices_Ptr_C &indices_, const int &arity_, const std::string &value_)
     : FEATURE(conditions_, bindings_, axis_, indices_, arity_),
     Feature_NullHOG_Data(value_)
    {
    }

    Feature_NullHOG * clone() const override {
      return new Feature_NullHOG(this->conditions, this->bindings, this->axis, this->indices, this->arity, value);
    }

    int64_t get_depth() const override {return 1;}

    int64_t compare_value(const Carli::Feature &rhs) const override {
      return Feature_NullHOG_Data::compare_value(debuggable_cast<const Feature_NullHOG &>(rhs));
    }

    void print(std::ostream &os) const override {
      this->print_axis(os);
      os << "!=" << value;
    }
  };

}

#endif
