#include "memory_pool.h"
#include "linked_list.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <list>

#include "value.h"

template <typename ITERATOR, typename OPERATION>
OPERATION list_for_each(ITERATOR it, const ITERATOR &iend, OPERATION op) {
  while(it != iend) {
    const ITERATOR current = it++;
    op(current);
  }

  return op;
}

template <typename TYPE>
void list_destroy(typename Zeni::Linked_List<TYPE>::iterator const &it) {
  delete it.get();
}

class Q_Value;
class Q_Value : public Value, public Zeni::Pool_Allocator<Q_Value> {
  Q_Value(const Q_Value &);
  Q_Value & operator=(const Q_Value &);

public:
  typedef Zeni::Linked_List<Q_Value> List;
  typedef List::iterator iterator;

  Q_Value(const double &q_value_ = double())
   : Value(q_value_),
   list(this)
  {
  }

  List list;
};

int main(int argc, char **argv) {
  Zeni::register_new_handler();

  srand(time(0));

//   for(int i = 0; i != 0x10000; ++i) {
//     Q_Value::List * head(&(new Q_Value)->list);
//     for(int i = 1; i != 100; ++i) {
//       Q_Value * const new_head = new Q_Value;
//       new_head->list.insert_before(head);
//       head = &new_head->list;
//     }
// 
//     list_for_each(head->begin(), head->end(), list_destroy<Q_Value>);
//   }

  for(int i = 0; i != 1000; ++i) {
    Q_Value::List * head(&(new Q_Value)->list);
    for(int i = 1; i != 1000; ++i) {
      Q_Value * const new_head = new Q_Value(84.0 * rand() / RAND_MAX);
      new_head->list.insert_before(head);
      head = &new_head->list;
    }

    Mean mean;
    list_for_each(head->begin(), head->end(), [&mean](Zeni::Linked_List<Q_Value>::iterator const &it) {
//       std::cout << *it << ' ';
      mean.contribute(*it);
    });

//     std::cout << "= " << mean;

    list_for_each(head->begin(), head->end(), [&mean](Zeni::Linked_List<Q_Value>::iterator const &it) {
      mean.uncontribute(*it);
    });

//     std::cout << " and then " << mean << std::endl;

    list_for_each(head->begin(), head->end(), list_destroy<Q_Value>);
  }

  return 0;
}
