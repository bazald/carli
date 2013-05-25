#ifndef Q_VALUE_H
#define Q_VALUE_H

#include "memory_pool.h"
#include "value.h"
#include "linked_list.h"

#include <cfloat>
#include <limits>

class Q_Value;
class Q_Value : public Zeni::Pool_Allocator<Q_Value> {
  Q_Value(const Q_Value &) = delete;
  Q_Value & operator=(const Q_Value &) = delete;

public:
  typedef Zeni::Linked_List<Q_Value> List;
  typedef List::iterator iterator;
  enum class Type : char {SPLIT, UNSPLIT, FRINGE};

  Q_Value(const double &q_value_ = double(), const Type &type_ = Type::UNSPLIT)
   : type(type_),
   value(q_value_),
   eligible(this),
   current(this),
   next(this)
  {
  }

  Q_Value & operator=(const double &q_value_) {
    value = q_value_;
    return *this;
  }

  size_t last_episode_fired = std::numeric_limits<size_t>::max();
  size_t last_step_fired = std::numeric_limits<size_t>::max();
  size_t pseudoepisode_count = 0;

  size_t update_count = 1;

  Type type;

  bool eligibility_init = false;
  double eligibility = -1.0;
  double credit = 1.0;
  double weight = 1.0;

  double value;
  Value cabe; ///< Cumulative Absolute Bellman Error
  Value mabe; ///< Mean Absolute Bellman Error (cabe / update_count)

#ifdef WHITESON_ADAPTIVE_TILE
  double minbe = DBL_MAX; ///< Minimum Bellman Error experienced
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
