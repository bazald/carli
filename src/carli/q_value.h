#ifndef Q_VALUE_H
#define Q_VALUE_H

#include "memory_pool.h"
#include "clone.h"
#include "value.h"
#include "linked_list.h"

class Q_Value;
class Q_Value : public Zeni::Pool_Allocator<Q_Value> {
  Q_Value(const Q_Value &);
  Q_Value & operator=(const Q_Value &);

public:
  typedef Zeni::Linked_List<Q_Value> List;
  typedef List::iterator iterator;

  Q_Value(const double &q_value_ = double())
   : update_count(0),
   split(false),
   credit(1.0),
   value(q_value_),
   current(this),
   next(this)
  {
  }

  Q_Value & operator=(const double &q_value_) {
    value = q_value_;
    return *this;
  }

  size_t update_count;

  bool split;

  double credit;

  Value value;
  Value cabe; ///< Cumulative Absolute Bellman Error

  double mean2;
  double variance_0;
  double variance_rest;
  Value variance_total;

  List current;
  List next;

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
