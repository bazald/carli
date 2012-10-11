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
#include "value.h"

class Q_Value;
class Q_Value : public Value, public Zeni::Pool_Allocator<Q_Value>, public Zeni::Cloneable<Q_Value> {
  Q_Value(const Q_Value &);
  Q_Value & operator=(const Q_Value &);

public:
  typedef Zeni::Map<std::string, Q_Value> List;
  typedef List::iterator iterator;

  Q_Value(const double &q_value_ = double(), const std::string &key_ = std::string())
   : Value(q_value_),
   list(this, key_)
  {
  }

  Q_Value * clone() const {
    return new Q_Value(double(*this));
  }

  List list;
};

typedef std::string test_key;
typedef int test_value;
typedef Zeni::Trie<test_key, test_value> test_trie;

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


  Q_Value::List * q_values = nullptr;
  (new Q_Value(42, "42"))->list.insert(q_values);
  (new Q_Value(42, "42"))->list.insert(q_values);
  (new Q_Value(43, "haha"))->list.insert(q_values);
  (new Q_Value(40, "hello"))->list.insert(q_values);
  (new Q_Value(41, "world"))->list.insert(q_values);
  q_values->set_key("1337");
  std::for_each(q_values->begin(), q_values->end(), [](const Q_Value &q) {std::cout << q.list.get_key() << ':' << q << std::endl;});
  q_values->destroy();

  test_trie * trie = nullptr;
  test_trie * key = nullptr;
  (new test_trie("hello"))->list_insert_before(key);
  (new test_trie("world"))->list_insert_before(key);
  **key->insert(trie) = 10;
  key = nullptr;
  (new test_trie("hello"))->list_insert_before(key);
  (new test_trie("you"))->list_insert_before(key);
  **key->insert(trie) = 20;
  key = nullptr;
  (new test_trie("hello"))->list_insert_before(key);
  (new test_trie("world"))->list_insert_before(key);
  std::cout << **key->insert(trie) << std::endl;
  key = nullptr;
  (new test_trie("hello"))->list_insert_before(key);
  (new test_trie("you"))->list_insert_before(key);
  std::cout << **key->insert(trie) << std::endl;
  key = nullptr;
  trie->destroy();

//   auto map = new Zeni::Map<Q_Value, int>;
//   (*trie)[*q_values] = 42;
//   map->destroy();

//   auto trie = new Zeni::Trie<Q_Value, int>;
//   (*trie)[q_values] = 42;
//   trie->destroy();

  return 0;
}
