#include "memory_pool.h"
#include "linked_list.h"
#include "set.h"
#include "map.h"
#include "trie.h"
#include "random.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <list>

#include "blocks_world.h"
#include "q_value.h"

typedef std::string test_key;
typedef Q_Value test_value;
typedef Zeni::Trie<test_key, test_value> test_trie;

int main(int argc, char **argv) {
  Zeni::register_new_handler();

  srand(time(0));

  auto env = std::make_shared<Blocks_World::Environment>();
  auto agent = std::make_shared<Blocks_World::Agent>(env);

  for(int i = 0; i != 100; ++i) {
    env->init();
    agent->init();

    std::cerr << *env /*<< *agent*/;
    do {
      agent->act();

      std::cerr << *env /*<< *agent*/;
    } while(agent->get_metastate() == Blocks_World::Agent::NON_TERMINAL);

    std::cout << "SUCCESS in " << agent->get_step_count() << " moves, yielding " << agent->get_total_reward() << " total reward." << std::endl;
  }

//   test_trie * trie = nullptr;
//   test_trie * key = nullptr;
//   (new test_trie("world"))->list_insert_before(key);
//   (new test_trie("hello"))->list_insert_before(key);
//   **key->insert(trie, Q_Value::list_offset()) = 1.0;
//   key = nullptr;
//   (new test_trie("world"))->list_insert_before(key);
//   (new test_trie("hi"))->list_insert_before(key);
//   **key->insert(trie, Q_Value::list_offset()) = 2.0;
//   key = nullptr;
//   (new test_trie("hi"))->list_insert_before(key);
//   **key->insert(trie, Q_Value::list_offset()) = 3.0;
//   key = nullptr;
//   (new test_trie("world"))->list_insert_before(key);
//   (new test_trie("hi"))->list_insert_before(key);
//   key = key->insert(trie, Q_Value::list_offset());
//   for_each((*key)->list.begin(), (*key)->list.end(), [](const Q_Value &q) {
//     std::cout << q << std::endl;
//   });
//   trie->destroy();

  return 0;
}
