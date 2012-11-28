// #define DEBUG_OUTPUT
// #define DEBUG_OUTPUT_VALUE_FUNCTION
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
    : credit_assignment(Blocks_World::Agent::EVEN),
    credit_assignment_epsilon(0.5),
    discount_rate(1.0),
    environment(BLOCKS_WORLD),
    epsilon(0.1),
    learning_rate(1.0),
    number_of_steps(50000),
    output(SIMPLE),
    on_policy(true),
    pseudoepisode_threshold(20),
    seed(uint32_t(time(0))),
    split_min(0),
    split_max(size_t(-1)),
    split_update_count(0),
    split_pseudoepisodes(0),
    split_cabe(0.84155),
    split_mabe(0.84155),
    contribute_update_count(0)
  {
  }

  Blocks_World::Agent::Credit_Assignment credit_assignment;
  double credit_assignment_epsilon;
  double discount_rate;
  enum {BLOCKS_WORLD, PUDDLE_WORLD} environment;
  double epsilon;
  double learning_rate;
  size_t number_of_steps;
  enum {SIMPLE, EXPERIMENTAL} output;
  bool on_policy;
  size_t pseudoepisode_threshold;
  uint32_t seed;
  size_t split_min;
  size_t split_max;
  size_t split_update_count;
  size_t split_pseudoepisodes;
  double split_cabe;
  double split_mabe;
  size_t contribute_update_count;
} g_args;

Options generate_options() {
  Options options("carli");

  options.add('h', "help", Options::Option([&options](const std::vector<const char *> &) {
    options.print_help(std::cout);
    exit(0);
  }, 0), "");
  options.add(     "contribute-update-count", Options::Option([](const std::vector<const char *> &args) {
    g_args.contribute_update_count = atoi(args.at(0));
  }, 1), "[0,inf)");
  options.add('c', "credit-assignment", Options::Option([](const std::vector<const char *> &args) {
    if(!strcmp(args.at(0), "specific"))
      g_args.credit_assignment = Blocks_World::Agent::SPECIFIC;
    else if(!strcmp(args.at(0), "even"))
      g_args.credit_assignment = Blocks_World::Agent::EVEN;
    else if(!strcmp(args.at(0), "inv-update-count"))
      g_args.credit_assignment = Blocks_World::Agent::INV_UPDATE_COUNT;
    else if(!strcmp(args.at(0), "inv-log-update-count"))
      g_args.credit_assignment = Blocks_World::Agent::INV_LOG_UPDATE_COUNT;
    else if(!strcmp(args.at(0), "inv-depth"))
      g_args.credit_assignment = Blocks_World::Agent::INV_DEPTH;
    else if(!strcmp(args.at(0), "epsilon-even-specific"))
      g_args.credit_assignment = Blocks_World::Agent::EPSILON_EVEN_SPECIFIC;
    else if(!strcmp(args.at(0), "epsilon-even-depth"))
      g_args.credit_assignment = Blocks_World::Agent::EPSILON_EVEN_DEPTH;
    else {
      std::cerr << "Illegal credit assignment selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal credit assignment selection.");
    }
  }, 1), "specific/even/inv-update-count/inv-log-update-count/inv-depth/epsilon-even-specific/epsilon-even-depth");
  options.add('d', "discount-rate", Options::Option([](const std::vector<const char *> &args) {
    g_args.discount_rate = atof(args.at(0));
    if(g_args.discount_rate < 0.0 || g_args.discount_rate > 1.0) {
      std::cerr << "Illegal discount rate selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal discount rate selection.");
    }
  }, 1), "[0,1]");
  options.add(     "credit-assignment-epsilon", Options::Option([](const std::vector<const char *> &args) {
    g_args.credit_assignment_epsilon = atof(args.at(0));
    if(g_args.credit_assignment_epsilon < 0.0 || g_args.credit_assignment_epsilon > 1.0) {
      std::cerr << "Illegal credit-assignment epsilon selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal credit-assignment epsilon selection.");
    }
  }, 1), "[0,1]");
  options.add('g', "epsilon-greedy", Options::Option([](const std::vector<const char *> &args) {
    g_args.epsilon = atof(args.at(0));
    if(g_args.epsilon < 0.0 || g_args.epsilon > 1.0) {
      std::cerr << "Illegal epsilon-greedy selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal epsilon-greedy selection.");
    }
  }, 1), "[0,1]");
  options.add('e', "environment", Options::Option([](const std::vector<const char *> &args) {
    if(!strcmp(args.at(0), "blocks-world"))
      g_args.environment = Arguments::BLOCKS_WORLD;
    else if(!strcmp(args.at(0), "puddle-world"))
      g_args.environment = Arguments::PUDDLE_WORLD;
    else {
      std::cerr << "Illegal environment selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal environment selection.");
    }
  }, 1), "blocks-world/puddle-world");
  options.add('l', "learning-rate", Options::Option([](const std::vector<const char *> &args) {
    g_args.learning_rate = atof(args.at(0));
    if(g_args.learning_rate <= 0.0 || g_args.learning_rate > 1.0) {
      std::cerr << "Illegal learning rate selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal learning rate selection.");
    }
  }, 1), "(0,1]");
  options.add('n', "num-steps", Options::Option([](const std::vector<const char *> &args) {
    g_args.number_of_steps = atoi(args.at(0));
    if(g_args.number_of_steps < 1) {
      std::cerr << "Illegal number of steps selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal number of steps selection.");
    }
  }, 1), "[1,inf)");
  options.add('o', "output", Options::Option([](const std::vector<const char *> &args) {
    if(!strcmp(args.at(0), "simple"))
      g_args.output = Arguments::SIMPLE;
    else if(!strcmp(args.at(0), "experimental"))
      g_args.output = Arguments::EXPERIMENTAL;
    else {
      std::cerr << "Illegal output selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal output selection.");
    }
  }, 1), "simple/experimental");
  options.add('p', "policy", Options::Option([](const std::vector<const char *> &args) {
    if(!strcmp(args.at(0), "on-policy"))
      g_args.on_policy = true;
    else if(!strcmp(args.at(0), "off-policy"))
      g_args.on_policy = false;
    else {
      std::cerr << "Illegal policy selection: " << args.at(0) << std::endl;
      throw std::runtime_error("Illegal policy selection.");
    }
  }, 1), "on-policy/off-policy");
  options.add('s', "seed", Options::Option([](const std::vector<const char *> &args) {
    g_args.seed = atoi(args.at(0));
  }, 1), "(-inf,inf)");
  options.add(     "split-cabe", Options::Option([](const std::vector<const char *> &args) {
    g_args.split_cabe = atof(args.at(0));
  }, 1), "[0,inf)");
  options.add(     "split-mabe", Options::Option([](const std::vector<const char *> &args) {
    g_args.split_mabe = atof(args.at(0));
  }, 1), "[0,inf)");
  options.add(     "split-max", Options::Option([](const std::vector<const char *> &args) {
    g_args.split_max = atoi(args.at(0));
  }, 1), "[0,inf)");
  options.add(     "split-min", Options::Option([](const std::vector<const char *> &args) {
    g_args.split_min = atoi(args.at(0));
  }, 1), "[0,inf)");
  options.add(     "split-pseudoepisodes", Options::Option([](const std::vector<const char *> &args) {
    g_args.split_pseudoepisodes = atoi(args.at(0));
  }, 1), "[0,inf)");
  options.add(     "split-update-count", Options::Option([](const std::vector<const char *> &args) {
    g_args.split_update_count = atoi(args.at(0));
  }, 1), "[0,inf)");
  options.add('t', "pseudoepisode-threshold", Options::Option([](const std::vector<const char *> &args) {
    g_args.pseudoepisode_threshold = atoi(args.at(0));
  }, 1), "[0,inf)");

  return options;
}

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

  Options options = generate_options();
  options.get(argc, argv);
  if(options.optind < argc) {
    std::cerr << "Unknown trailing arguments:";
    while(options.optind < argc)
      std::cerr << ' ' << argv[options.optind++];
    std::cerr << std::endl;
    options.print_help(std::cerr);
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

  agent->set_contribute_update_count(g_args.contribute_update_count);
  agent->set_credit_assignment(typename AGENT::Credit_Assignment(g_args.credit_assignment));
  agent->set_credit_assignment_epsilon(g_args.credit_assignment_epsilon);
  agent->set_discount_rate(g_args.discount_rate);
  agent->set_epsilon(g_args.epsilon);
  agent->set_learning_rate(g_args.learning_rate);
  agent->set_on_policy(g_args.on_policy);
  agent->set_pseudoepisode_threshold(g_args.pseudoepisode_threshold);
  agent->set_split_min(g_args.split_min);
  agent->set_split_max(g_args.split_max);
  agent->set_split_update_count(g_args.split_update_count);
  agent->set_split_pseudoepisodes(g_args.split_pseudoepisodes);
  agent->set_split_cabe(g_args.split_cabe);
  agent->set_split_mabe(g_args.split_mabe);

  size_t total_steps = 0;
  size_t successes = 0;
  size_t failures = 0;
  while(total_steps < g_args.number_of_steps) {
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
    } while(agent->get_metastate() == NON_TERMINAL && agent->get_step_count() < 5000 && total_steps < g_args.number_of_steps);

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

    if(g_args.environment == Arguments::PUDDLE_WORLD) {
      auto pwa = std::dynamic_pointer_cast<Puddle_World::Agent>(agent);
      pwa->print_policy(std::cout, 32);
    }
  }
  else if(g_args.output == Arguments::EXPERIMENTAL) {
    if(g_args.environment == Arguments::PUDDLE_WORLD) {
      auto pwa = std::dynamic_pointer_cast<Puddle_World::Agent>(agent);
      pwa->print_policy(std::cerr, 32);
      pwa->print_value_function_grid(std::cerr);
      pwa->print_update_count_grid(std::cerr);
    }
  }
}
