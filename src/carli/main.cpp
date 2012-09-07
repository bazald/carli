#include "memory_pool.h"
#include "linked_list.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <list>

#ifdef WIN32
#include <Windows.h>
#endif

class Value;
class Value : public Zeni::Pool_Allocator<Value> {
public:
  typedef Zeni::Linked_List<Value> List;
  typedef List::iterator iterator;

  Value()
   : list(this)
  {
  }

  Value(const Value &)
   : list(this)
  {
  }

  Value & operator=(const Value &) {
    return *this;
  }

  virtual ~Value() {}

  List list;

private:
  
};

int main(int argc, char **argv) {
  Zeni::register_new_handler();

  for(int i = 0; i != 0x10000; ++i) {
    Value::List * const head(&(new Value)->list);
    Value::List * tail(head);
    for(int i = 1; i != 100; ++i, tail = tail->next())
      (new Value)->list.insert_after(tail);

    for(Value::iterator it = head->begin(), iend = head->end(); it != iend; )
      delete it++.get();
  }

  return 0;
}
