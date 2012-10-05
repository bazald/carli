#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <memory>
#include <utility>

#include "linked_list.h"
#include "memory_pool.h"

template <typename TYPE>
struct Action : public Zeni::Pool_Allocator<TYPE> {
  typedef typename Zeni::Linked_List<Action> List;
  typedef typename List::iterator iterator;

  Action()
    : candidates(this)
  {
  }

  virtual ~Action() {}

  void print(std::ostream &os) const {
    print_impl(os);
  }

  virtual void print_impl(std::ostream &os) const = 0;

  List candidates;
};

template <typename TYPE>
std::ostream & operator << (std::ostream &os, const Action<TYPE> &action) {
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

