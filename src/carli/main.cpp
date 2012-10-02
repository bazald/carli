#include "memory_pool.h"
#include "linked_list.h"
#include "random.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <list>

#include "blocks_world.h"
#include "value.h"

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

  Zeni::Random random;
  Mean mean;

  for(int i = 0; i != 1000; ++i) {
    Q_Value::List * head(&(new Q_Value)->list);
    for(int i = 1; i != 1000; ++i) {
      Q_Value * const new_head = new Q_Value(84.0 * random.frand_lte());
      new_head->list.insert_before(head);
      head = &new_head->list;
    }

    std::for_each(head->begin(), head->end(), [&mean](Q_Value &ptr) {
//       std::cout << *ptr << ' ';
      mean.contribute(ptr);
    });

//     std::cout << "= " << mean;

    std::for_each(head->begin(), head->end(), [&mean](Q_Value &ptr) {
      mean.uncontribute(ptr);
    });

//     std::cout << " and then " << mean << std::endl;

    head->destroy();
  }

  Blocks_World::Environment env;

  std::cout << env;
  env.transition(Blocks_World::Move(3, 2));
  std::cout << env;
  env.transition(Blocks_World::Move(3, 0));
  std::cout << env;
  env.transition(Blocks_World::Move(2, 3));
  std::cout << env;
//   env.transition(Blocks_World::Move(3, 0));
//   std::cout << env;

  return 0;
}
