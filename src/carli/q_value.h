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
    typedef std::tuple<double, double, double, double, int64_t> Token;

    Q_Value(const Token value, const Type &type_, const int64_t &depth_, const tracked_ptr<Feature> &feature_, const int64_t &creation_time_)
     : creation_time(creation_time_),
     depth(depth_),
     update_count(std::get<4>(value)),
     type(type_),
     primary(std::get<0>(value)),
     primary_mean2(std::get<1>(value)),
     primary_variance(std::get<2>(value)),
     secondary(std::get<3>(value)),
     eligible(this),
     feature(feature_)
    {
      update_totals();
    }

    Q_Value * clone() const {
      Q_Value * const lhs = new Q_Value(Token(primary, primary_mean2, primary_variance, secondary, update_count),
                                        type, depth, feature->clone(), creation_time);

      lhs->last_episode_fired = last_episode_fired;
      lhs->last_step_fired = last_step_fired;
      lhs->pseudoepisode_count = pseudoepisode_count;

      lhs->depth = depth;

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

    void update_totals() {
      primary = normalize(primary);
      primary_variance = normalize(primary_variance);
      secondary = normalize(secondary);
    }

    int64_t creation_time = 0;

    int64_t last_episode_fired = std::numeric_limits<int64_t>::max();
    int64_t last_step_fired = std::numeric_limits<int64_t>::max();
    int64_t pseudoepisode_count = 0;

    int64_t depth;
    int64_t update_count;

    Type type;

    /** Not cloned **/
    bool eligibility_init = false;
    double eligibility = -1.0;
    double credit = 1.0;
  //  double weight = 1.0;

    double primary = 0.0;
    double primary_mean2 = 0.0;
    double primary_variance = 0.0;

    double secondary = 0.0;

    Value catde; ///< Cumulative Absolute Bellman Error
    Value matde; ///< Mean Absolute Bellman Error (catde / update_count)

#ifdef WHITESON_ADAPTIVE_TILE
    double minbe = DBL_MAX; ///< Minimum Bellman Error experienced
#endif

    double t0; ///< temp "register"

    List eligible;

    tracked_ptr<Feature> feature;

  private:
    static double normalize(const double &input) {
      switch(std::fpclassify(input)) {
        case FP_NORMAL:   return input;
        case FP_INFINITE: return input > 0.0 ? std::numeric_limits<double>::max() : std::numeric_limits<double>::lowest();
        default:          return 0.0;
      }
    }
  };

}

#endif
