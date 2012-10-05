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

  auto env = std::make_shared<Blocks_World::Environment>();
  auto agent = std::make_shared<Blocks_World::Agent>(env);

  std::cout << *env << *agent;
  do {
    agent->act();

    std::cout << *env << *agent;
  } while(agent->get_metastate() == Blocks_World::Agent::NON_TERMINAL);

  std::cout << "SUCCESS in " << agent->get_step_count() << " moves, yielding " << agent->get_total_reward() << " total reward." << std::endl;

  return 0;
}
