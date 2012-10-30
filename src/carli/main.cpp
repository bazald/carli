// #define DEBUG_OUTPUT
// #define NULL_Q_VALUES
// #define TO_FILE

#include "blocks_world.h"
#include "puddle_world.h"
#include "getopt.h"

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>

#ifdef TO_FILE
#include <fstream>
#endif

typedef std::string test_key;
typedef Q_Value test_value;
typedef Zeni::Trie<test_key, test_value> test_trie;

template <typename ENVIRONMENT, typename AGENT>
void run_agent();

struct Arguments {
  Arguments()
    : discount_rate(1.0),
    environment(BLOCKS_WORLD),
    epsilon(0.1),
    learning_rate(1.0),
    output(SIMPLE),
    on_policy(true),
    seed(uint32_t(time(0)))
  {
  }

  double discount_rate;
  enum {BLOCKS_WORLD, PUDDLE_WORLD} environment;
  double epsilon;
  double learning_rate;
  enum {SIMPLE, EXPERIMENTAL} output;
  bool on_policy;
  uint32_t seed;
} g_args;

int main2(int argc, char **argv) {
#ifdef TO_FILE
  auto cout_bak = std::cout.rdbuf();
  auto cerr_bak = std::cerr.rdbuf();
  std::ofstream cout2file("dump.txt");
  std::cout.rdbuf(cout2file.rdbuf());
  std::cerr.rdbuf(cout2file.rdbuf());
#endif

  Zeni::register_new_handler();

//   std::cerr << "sizeof(env) = " << sizeof(*env) << std::endl;
//   std::cerr << "sizeof(agent) = " << sizeof(*agent) << std::endl;
//   std::cerr << "sizeof(Blocks_World::Feature) = " << sizeof(Blocks_World::Feature) << std::endl;
//   std::cerr << "sizeof(Blocks_World::In_Place) = " << sizeof(Blocks_World::In_Place) << std::endl;
//   std::cerr << "sizeof(Blocks_World::On_Top) = " << sizeof(Blocks_World::On_Top) << std::endl;
//   std::cerr << "sizeof(Blocks_World::Move) = " << sizeof(Blocks_World::Move) << std::endl;
//   std::cerr << "sizeof(Puddle_World::Feature) = " << sizeof(Puddle_World::Feature) << std::endl;
//   std::cerr << "sizeof(Puddle_World::Move) = " << sizeof(Puddle_World::Move) << std::endl;
//   std::cerr << "sizeof(Q_Value) = " << sizeof(Q_Value) << std::endl;
//   std::cerr << "sizeof(Trie) = " << sizeof(Blocks_World::Agent::feature_trie_type) << std::endl;

  int c;
  const char * const options = "d:e:g:hl:o:p:s:";
  while((c = getopt(argc, argv, options)) != -1) {
    switch (c) {
    case 'd':
      g_args.discount_rate = atof(optarg);
      if(g_args.discount_rate < 0.0 || g_args.discount_rate > 1.0) {
        std::cerr << "Illegal discount rate selection: " << optarg << std::endl;
        throw std::runtime_error("Illegal discount rate selection.");
      }
      break;

    case 'e':
      if(!strcmp(optarg, "blocks-world"))
        g_args.environment = Arguments::BLOCKS_WORLD;
      else if(!strcmp(optarg, "puddle-world"))
        g_args.environment = Arguments::PUDDLE_WORLD;
      else {
        std::cerr << "Illegal environment selection: " << optarg << std::endl;
        throw std::runtime_error("Illegal environment selection.");
      }
      break;

    case 'g':
      g_args.epsilon = atof(optarg);
      if(g_args.epsilon < 0.0 || g_args.epsilon > 1.0) {
        std::cerr << "Illegal epsilon-greedy selection: " << optarg << std::endl;
        throw std::runtime_error("Illegal epsilon-greedy selection.");
      }
      break;

    case 'l':
      g_args.learning_rate = atof(optarg);
      if(g_args.learning_rate <= 0.0 || g_args.learning_rate > 1.0) {
        std::cerr << "Illegal learning rate selection: " << optarg << std::endl;
        throw std::runtime_error("Illegal learning rate selection.");
      }
      break;

    case 'o':
      if(!strcmp(optarg, "simple"))
        g_args.output = Arguments::SIMPLE;
      else if(!strcmp(optarg, "experimental"))
        g_args.output = Arguments::EXPERIMENTAL;
      else {
        std::cerr << "Illegal output selection: " << optarg << std::endl;
        throw std::runtime_error("Illegal output selection.");
      }
      break;

    case 'p':
      if(!strcmp(optarg, "on-policy"))
        g_args.on_policy = true;
      else if(!strcmp(optarg, "off-policy"))
        g_args.on_policy = false;
      else {
        std::cerr << "Illegal policy selection: " << optarg << std::endl;
        throw std::runtime_error("Illegal policy selection.");
      }
      break;

    case 's':
      g_args.seed = atoi(optarg);
      break;

    case 'h':
    case '?':
      std::cerr << "Usage: carli [" << options << ']' << std::endl;
      std::cerr << "  -d Set the discount rate:" << std::endl
                << "       [0,1]" << std::endl;
      std::cerr << "  -e Select an environment:" << std::endl
                << "       blocks-world" << std::endl
                << "       puddle-world" << std::endl;
      std::cerr << "  -g Set the epsilon-greedy policy:" << std::endl
                << "       [0,1]" << std::endl;
      std::cerr << "  -h Print this help." << std::endl;
      std::cerr << "  -l Set the learning rate:" << std::endl
                << "       (0,1]" << std::endl;
      std::cerr << "  -p Choose:" << std::endl
                << "       on-policy" << std::endl
                << "       off-policy" << std::endl;
      std::cerr << "  -s Set the random seed:" << std::endl
                << "       (-inf,inf)" << std::endl;
      return c == '?';

    default:
      std::cerr << "?? getopt returned character code 0" << c << " ??\n";
      break;
    }
  }
  if(optind < argc) {
    std::cerr << "Unknown trailing arguments:";
    while(optind < argc)
      std::cerr << ' ' << argv[optind++];
    std::cerr << std::endl;
    throw std::runtime_error("Unknown trailing arguments.");
  }

  switch(g_args.environment) {
    case Arguments::BLOCKS_WORLD:
      run_agent<Blocks_World::Environment, Blocks_World::Agent>();
      break;

    case Arguments::PUDDLE_WORLD:
      run_agent<Puddle_World::Environment, Puddle_World::Agent>();
      break;

    default:
      throw std::runtime_error("Internal error: g_args.environment");
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

#ifdef TO_FILE
  std::cout.rdbuf(cout_bak);
  std::cerr.rdbuf(cerr_bak);
#endif

  return 0;
}

int main(int argc, char **argv) {
  try {
    return main2(argc, argv);
  }
  catch(std::exception &ex) {
    std::cerr << "Exiting with exception: " << ex.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Exiting with unknown exception." << std::endl;
  }

  return -1;
}

template <typename ENVIRONMENT, typename AGENT>
void run_agent() {
  Zeni::Random::get().seed(uint32_t(g_args.seed));
  std::cout << "SEED " << g_args.seed << std::endl;

  auto env = std::make_shared<ENVIRONMENT>();
  auto agent = std::make_shared<AGENT>(env);

  agent->set_discount_rate(g_args.discount_rate);
  agent->set_epsilon(g_args.epsilon);
  agent->set_learning_rate(g_args.learning_rate);
  agent->set_on_policy(g_args.on_policy);

  size_t total_steps = 0;
  size_t successes = 0;
  size_t failures = 0;
  while(total_steps < 50000) {
    env->init();
    agent->init();

#ifdef DEBUG_OUTPUT
    std::cerr << *env << *agent;
#endif
    do {
      const double reward = agent->act();
      ++total_steps;

      if(g_args.output == Arguments::EXPERIMENTAL)
        std::cout << total_steps << ' ' << agent->get_episode_number() << ' ' << agent->get_step_count() << ' ' << reward << std::endl;

#ifdef DEBUG_OUTPUT
      std::cerr << *env << *agent;
#endif
    } while(agent->get_metastate() == NON_TERMINAL && agent->get_step_count() < 5000);

    if(agent->get_metastate() == SUCCESS) {
      if(g_args.output == Arguments::SIMPLE)
        std::cout << "SUCCESS";
      ++successes;
    }
    else {
      if(g_args.output == Arguments::SIMPLE)
        std::cout << "FAILURE";
      ++failures;
    }

    if(g_args.output == Arguments::SIMPLE)
      std::cout << " in " << agent->get_step_count() << " moves, yielding " << agent->get_total_reward() << " total reward." << std::endl;
  }

  if(g_args.output == Arguments::SIMPLE) {
    std::cout << successes << " SUCCESSes" << std::endl;
    std::cout << failures << " FAILUREs" << std::endl;
    std::cout << agent->get_value_function_size() << " Q-values" << std::endl;
  }
}
