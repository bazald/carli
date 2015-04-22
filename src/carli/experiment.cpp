#include "experiment.h"

#include "experimental_output.h"
#include "git.h"
#include "utility/getopt.h"
#include "utility/memory_pool.h"

#include <cstring>
#include <ctime>
#include <limits>
#include <random>

namespace Carli {

  using std::cerr;
  using std::cout;
  using std::dynamic_pointer_cast;
  using std::endl;
  using std::function;
  using std::make_shared;
  using std::numeric_limits;
  using std::runtime_error;
  using std::set;
  using std::shared_ptr;
  using std::string;

  // static void run_agent(const function<shared_ptr<Environment> ()> &make_env, const function<shared_ptr<Agent> (const shared_ptr<Environment> &)> &make_agent);

  Experiment::Experiment()
   : cerr_bak(cerr.rdbuf()),
   cout_bak(cout.rdbuf())
  {
    cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
    cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    cerr.precision(9);
    cout.precision(9);

    Zeni::register_new_handler();

    string version_str;
    if(git_modified())
      version_str = string("Built from revision ") + git_revision_string() + " (" + git_modified_string() + " modified files) on " __DATE__ " at " __TIME__ ".";
    else
      version_str = string("Built from revision ") + git_revision_string() + " (clean) on " __DATE__ " at " __TIME__ ".";

    Options &options = Options::get_global();

    options.add_line("\n  " + version_str);
    options.add_line("\n  Print Information:");
    options.add('h', make_shared<Option_Function>("help", 0, [&options](const Option::Arguments &){
      options.print_help(cout);
      cout << endl;
      options.print(cout);
      exit(0);
    }), "");
    options.add('v', make_shared<Option_Function>("version", 0, [&version_str](const Option::Arguments &){
      cout << version_str << endl;
      exit(0);
    }), "");
    options.add_line("\n  Experiment Options:");
    options.add(     make_shared<Option_Ranged<int64_t>>("num-episodes", 0, true, numeric_limits<int64_t>::max(), true, 0), "Maximum number of episodes; 0 disables.");
    options.add('n', make_shared<Option_Ranged<int64_t>>("num-steps", 0, true, numeric_limits<int64_t>::max(), true, 50000), "Maximum number of steps; 0 disables.");
    options.add('o', make_shared<Option_Itemized>("output", set<string>({"null", "simple", "experiment"}), "simple"), "What kind of output should be generated.");
    options.add('p', make_shared<Option_Ranged<int64_t>>("print-every", 1, true, numeric_limits<int64_t>::max(), true, 100), "How many steps per line of output.");
    options.add('r', make_shared<Option_String>("rules", "default"), "Which .carli rules should the agent load?");
    options.add(     make_shared<Option_String>("rules-out", ""), "Where should final .carli rules be saved?");
    options.add(     make_shared<Option_Ranged<int64_t>>("scenario", 0, true, numeric_limits<int64_t>::max(), true, 0), "Which experimental scenario should be run, environment specific.");
    options.add('s', make_shared<Option_Ranged<int64_t>>("seed", numeric_limits<int64_t>::min(), true, numeric_limits<int64_t>::max(), true, std::random_device()()), "Random seed.");
    options.add(     make_shared<Option_Function>("stderr", 1, [this,&options](const Option::Arguments &args){
      this->cerr2file.open(args.at(0));
      cerr.rdbuf(this->cerr2file.rdbuf());
    }), "<file> Redirect stderr to <file>");
    options.add(     make_shared<Option_Function>("stdout", 1, [this,&options](const Option::Arguments &args){
      this->cout2file.open(args.at(0));
      cout.rdbuf(this->cout2file.rdbuf());
    }), "<file> Redirect stdout to <file>");
    options.add_line("\n  Environment Options:");
    options.add(     make_shared<Option_Ranged<bool>>("ignore-x", false, true, true, true, false), "Simplify cart-pole from 4D to 2D, eliminating x and x-dot.");
    options.add(     make_shared<Option_Ranged<int64_t>>("num-blocks", 2, true, 26, true, 3), "Number of blocks to generate in Blocks World 2.");
    options.add(     make_shared<Option_Ranged<bool>>("random-start", false, true, true, true, false), "Should starting positions be randomized in mountain-car and puddle-world.");
    options.add(     make_shared<Option_Ranged<bool>>("reward-negative", false, true, true, true, true), "Use negative rewards per step in mountain-car rather than positive terminal rewards.");
    options.add(     make_shared<Option_Ranged<bool>>("set-goal", false, true, true, true, false), "Convert Cart Pole from an equilibrium task to a \"minimum time maneuver to a small goal region\" task.");
    options.add_line("\n  Standard TD Options:");
    options.add('d', make_shared<Option_Ranged<double>>("discount-rate", 0.0, true, 1.0, true, 1.0), "");
    options.add(     make_shared<Option_Ranged<double>>("eligibility-trace-decay-rate", 0.0, true, 1.0, true, 0.0), "Rate at which weights should lose credit.");
    options.add(     make_shared<Option_Ranged<double>>("eligibility-trace-decay-threshold", 0.0, true, 1.0, true, 0.0001), "Weights with credit below this threshold cease to receive updates.");
    options.add('g', make_shared<Option_Ranged<double>>("epsilon-greedy", 0.0, true, 1.0, true, 0.1), "Simple exploration rate.");
    options.add('l', make_shared<Option_Ranged<double>>("learning-rate", 0.0, true, 1.0, true, 1.0), "");
    options.add(     make_shared<Option_Ranged<double>>("secondary-learning-rate", 0.0, true, 1.0, true, 1.0), "");
    options.add(     make_shared<Option_Itemized>("policy", set<string>({"on-policy", "off-policy"}), "on-policy"), "Learn about greedy or optimal policy.");
    options.add_line("\n  Credit Assignment:");
    options.add('c', make_shared<Option_Itemized>("credit-assignment", set<string>({"all", "random", "specific", "even", "inv-update-count", "inv-log-update-count", "inv-root-update-count", "inv-depth", "epsilon-even-specific", "epsilon-even-depth"}), "even"), "How to split credit between weights.");
    options.add(     make_shared<Option_Ranged<double>>("credit-assignment-epsilon", 0.0, true, 1.0, true, 0.5), "How to split credit assignment strategies for epsilon-*.");
    options.add(     make_shared<Option_Ranged<double>>("credit-assignment-log-base", 0.0, false, numeric_limits<double>::infinity(), false, 2.71828182846), "Which log to perform for inv-log-update-count.");
    options.add(     make_shared<Option_Ranged<double>>("credit-assignment-root", 1.0, false, numeric_limits<double>::infinity(), false, 2.0), "Which root to perform for inv-root-update-count.");
    options.add(     make_shared<Option_Ranged<bool>>("credit-assignment-normalize", false, true, true, true, true), "Ensure credit assignment sums to *at most* 1.");
    options.add_line("\n  Refinement Options:");
    options.add(     make_shared<Option_Ranged<int64_t>>("pseudoepisode-threshold", 0, true, numeric_limits<int64_t>::max(), true, 20), "How any steps must separate updates for it to count as a pseudoepisode.");
    options.add(     make_shared<Option_Ranged<double>>("split-catde", 0.0, true, numeric_limits<double>::infinity(), false, 0.84155), "How many standard deviations above the mean to refine.");
    options.add(     make_shared<Option_Ranged<double>>("split-catde-qmult", 0.0, true, numeric_limits<double>::infinity(), false, 0.0), "Increase split-catde by this factor of the number of weights.");
    options.add(     make_shared<Option_Ranged<int64_t>>("split-max", 0, true, numeric_limits<int64_t>::max(), true, numeric_limits<int64_t>::max()), "Refinement is strictly prohibited from this depth.");
    options.add(     make_shared<Option_Ranged<int64_t>>("split-min", 0, true, numeric_limits<int64_t>::max(), true, 0), "Refinement is assured through this depth.");
    options.add(     make_shared<Option_Ranged<int64_t>>("split-pseudoepisodes", 0, true, numeric_limits<int64_t>::max(), true, 0), "Require 1 more pseudoepisode than this to allow refinement.");
    options.add(     make_shared<Option_Itemized>("split-test", set<string>({"catde", "policy", "value"}), "catde"), "'catde' splits based on TD error. 'policy' splits when there's a change to the greedy action. 'value' splits on maximal value difference between fringe nodes.");
    options.add(     make_shared<Option_Ranged<int64_t>>("split-update-count", 0, true, numeric_limits<int64_t>::max(), true, 0), "Require 1 more update than this to allow refinement.");
    options.add_line("\n  Rete Representation Saving and Loading:");
    options.add(     make_shared<Option_Itemized>("value-function-map-mode", set<string>({"null", "in", "out"}), "null"), "'in' reads in a file written with 'out', resulting in a fixed hierarchical representation.");
    options.add(     make_shared<Option_String>("value-function-map-filename", "value-function-map.txt"), "");
    options.add_line("\n  CMAC Options:");
    options.add(     make_shared<Option_Ranged<bool>>("cmac", false, true, true, true, false), "Implement a CMAC for the domain instead.");
    options.add(     make_shared<Option_Ranged<int64_t>>("cmac-tilings", 1, true, numeric_limits<int64_t>::max(), true, 10), "The number of tile codings in the CMAC.");
    options.add(     make_shared<Option_Ranged<int64_t>>("cmac-resolution", 1, true, numeric_limits<int64_t>::max(), true, 16), "The number of tile codings in the CMAC.");
    options.add(     make_shared<Option_Ranged<int64_t>>("cmac-offset", 0, true, numeric_limits<int64_t>::max(), true, 0), "The tiling that should be the base tiling.");
    options.add_line("\n  Unusual Options:");
    options.add(     make_shared<Option_Ranged<int64_t>>("contribute-update-count", 0, true, numeric_limits<int64_t>::max(), true, 0), "Require 1 more update than this to count toward means and variances.");
    options.add(     make_shared<Option_String>("dependencies-out", ""), "Where to output feature dependencies depender\\dependee .csv?");
    options.add(     make_shared<Option_Ranged<bool>>("rete-flush-wmes", false, true, true, true, false), "Flush all WMEs from step to step for purposes of performance comparison.");
    options.add(     make_shared<Option_Ranged<bool>>("rete-node-sharing", false, true, true, true, true), "Enable sharing of rete nodes. Disable for purposes of performance comparison.");
    options.add(     make_shared<Option_Ranged<bool>>("dynamic-midpoint", false, true, true, true, false), "Dynamically modify midpoint values for features to better balance refinements.");
    options.add(     make_shared<Option_Ranged<double>>("fringe-learning-scale", 0.0, false, 1.0, true, 0.3), "How quickly should the fringe learn relative to rest of the value function? (1.0 == same)");
    options.add(     make_shared<Option_Ranged<int64_t>>("mean-catde-queue-size", 0, true, numeric_limits<int64_t>::max(), true, 0), "How large of a working set to use for means and variances; 0 disables.");
    options.add(     make_shared<Option_Ranged<bool>>("output-dot", false, true, true, true, false), "Generate .dot files for the rete [pre/post]-[expansion/collapse].");
    options.add(     make_shared<Option_Ranged<int64_t>>("value-function-cap", 0, true, numeric_limits<int64_t>::max(), true, 0), "The maximum number of weights allowed in the value functions; 0 disables.");
  //  options.add(     make_shared<Option_Itemized>("weight-assignment", set<string>({"all", "specific", "even", "inv-update-count", "inv-log-update-count", "inv-root-update-count", "inv-depth", "epsilon-even-specific", "epsilon-even-depth"}), "all"),
  //#ifdef ENABLE_WEIGHT
  //              "How to scale weights before summing them.");
  //#else
  //              "Option disabled at compile-time.");
  //#endif
    options.add_line("\n  For Transfer Experiments:");
    options.add(     make_shared<Option_Ranged<int64_t>>("skip-steps", -1, true, numeric_limits<int64_t>::max(), true, 0), "How many steps to run before counting them and generating output; 0 disables, -1 alters immediately.");
    options.add(     make_shared<Option_Ranged<bool>>("reset-update-counts", false, true, true, true, false), "Reset update counts after skip-steps.");
  }

  Experiment::~Experiment() {
    cout.rdbuf(cout_bak);
    cerr.rdbuf(cerr_bak);
  }

  int64_t Experiment::take_args(int argc, const char * const * argv) {
    Options &options = Options::get_global();

    options.get(argc, argv);

    const auto seed = uint32_t(dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value());
    Zeni::Random::get().seed(seed);
    cout << "SEED " << seed << endl;

    std::ofstream seed_file("seed.txt");
    if(seed_file)
      seed_file << "SEED " << seed << endl;

    return options.optind;
  }

  void Experiment::standard_run(const function<shared_ptr<Environment> ()> &make_env,
                                const function<shared_ptr<Agent> (const shared_ptr<Environment> &)> &make_agent,
                                const function<void (const shared_ptr<Agent> &)> &on_episode_termination) {
    auto env = make_env();
    auto agent = make_agent(env);

    Experimental_Output experimental_output(dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["print-every"]).get_value());

    const auto num_episodes = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["num-episodes"]).get_value();
    const auto num_steps = dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["num-steps"]).get_value();
    const auto output = dynamic_cast<const Option_Itemized &>(Options::get_global()["output"]).get_value();

    int64_t total_steps = -dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["skip-steps"]).get_value();
    if(total_steps > 0) {
      total_steps = 0;
      env->alter();
    }

    size_t successes = 0;
    size_t failures = 0;
    for(int64_t episodes = 0; !num_episodes || episodes < num_episodes; ++episodes) {
      if(num_steps && total_steps > -1 && total_steps >= num_steps)
        break;

      env->init();
#ifdef DEBUG_OUTPUT
      cerr << *env;
#endif

      agent->init();
#ifdef DEBUG_OUTPUT
      cerr << *agent;
#endif

      bool done = false;
      do {
        const double reward = agent->act();
        ++total_steps;

#ifdef DEBUG_OUTPUT
        cerr << *env << *agent;
#endif

        if(!total_steps) {
          env->alter();
          agent->reset_statistics();
          if(dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["reset-update-counts"]).get_value())
            agent->visit_reset_update_count();
        }

        done = agent->get_metastate() != Metastate::NON_TERMINAL /*|| agent->get_step_count() >= 5000*/ || (num_steps && total_steps > 0 && total_steps >= num_steps);

        if(output == "experiment" && total_steps > -1)
          experimental_output.print(size_t(total_steps), agent->get_episode_number(), agent->get_step_count(), reward, done, agent->q_value_count);
      } while(!done);

      if(agent->get_metastate() == Metastate::SUCCESS) {
        if(output == "simple")
          cout << "SUCCESS";
        ++successes;
      }
      else {
        if(output == "simple")
          cout << "FAILURE";
        ++failures;
      }

      if(output == "simple")
        cout << " in " << agent->get_step_count() << " moves, yielding " << agent->get_total_reward() << " total reward." << endl;
    }

    on_episode_termination(agent);

    if(output == "simple") {
      cout << successes << " SUCCESSes" << endl;
      cout << failures << " FAILUREs" << endl;
      cout << agent->q_value_count << " Q-values" << endl;

  //    if(auto mca = dynamic_pointer_cast<Mountain_Car::Agent>(agent))
  //      mca->print_policy(cout, 32);
  //    else if(auto pwa = dynamic_pointer_cast<Puddle_World::Agent>(agent))
  //      pwa->print_policy(cout, 32);
    }
  //   else if(output == "experiment") {
  // //    if(auto cpa = dynamic_pointer_cast<Cart_Pole::Agent>(agent)) {
  // //      cpa->print_value_function_grid(cerr);
  // //      cpa->print_update_count_grid(cerr);
  // //    }
  // //    else
  //     if(auto mca = dynamic_pointer_cast<Mountain_Car::Agent>(agent)) {
  //       mca->print_policy(cerr, 32);
  //       if(!dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value()) {
  //         mca->print_value_function_grid(cerr);
  // //        mca->print_update_count_grid(cerr);
  //       }
  //     }
  //     else if(auto pwa = dynamic_pointer_cast<Puddle_World::Agent>(agent)) {
  //       pwa->print_policy(cerr, 32);
  //       if(!dynamic_cast<const Option_Ranged<bool> &>(Options::get_global()["cmac"]).get_value()) {
  //         pwa->print_value_function_grid(cerr);
  // //        pwa->print_update_count_grid(cerr);
  //       }
  //     }
  //   }
  }

  // static int main2(int argc, char **argv) {
  //   /// Run the simulation
  //
  //   const string env = dynamic_cast<const Option_Itemized &>(options["environment"]).get_value();
  //   if(env == "blocks-world")
  //     run_agent([](){return make_shared<Blocks_World::Environment>();}, [](const shared_ptr<Environment> &env){return make_shared<Blocks_World::Agent>(env);});
  // //  else if(env == "cart-pole")
  // //    run_agent([](){return make_shared<Cart_Pole::Environment>();}, [](const shared_ptr<Environment> &env){return make_shared<Cart_Pole::Agent>(env);});
  //   else if(env == "puddle-world")
  //     run_agent([](){return make_shared<Puddle_World::Environment>();}, [](const shared_ptr<Environment> &env){return make_shared<Puddle_World::Agent>(env);});
  //   else if(env == "mountain-car")
  //     run_agent([](){return make_shared<Mountain_Car::Environment>();}, [](const shared_ptr<Environment> &env){return make_shared<Mountain_Car::Agent>(env);});
  //   else if(env == "tetris")
  //     run_agent([](){return make_shared<Tetris::Environment>();}, [](const shared_ptr<Environment> &env){return make_shared<Tetris::Agent>(env);});
  //   else
  //     throw runtime_error("Internal error: g_args.environment");
  //
  //
  // //  typedef string test_key;
  // //  typedef Q_Value test_value;
  // //  typedef Zeni::Trie<test_key, test_value> test_trie;
  // //
  // //  test_trie * trie = new test_trie;
  // //
  // //  test_trie * key = nullptr;
  // //  (new test_trie("world"))->list_insert_before(key);
  // //  (new test_trie("hello"))->list_insert_before(key);
  // //  key = key->insert(trie, Q_Value::eligible_offset());
  // //  **key = 1.0;
  // //  for(const Q_Value &q : (*key)->eligible)
  // //    cout << q.value << ' ';
  // //  cout << endl;
  // //
  // //  key = nullptr;
  // //  (new test_trie("world"))->list_insert_before(key);
  // //  (new test_trie("hi"))->list_insert_before(key);
  // //  key = key->insert(trie, Q_Value::eligible_offset());
  // //  **key = 2.0;
  // //  for(const Q_Value &q : (*key)->eligible)
  // //    cout << q.value << ' ';
  // //  cout << endl;
  // //
  // //  key = nullptr;
  // //  (new test_trie("hi"))->list_insert_before(key);
  // //  key = key->insert(trie, Q_Value::eligible_offset());
  // //  **key = 3.0;
  // //  for(const Q_Value &q : (*key)->eligible)
  // //    cout << q.value << ' ';
  // //  cout << endl;
  // //
  // //  key = nullptr;
  // //  (new test_trie("world"))->list_insert_before(key);
  // //  (new test_trie("hi"))->list_insert_before(key);
  // //  key = key->insert(trie, Q_Value::eligible_offset());
  // //  for(const Q_Value &q : (*key)->eligible)
  // //    cout << q.value << ' ';
  // //  cout << endl;
  // //
  // //  trie->destroy(trie);
  //
  //
  // //  typedef Zeni::Map<int, int> map_type;
  // //  default_random_engine generator;
  // //  generator.seed(dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value());
  // //  vector<int> numbers;
  // //  int i;
  // //  for(i = 0; i != dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["num-steps"]).get_value(); ++i)
  // //    numbers.push_back(i);
  // //  shuffle(numbers.begin(), numbers.end(), generator);
  // //
  // //  cerr << "seed=" << dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value() << endl;
  // //
  // //  map_type * tree = nullptr;
  // ////  RB_Tree<int> * tree = nullptr;
  // //#ifndef NDEBUG
  // //  cerr << "height=" << tree->debug_height() << ", " << "size=" << tree->debug_size() << endl;
  // ////  tree->debug_print(cerr) << endl;
  // //#endif
  // //
  // //  i = 0;
  // //  for(std::vector<int>::const_iterator it = numbers.begin(), iend = numbers.end(); it != iend; ++it) {
  // //    (new map_type(nullptr, *it))->insert_into(tree);
  // ////    (new RB_Tree<int>(*it))->insert_into(tree);
  // //#ifndef NDEBUG
  // //    cerr << "insertion(" << ++i << ")=" << *it << ", height=" << tree->debug_height() << ", " << "size=" << tree->debug_size() << endl;
  // ////    tree->debug_print(cerr) << endl;
  // //#endif
  // //  }
  // //
  // //  i = 0;
  // //  for(std::vector<int>::const_iterator it = numbers.begin(), iend = numbers.end(); it != iend; ++it) {
  // //#ifndef NDEBUG
  // //    cerr << "deletion(" << ++i << ")=" << *it << endl;
  // //#endif
  // //    if(i == 13)
  // //      cerr << "";
  // //    tree->find(*it)->remove_from(tree);
  // //#ifndef NDEBUG
  // ////    tree->debug_print(cerr) << endl;
  // //    cerr << "deletion(" << i << ")=" << *it << ", height=" << tree->debug_height() << ", " << "size=" << tree->debug_size() << endl;
  // //#endif
  // //  }
  //
  //
  // //  Zeni::Random m_random(dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value());
  // //  cerr << "seed=" << dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value() << endl;
  // //
  // //  Mean mean;
  // //  std::list<std::shared_ptr<Value>> values;
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  values.push_back(make_shared<Value>(m_random.rand_lt(42)));
  // //  for(auto &value : values) {
  // //    mean.contribute(*value);
  // //    cout << "Adding " << *value << " yields " << mean.get_mean() << ':' << mean.get_stddev() << endl;
  // //  }
  // //  for(auto &value : values) {
  // //    mean.uncontribute(*value);
  // //    cout << "Removing " << *value << " yields " << mean.get_mean() << ':' << mean.get_stddev() << endl;
  // //  }
  //
  //
  // //  Zeni::Random m_random(dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value());
  // //  cerr << "seed=" << dynamic_cast<const Option_Ranged<int64_t> &>(Options::get_global()["seed"]).get_value() << endl;
  // //
  // //  Q_Value::List *q_values = nullptr;
  // //  const auto comparator = [](const Q_Value &lhs, const Q_Value &rhs)->bool{
  // //    return lhs.value < rhs.value;
  // //  };
  // //  for(int i = 0; i != 10; ++i) {
  // //    auto val = m_random.rand_lt(42);
  // //    cout << "Inserting: " << val << endl;
  // //    (new Q_Value(val))->current.insert_before_unique(q_values, comparator);
  // //  }
  // //  for(auto &q : *q_values)
  // //    cout << "Value: " << q.value << endl;
  // //  while(q_values) {
  // //    Q_Value::List * const next = q_values->next();
  // //    cout << "Deleting: " << (*q_values)->value << endl;
  // //    q_values->erase();
  // //    delete q_values->get();
  // //    q_values = next;
  // //  }
  //
  //
  //
  //   return 0;
  // }

}
