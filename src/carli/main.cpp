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
  Blocks_World::Environment env;

  std::cout << env;
  do {
    const Blocks_World::Environment::action_type * actions = env.get_candidates();

    int counter = 0;
    std::for_each(actions->candidates.begin(), actions->candidates.end(), [&counter](const Blocks_World::Environment::action_type &) {
      ++counter;
    });

    counter = random.rand_lt(counter) + 1;
    std::for_each(actions->candidates.begin(), actions->candidates.end(), [&counter,&env](const Blocks_World::Environment::action_type &action) {
      if(!--counter)
        env.transition(action);
    });

    std::cout << env;
  } while(env.get_metastate() == Blocks_World::Environment::NON_TERMINAL);

  std::cout << "SUCCESS in " << env.get_step_count() << " moves, yielding " << env.get_total_reward() << " total reward." << std::endl;

  return 0;
}
