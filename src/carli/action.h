#ifndef ACTION_H
#define ACTION_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

#include <memory>

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Action : public Zeni::Pool_Allocator<DERIVED2> {
  Action(const Action &) = delete;
  Action & operator=(const Action &) = delete;

public:
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;
  typedef DERIVED derived_type;

  Action()
    : candidates(this)
  {
  }

  virtual ~Action() {}

  virtual DERIVED * clone() const = 0;

  bool operator<(const Action &rhs) const {return compare(rhs) < 0;}
  bool operator<=(const Action &rhs) const {return compare(rhs) <= 0;}
  bool operator>(const Action &rhs) const {return compare(rhs) > 0;}
  bool operator>=(const Action &rhs) const {return compare(rhs) >= 0;}
  bool operator==(const Action &rhs) const {return compare(rhs) == 0;}
  bool operator!=(const Action &rhs) const {return compare(rhs) != 0;}

  void print(std::ostream &os) const {
    print_impl(os);
  }

  int compare(const Action &rhs) const {
    return static_cast<const DERIVED *>(this)->compare(static_cast<const DERIVED &>(rhs));
  }

  virtual void print_impl(std::ostream &os) const = 0;

  List candidates;
};

template <typename DERIVED, typename DERIVED2>
std::ostream & operator << (std::ostream &os, const Action<DERIVED, DERIVED2> &action) {
  action.print(os);
  return os;
}

#endif
