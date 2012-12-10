#ifndef Q_VALUE_H
#define Q_VALUE_H

#include "memory_pool.h"
#include "clone.h"
#include "value.h"
#include "linked_list.h"

#include <cfloat>

class Q_Value;
class Q_Value : public Zeni::Pool_Allocator<Q_Value> {
  Q_Value(const Q_Value &);
  Q_Value & operator=(const Q_Value &);

public:
  typedef Zeni::Linked_List<Q_Value> List;
  typedef List::iterator iterator;
  enum Type {SPLIT, UNSPLIT, FRINGE};

  Q_Value(const double &q_value_ = double(), const Type &type_ = UNSPLIT)
   : last_episode_fired(size_t(-1)),
   last_step_fired(size_t(-1)),
   pseudoepisode_count(0),
   update_count(0),
   type(type_),
   eligibility_init(false),
   eligibility(-1.0),
   credit(1.0),
   value(q_value_),
#ifdef WHITESON_ADAPTIVE_TILE
   minbe(DBL_MAX),
#endif
#ifdef TRACK_Q_VALUE_VARIANCE
   mean2(0),
   variance_0(0),
   variance_rest(0),
#endif
   eligible(this),
   current(this),
   next(this)
  {
  }

  Q_Value & operator=(const double &q_value_) {
    value = q_value_;
    return *this;
  }

  size_t last_episode_fired;
  size_t last_step_fired;

  size_t pseudoepisode_count;
  size_t update_count;

  Type type;

  bool eligibility_init;
  double eligibility;
  double credit;

  double value;
  Value cabe; ///< Cumulative Absolute Bellman Error
  Value mabe; ///< Mean Absolute Bellman Error (cabe / update_count)

#ifdef WHITESON_ADAPTIVE_TILE
  double minbe; ///< Minimum Bellman Error experienced
#endif

#ifdef TRACK_Q_VALUE_VARIANCE
  double mean2;
  double variance_0;
  double variance_rest;
  Value variance_total;
#endif

  double t0; ///< temp "register"

  List eligible;
  List current;
  List next;

  static size_t eligible_offset() {
    union {
      const List Q_Value:: * m;
      const char * c;
    } u;
    
    u.c = nullptr;
    u.m = &Q_Value::eligible;
    return u.c - static_cast<char *>(nullptr);
  }

  static size_t current_offset() {
    union {
      const List Q_Value:: * m;
      const char * c;
    } u;
    
    u.c = nullptr;
    u.m = &Q_Value::current;
    return u.c - static_cast<char *>(nullptr);
  }

  static size_t next_offset() {
    union {
      const List Q_Value:: * m;
      const char * c;
    } u;
    
    u.c = nullptr;
    u.m = &Q_Value::next;
    return u.c - static_cast<char *>(nullptr);
  }
};

#endif
