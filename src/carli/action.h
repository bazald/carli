#ifndef ACTION_H
#define ACTION_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

#include <memory>

class Action : public Zeni::Pool_Allocator<char> {
  Action(const Action &) = delete;
  Action & operator=(const Action &) = delete;

public:
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;

  Action()
    : candidates(this)
  {
  }

  virtual ~Action() {}

  virtual Action * clone() const = 0;

  bool operator<(const Action &rhs) const {return compare(rhs) < 0;}
  bool operator<=(const Action &rhs) const {return compare(rhs) <= 0;}
  bool operator>(const Action &rhs) const {return compare(rhs) > 0;}
  bool operator>=(const Action &rhs) const {return compare(rhs) >= 0;}
  bool operator==(const Action &rhs) const {return compare(rhs) == 0;}
  bool operator!=(const Action &rhs) const {return compare(rhs) != 0;}

  void print(std::ostream &os) const {
    print_impl(os);
  }

  virtual int compare(const Action &rhs) const = 0;

  virtual void print_impl(std::ostream &os) const = 0;

  List candidates;
};

std::ostream & operator << (std::ostream &os, const Action &action) {
  action.print(os);
  return os;
}

#endif
