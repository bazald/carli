#include "carli/parser/rete_parser.h"
#include "carli/experiment.h"
#include "blocks_world_2_env/blocks_world_2.h"

#include <sstream>

using namespace std;

// template <typename TYPE, typename NUMBER>
// map<TYPE, double> normalize(const map<TYPE, NUMBER> &to_number) {
//   map<TYPE, double> to_double;
//   NUMBER max_val = 0;
//   for(auto entry : to_number)
//     max_val = max(max_val, abs(entry.second));
//   for(auto entry : to_number)
//     to_double[entry.first] = double(entry.second) / max_val;
//   return to_double;
// }

template <typename TYPE, typename NUMBER>
map<pair<TYPE, TYPE>, double> first_vs_second_ratio(const map<pair<TYPE, TYPE>, NUMBER> &to_number) {
  map<pair<TYPE, TYPE>, double> fvsr;
  for(auto entry : to_number) {
    const auto converse = to_number.find(make_pair(entry.first.second, entry.first.first));
    if(converse != to_number.end())
      fvsr[entry.first] = double(entry.second) / (entry.second + converse->second);
    else
      fvsr[entry.first] = 1.0;
  }
  return fvsr;
}

template <typename TYPE>
map<TYPE, double> simplify_and_purge(const map<pair<TYPE, TYPE>, double> &in, const TYPE &purge_value) {
  map<TYPE, double> out;
  for(auto entry : in) {
    if(entry.first.first != purge_value)
      out[entry.first.first] = entry.second;
  }
  return out;
}

template <typename TYPE>
map<TYPE, double> combine(const map<TYPE, double> &cumulative, const map<TYPE, double> &next, const int64_t &nth) {
  map<TYPE, double> combined;
  for(auto entry : cumulative)
    combined[entry.first] = double(entry.second) * ((nth - 1.0) / nth);
  for(auto entry : next)
    combined[entry.first] += double(entry.second) / nth;
  return combined;
}

template <typename TYPE>
multimap<double, TYPE, greater<double>> converse_descending(const map<TYPE, double> &to_double) {
  multimap<double, TYPE, greater<double>> from_double;
  for(auto entry : to_double)
    from_double.insert(make_pair(entry.second, entry.first));
  return from_double;
}

string cleanup(const string &str) {
  string rv;
  for(char c : str) {
    if(c == ' ' && !rv.empty() && *rv.rbegin() == ' ')
      rv.pop_back();
    else if(c != '\r' && c != '\n')
      rv += c;
  }
  return rv;
}

/// Deliberately ignore +/-
string last_condition(const string &str) {
  int end = str.rfind(')');
  int begin = str.rfind('(', end) + 1;
  string rv = str.substr(begin, end - begin);

  return rv;
}

string strip_predicate(const string &cond) {
  int pred = min(cond.find(" < "), min(cond.find(" > "), min(cond.find(" <= "), min(cond.find(" >= "), min(cond.find(" == "), cond.find(" != "))))));
  if(pred != -1)
    return cond.substr(0, pred);
  return cond;
}

struct Stats {
  map<pair<string, string>, int64_t> before;
  unordered_set<Rete::Rete_Action_Ptr_C> visited;

  void operator()(Rete::Rete_Node &rete_node) {
    Rete::Rete_Action_Ptr rete_action = dynamic_pointer_cast<Rete::Rete_Action>(rete_node.shared());
    if(!rete_action)
      return;

    if(visited.find(rete_action) != visited.end())
      return;
    visited.insert(rete_action);
    
    auto node = dynamic_pointer_cast<Carli::Node>(rete_action->data);
    if(!node)
      abort();

    const string right = strip_predicate(last_condition(cleanup(rete_action->get_text_right())));
    tracked_ptr<Carli::Feature> right_feature = node->q_value_fringe->feature;
    list<pair<string, tracked_ptr<Carli::Feature>>> lefts;
    collect_left(node->parent_action.lock(), lefts);
    
    for(auto left : lefts) {
//       if(left.second->compare_axis(*right_feature) != 0)
      if(left.first != right)
        ++before[make_pair(left.first, right)];
    }
  }
  
  bool collect_left(const Rete::Rete_Action_Ptr &rete_action, list<pair<string, tracked_ptr<Carli::Feature>>> &lefts) {
    if(!rete_action)
      return false;

    auto node = dynamic_pointer_cast<Carli::Node>(rete_action->data);
    if(!node)
      abort();

    auto locked = node->parent_action.lock();
    if(locked) {
      if(collect_left(locked, lefts))
        lefts.push_front(make_pair(strip_predicate(last_condition(cleanup(locked->get_text_right()))), node->q_value_fringe->feature));
      return true;
    }

    return false;
  }
  
  map<pair<string, string>, int64_t> before_all() const {
    map<pair<string, string>, int64_t> ba;
    for(auto entry : before) {
      ba[make_pair(entry.first.first, "*")] += entry.second;
      ba[make_pair("*", entry.first.second)] += entry.second;
    }
    
    return ba;
  }
  
  map<pair<string, string>, int64_t> before_other() const {
    return before;
  }
};

int main(int argc, char **argv) {
  try {
    Options &options = Options::get_global();
    Carli::Experiment experiment;

    if(experiment.take_args(argc, argv) < argc) {
      shared_ptr<Carli::Environment> env = make_shared<Blocks_World_2::Environment>();
      shared_ptr<Carli::Agent> agent = make_shared<Blocks_World_2::Agent>(env);

      map<string, double> before_all;
      map<pair<string, string>, double> before_other;
      int64_t nth = 0;

      for(int64_t i = options.optind; !Rete::rete_get_exit() && i != argc; ++i) {
        Rete::rete_parse_file(*agent, argv[i]);
        
        Stats stats = agent->visit_preorder(Stats(), true);
        ++nth;
        before_all = combine(before_all, simplify_and_purge(first_vs_second_ratio(stats.before_all()), string("*")), nth);
        before_other = combine(before_other, first_vs_second_ratio(stats.before_other()), nth);
        
        agent->excise_all();
      }
      
      for(auto entry : converse_descending(before_all))
        cout << entry.first << ' ' << entry.second << " < *" << endl;
      cout << endl;
      for(auto entry : converse_descending(before_other))
        cout << entry.first << ' ' << entry.second.first << " < " << entry.second.second << endl;
    }

    return 0;
  }
  catch(exception &ex) {
    cerr << "Exiting with exception: " << ex.what() << endl;
  }
  catch(...) {
    cerr << "Exiting with unknown exception." << endl;
  }

  return -1;
}
