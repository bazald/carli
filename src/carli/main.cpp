#define DEBUG_OUTPUT

#include "blocks_world.h"
#include "puddle_world.h"

#include <memory>
#include <cstring>
#include <ctime>
#include <iostream>
#include <list>

typedef std::string test_key;
typedef Q_Value test_value;
typedef Zeni::Trie<test_key, test_value> test_trie;

int main(int argc, char **argv) {
  Zeni::register_new_handler();

  srand(uint32_t(time(0)));

//   auto env = std::make_shared<Blocks_World::Environment>();
//   auto agent = std::make_shared<Blocks_World::Agent>(env);

  auto env = std::make_shared<Puddle_World::Environment>();
  auto agent = std::make_shared<Puddle_World::Agent>(env);

  for(int i = 0; i != 100; ++i) {
    env->init();
    agent->init();

#ifdef DEBUG_OUTPUT
    std::cerr << *env << *agent;
#endif
    do {
      agent->act();

#ifdef DEBUG_OUTPUT
      std::cerr << *env << *agent;
#endif
    } while(agent->get_metastate() == Puddle_World::Agent::NON_TERMINAL);

    std::cout << "SUCCESS in " << agent->get_step_count() << " moves, yielding " << agent->get_total_reward() << " total reward." << std::endl;
  }

//   test_trie * trie = new test_trie;
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
//   trie->destroy(trie);

//   Zeni::Random m_random;
//   Mean mean;
//   std::list<Value> values;
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   values.push_back(m_random.rand_lt(42));
//   for_each(values.begin(), values.end(), [&mean](Value &value) {
//     mean.contribute(value);
//     std::cout << "Adding " << value << " yields " << mean.get_mean() << ':' << mean.get_stddev() << std::endl;
//   });
//   for_each(values.rbegin(), values.rend(), [&mean](Value &value) {
//     mean.uncontribute(value);
//     std::cout << "Removing " << value << " yields " << mean.get_mean() << ':' << mean.get_stddev() << std::endl;
//   });

  return 0;
}
