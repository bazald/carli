#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <memory>
#include <utility>

#include "clone.h"
#include "linked_list.h"
#include "memory_pool.h"

template <typename DERIVED, typename DERIVED2 = DERIVED>
class Action : public Zeni::Pool_Allocator<DERIVED2>, public Zeni::Cloneable<DERIVED> {
  Action operator=(const Action &);

public:
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;

  struct Compare {
    bool operator()(const Action &lhs, const Action &rhs) const {
      return lhs < rhs;
    }

    bool operator()(const Action * const &lhs, const Action * const &rhs) const {
      return *lhs < *rhs;
    }

    bool operator()(const std::shared_ptr<const Action> &lhs, const std::shared_ptr<const Action> &rhs) const {
      return *lhs < *rhs;
    }

    bool operator()(const std::unique_ptr<const Action> &lhs, const std::unique_ptr<const Action> &rhs) const {
      return *lhs < *rhs;
    }
  };

  Action()
    : candidates(this)
  {
  }

  Action(const Action &)
    : candidates(this)
  {
  }

  virtual ~Action() {}

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
    return dynamic_cast<const DERIVED *>(this)->compare(dynamic_cast<const DERIVED &>(rhs));
  }

  virtual void print_impl(std::ostream &os) const = 0;

  List candidates;
};

template <typename DERIVED, typename DERIVED2>
std::ostream & operator << (std::ostream &os, const Action<DERIVED, DERIVED2> &action) {
  action.print(os);
  return os;
}

template <typename ACTION>
class Environment : public std::enable_shared_from_this<Environment<ACTION> > {
  Environment(const Environment &);
  Environment operator=(const Environment &);

public:
  typedef ACTION action_type;
  typedef double reward_type;

  Environment() {}

  void init() {
    init_impl();
  }

  reward_type transition(const action_type &action) {
    return transition_impl(action);
  }

  void print(std::ostream &os) const {
    print_impl(os);
  }

private:
  virtual void init_impl() = 0;
  virtual reward_type transition_impl(const action_type &action) = 0;
  virtual void print_impl(std::ostream &os) const = 0;
};

template <typename ACTION>
std::ostream & operator << (std::ostream &os, const Environment<ACTION> &env) {
  env.print(os);
  return os;
}

#endif

