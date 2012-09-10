#include "memory_pool.h"
#include "linked_list.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <list>

template <typename ITERATOR>
void destroy_list(ITERATOR it, ITERATOR iend) {
  while(it != iend)
    delete it++.get();
}

class Value;
class Value : public Zeni::Pool_Allocator<Value> {
public:
  typedef Zeni::Linked_List<Value> List;
  typedef List::iterator iterator;

  Value()
   : list(this),
   q_value(0.0)
  {
  }

  Value(const Value &rhs)
   : list(this),
   q_value(rhs.q_value)
  {
  }

  Value & operator=(const Value &rhs) {
    Value temp(rhs);

    std::swap(temp.q_value, q_value);

    return *this;
  }

  virtual ~Value() {}

  List list;

  double q_value;
};

int main(int argc, char **argv) {
  Zeni::register_new_handler();

  for(int i = 0; i != 0x10000; ++i) {
    Value::List * head(&(new Value)->list);
    for(int i = 1; i != 100; ++i) {
      Value * const new_head = new Value;
      new_head->list.insert_before(head);
      head = &new_head->list;
    }

    destroy_list(head->begin(), head->end());
  }

  return 0;
}
