#ifndef CARLI_Q_VALUE_H
#define CARLI_Q_VALUE_H

#include "utility/memory_pool.h"
#include "utility/linked_list.h"

#include "value.h"

#include <cfloat>
#include <limits>
#include <memory>

namespace Carli {

  class Node_Fringe;

  class Q_Value;
  class CARLI_LINKAGE Q_Value : public Zeni::Pool_Allocator<Q_Value>, public std::enable_shared_from_this<Q_Value> {
    Q_Value(const Q_Value &) = delete;
    Q_Value & operator=(const Q_Value &) = delete;

  public:
    typedef Zeni::Linked_List<Q_Value> List;
    typedef List::iterator iterator;
    enum class Type : char {SPLIT, UNSPLIT, FRINGE};

    Q_Value(const double &q_value_, const Type &type_, const int64_t &depth_, const tracked_ptr<Feature> &feature_)
     : depth(depth_),
     type(type_),
     value(q_value_),
     eligible(this),
     feature(feature_)
    {
    }

    Q_Value * clone() const {
      Q_Value * const lhs = new Q_Value(value, type, depth, feature->clone());

      lhs->last_episode_fired = last_episode_fired;
      lhs->last_step_fired = last_step_fired;
      lhs->pseudoepisode_count = pseudoepisode_count;

      lhs->depth = depth;
      lhs->update_count = update_count;

      lhs->catde = catde;
      lhs->matde = matde;
      
#ifdef WHITESON_ADAPTIVE_TILE
      lhs->minbe = minbe;
#endif

      return lhs;
    }

    ~Q_Value() {
      feature.delete_and_zero();
    }

    Q_Value & operator=(const double &q_value_) {
      value = q_value_;
      return *this;
    }

    int64_t last_episode_fired = std::numeric_limits<int64_t>::max();
    int64_t last_step_fired = std::numeric_limits<int64_t>::max();
    int64_t pseudoepisode_count = 0;

    int64_t depth;
    int64_t update_count = 0;

    Type type;

    /** Not cloned **/
    bool eligibility_init = false;
    double eligibility = -1.0;
    double credit = 1.0;
  //  double weight = 1.0;

    double value;
    Value catde; ///< Cumulative Absolute Bellman Error
    Value matde; ///< Mean Absolute Bellman Error (catde / update_count)

#ifdef WHITESON_ADAPTIVE_TILE
    double minbe = DBL_MAX; ///< Minimum Bellman Error experienced
#endif

    double t0; ///< temp "register"

    List eligible;

    tracked_ptr<Feature> feature;
  };

}

#endif
