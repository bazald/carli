#ifndef Q_VALUE_H
#define Q_VALUE_H

#include "memory_pool.h"
#include "clone.h"
#include "value.h"
#include "linked_list.h"

class Q_Value;
class Q_Value : public Value, public Zeni::Pool_Allocator<Q_Value>, public Zeni::Cloneable<Q_Value> {
  Q_Value(const Q_Value &);
  Q_Value & operator=(const Q_Value &);

public:
  typedef Zeni::Linked_List<Q_Value> List;
  typedef List::iterator iterator;

  Q_Value(const double &q_value_ = double())
   : Value(q_value_),
   current(this),
   next(this)
  {
  }

  Q_Value * clone() const {
    return new Q_Value(double(*this));
  }

  Q_Value & operator=(const double &q_value_) {
    Value::operator=(q_value_);
    return *this;
  }

  List current;
  List next;

  static size_t current_offset() {
    union {
      const List Q_Value:: * m;
      const char * c;
    } u;
    
    u.m = &Q_Value::current;
    return u.c - static_cast<char *>(nullptr);
  }

  static size_t next_offset() {
    union {
      const List Q_Value:: * m;
      const char * c;
    } u;
    
    u.m = &Q_Value::next;
    return u.c - static_cast<char *>(nullptr);
  }
};

#endif
