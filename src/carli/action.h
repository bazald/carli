#ifndef ACTION_H
#define ACTION_H

#include <memory>

#include "linked_list.h"
#include "memory_pool.h"

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Action : public Zeni::Pool_Allocator<DERIVED2> {
  Action(const Action &) = delete;
  Action & operator=(const Action &) = delete;

public:
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;

  struct Compare {
    bool operator()(const Action &lhs, const Action &rhs) const {
      return lhs < rhs;
    }
    bool operator()(const Action &lhs, const Action * const &rhs) const {return operator()(lhs, *rhs);}
    bool operator()(const Action &lhs, const std::shared_ptr<const Action> &rhs) const {return operator()(lhs, *rhs);}
    bool operator()(const Action &lhs, const std::unique_ptr<const Action> &rhs) const {return operator()(lhs, *rhs);}

    bool operator()(const Action * const &lhs, const Action &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const Action * const &lhs, const Action * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const Action * const &lhs, const std::shared_ptr<const Action> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const Action * const &lhs, const std::unique_ptr<const Action> &rhs) const {return operator()(*lhs, *rhs);}

    bool operator()(const std::shared_ptr<const Action> &lhs, const Action &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const std::shared_ptr<const Action> &lhs, const Action * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::shared_ptr<const Action> &lhs, const std::shared_ptr<const Action> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::shared_ptr<const Action> &lhs, const std::unique_ptr<const Action> &rhs) const {return operator()(*lhs, *rhs);}

    bool operator()(const std::unique_ptr<const Action> &lhs, const Action &rhs) const {return operator()(*lhs, rhs);}
    bool operator()(const std::unique_ptr<const Action> &lhs, const Action * const &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::unique_ptr<const Action> &lhs, const std::shared_ptr<const Action> &rhs) const {return operator()(*lhs, *rhs);}
    bool operator()(const std::unique_ptr<const Action> &lhs, const std::unique_ptr<const Action> &rhs) const {return operator()(*lhs, *rhs);}
  };

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
