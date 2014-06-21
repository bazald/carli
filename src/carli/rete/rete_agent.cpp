#include "rete_agent.h"

#include <map>

namespace Rete {

  Rete_Agent::Rete_Agent()
  {
  }

  Rete_Action_Ptr Rete_Agent::make_action(const Rete_Action::Action &action, const Rete_Node_Ptr &out) {
    if(auto existing = Rete_Action::find_existing(action, [](const Rete_Action &, const WME_Token &){}, out))
      return existing;
//      std::cerr << "DEBUG: make_action" << std::endl;
    auto action_fun = std::make_shared<Rete_Action>(this->agenda, action, [](const Rete_Action &, const WME_Token &){});
    bind_to_action(action_fun, out);
//      std::cerr << "END: make_action" << std::endl;
    return action_fun;
  }

  Rete_Action_Ptr Rete_Agent::make_action_retraction(const Rete_Action::Action &action, const Rete_Action::Action &retraction, const Rete_Node_Ptr &out) {
    if(auto existing = Rete_Action::find_existing(action, retraction, out))
      return existing;
    auto action_fun = std::make_shared<Rete_Action>(this->agenda, action, retraction);
    bind_to_action(action_fun, out);
    return action_fun;
  }

  Rete_Existential_Ptr Rete_Agent::make_existential(const Rete_Node_Ptr &out) {
    if(auto existing = Rete_Existential::find_existing(out))
      return existing;
    auto existential = std::make_shared<Rete_Existential>();
    bind_to_existential(existential, out);
    return existential;
  }

  Rete_Existential_Join_Ptr Rete_Agent::make_existential_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    if(auto existing = Rete_Existential_Join::find_existing(bindings, out0, out1))
      return existing;
    auto existential_join = std::make_shared<Rete_Existential_Join>(bindings);
    bind_to_existential_join(existential_join, out0, out1);
    return existential_join;
  }

  Rete_Filter_Ptr Rete_Agent::make_filter(const WME &wme) {
    auto filter = std::make_shared<Rete_Filter>(wme);

    for(auto &existing_filter : filters) {
      if(*existing_filter == *filter)
        return existing_filter;
    }

    this->filters.push_back(filter);
    for(auto &wme : this->working_memory.wmes)
      filter->insert_wme(wme);
    return filter;
  }

  Rete_Join_Ptr Rete_Agent::make_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    if(auto existing = Rete_Join::find_existing(bindings, out0, out1))
      return existing;
    auto join = std::make_shared<Rete_Join>(bindings);
    bind_to_join(join, out0, out1);
    return join;
  }

  Rete_Negation_Ptr Rete_Agent::make_negation(const Rete_Node_Ptr &out) {
    if(auto existing = Rete_Negation::find_existing(out))
      return existing;
    auto negation = std::make_shared<Rete_Negation>();
    bind_to_negation(negation, out);
    return negation;
  }

  Rete_Negation_Join_Ptr Rete_Agent::make_negation_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    if(auto existing = Rete_Negation_Join::find_existing(bindings, out0, out1))
      return existing;
    auto negation_join = std::make_shared<Rete_Negation_Join>(bindings);
    bind_to_negation_join(negation_join, out0, out1);
    return negation_join;
  }

  Rete_Predicate_Ptr Rete_Agent::make_predicate_vc(const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out) {
    if(auto existing = Rete_Predicate::find_existing(pred, lhs_index, rhs, out))
      return existing;
    auto predicate = std::make_shared<Rete_Predicate>(pred, lhs_index, rhs);
    bind_to_predicate(predicate, out);
    return predicate;
  }

  Rete_Predicate_Ptr Rete_Agent::make_predicate_vv(const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out) {
    if(auto existing = Rete_Predicate::find_existing(pred, lhs_index, rhs_index, out))
      return existing;
    auto predicate = std::make_shared<Rete_Predicate>(pred, lhs_index, rhs_index);
    bind_to_predicate(predicate, out);
    return predicate;
  }

  void Rete_Agent::source_rule(const std::string &name, const Rete_Action_Ptr &action) {
    auto found = rules.find(name);
    if(found == rules.end()) {
//        std::cerr << "Rule '" << name << "' sourced." << std::endl;
      rules[name] = action;
    }
    else {
//        std::cerr << "Rule '" << name << "' replaced." << std::endl;
      found->second->destroy(filters);
      std::cerr << '#';
      found->second = action;
    }
    std::cerr << '*';
  }

  void Rete_Agent::excise_rule(const std::string &name) {
    auto found = rules.find(name);
    if(found == rules.end()) {
//        std::cerr << "Rule '" << name << "' not found." << std::endl;
    }
    else {
//        std::cerr << "Rule '" << name << "' excised." << std::endl;
      found->second->destroy(filters);
      rules.erase(found);
      std::cerr << '#';
    }
  }

  void Rete_Agent::excise_rule(const Rete_Action_Ptr &action) {
    action->destroy(filters);
    std::cerr << '#';
  }

  Rete_Action_Ptr Rete_Agent::unname_rule(const std::string &name) {
    Rete_Action_Ptr ptr;
    auto found = rules.find(name);
    if(found != rules.end()) {
      ptr = found->second;
      rules.erase(found);
    }
    return ptr;
  }

  void Rete_Agent::insert_wme(const WME_Ptr_C &wme) {
    if(working_memory.wmes.find(wme) != working_memory.wmes.end()) {
#ifdef DEBUG_OUTPUT
      std::cerr << "rete.already_inserted" << *wme << std::endl;
#endif
      return;
    }

    //const auto wme_clone = std::make_shared<WME>(Symbol_Ptr_C(wme->symbols[0]->clone()), Symbol_Ptr_C(wme->symbols[1]->clone()), Symbol_Ptr_C(wme->symbols[2]->clone()));

    agenda.lock();
    working_memory.wmes.insert(wme);
#ifdef DEBUG_OUTPUT
    std::cerr << "rete.insert" << *wme << std::endl;
#endif
    for(auto &filter : filters)
      filter->insert_wme(wme);
    agenda.unlock();
    agenda.run();
  }

  void Rete_Agent::remove_wme(const WME_Ptr_C &wme) {
    auto found = working_memory.wmes.find(wme);

    if(found == working_memory.wmes.end()) {
#ifdef DEBUG_OUTPUT
      std::cerr << "rete.already_removed" << *wme << std::endl;
#endif
      return;
    }

    agenda.lock();
    working_memory.wmes.erase(found);
#ifdef DEBUG_OUTPUT
    std::cerr << "rete.remove" << *wme << std::endl;
#endif
    for(auto &filter : filters)
      filter->remove_wme(wme);
    agenda.unlock();
    agenda.run();
  }

  void Rete_Agent::clear_wmes() {
    agenda.lock();
    for(auto &wme : working_memory.wmes) {
      for(auto &filter : filters)
        filter->remove_wme(wme);
    }
    working_memory.wmes.clear();
    agenda.unlock();
    agenda.run();
  }

  size_t Rete_Agent::rete_size() const {
    size_t size = 0;
    std::function<void (const Rete_Node &)> visitor = [&size](const Rete_Node &){++size;};
    const_cast<Rete_Agent *>(this)->visit_preorder(visitor, true);
    return size;
  }

  void Rete_Agent::rete_print(std::ostream &os) const {
    os << "digraph Rete {" << std::endl;

    std::map<Rete_Node_Ptr_C, std::map<int64_t, std::list<Rete_Node_Ptr_C>>> clusters;
    std::map<Rete_Node_Ptr_C, std::list<Rete_Node_Ptr_C>> cluster_children;

    std::function<void (Rete_Node &)> visitor = [&os, &clusters](Rete_Node &node) {
      node.print_details(os);
      if(node.data) {
        clusters[node.data->cluster_root_ancestor()];
      }
    };
    const_cast<Rete_Agent *>(this)->visit_preorder(visitor, false);

    visitor = [&clusters, &cluster_children](Rete_Node &node) {
      if(dynamic_cast<Rete_Filter *>(&node))
        return;
      const auto found = clusters.find(node.shared());
      if(found == clusters.end()) {
        int64_t i = 1;
        for(auto ancestor = node.parent_left(); !dynamic_cast<Rete_Filter *>(ancestor.get()); ancestor = ancestor->parent_left(), ++i) {
          const auto found2 = clusters.find(ancestor);
          if(found2 != clusters.end()) {
            found2->second[i].push_back(node.shared());
            break;
          }
        }
      }
      else {
        found->second[0].push_back(node.shared());
        for(auto ancestor = node.parent_left(); !dynamic_cast<Rete_Filter *>(ancestor.get()); ancestor = ancestor->parent_left()) {
          const auto found2 = clusters.find(ancestor);
          if(found2 != clusters.end()) {
            cluster_children[found2->first].push_back(found->first);
            break;
          }
        }
      }

    };
    const_cast<Rete_Agent *>(this)->visit_preorder(visitor, true);

    if(!filters.empty()) {
      os << "  { rank=source;";
      for(const auto &filter : filters)
        os << ' ' << intptr_t(filter.get());
      os << " }" << std::endl;
    }

    for(const auto &cluster : clusters) {
      os << "  subgraph cluster" << intptr_t(cluster.first.get()) << " {" << std::endl;
      for(const auto &rank_cluster : cluster.second) {
        os << "    { rank=same;";
        for(const auto &node : rank_cluster.second)
          os << ' ' << intptr_t(node.get());
        os << " }" << std::endl;
      }
      os << "  }" << std::endl;
    }

    ///// Force ranks to be top to bottom
    for(const auto &cluster : clusters) {
      const auto found = cluster_children.find(cluster.first);
      if(found != cluster_children.end()) {
        for(const auto &src : cluster.second.rbegin()->second) {
          for(const auto dest : found->second) {
            for(const auto &dest2 : clusters[dest].begin()->second)
              os << "  " << intptr_t(src.get()) << " -> " << intptr_t(dest2.get()) << " [style=\"invis\"]" << std::endl;
          }
        }
      }
    }

    os << "}" << std::endl;
  }

  void Rete_Agent::destroy() {
    agenda.lock();
    filters.clear();
    agenda.unlock();
    agenda.run();
  }

}
