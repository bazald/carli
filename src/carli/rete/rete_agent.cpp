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
      found->second = action;
    }
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
    }
  }

  void Rete_Agent::excise_rule(const Rete_Action_Ptr &action) {
    action->destroy(filters);
  }

  void Rete_Agent::insert_wme(const WME_Ptr_C &wme) {
    assert(working_memory.wmes.find(wme) == working_memory.wmes.end());
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
    assert(found != working_memory.wmes.end());
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
    
    std::map<int64_t, std::list<Rete_Node_Ptr_C>> clusters;
    std::map<int64_t, Rete_Node_Ptr_C> cluster_owners;
    std::map<int64_t, std::list<Rete_Node_Ptr_C>> ranks;
    std::function<void (Rete_Node &)> visitor = [&os, &clusters, &cluster_owners, &ranks](Rete_Node &node) {
      node.print_details(os);
      if(node.data) {
        clusters[node.data->cluster()].push_back(node.shared());
        cluster_owners[node.data->cluster_owner()] = node.shared();
        ranks[node.data->rank()].push_back(node.shared());
      }
    };

    const_cast<Rete_Agent *>(this)->visit_preorder(visitor, false);

    if(!filters.empty()) {
      os << "  { rank=source;";
      for(const auto &filter : filters)
        os << ' ' << intptr_t(filter.get());
      os << " }" << std::endl;
    }

    for(const auto &cluster : clusters) {
      if(cluster.first) {
        for(const auto &node : cluster.second) {
          os << "  " << intptr_t(cluster_owners[cluster.first].get()) << " -> ";
          if(node->data->attached_parent())
            os << intptr_t(node->parent_left().get());
          else
            os << intptr_t(node.get());
          os << " [arrowhead=\"none\",style=\"dashed\"]" << std::endl;
        }
      }

      os << "  subgraph cluster" << cluster.first << " {" << std::endl;
      os << "    { rank=same;";
      for(const auto &node : cluster.second) {
        if(node->data->attached_parent())
          os << ' ' << intptr_t(node->parent_left().get());
      }
      os << " }" << std::endl << "    { rank=same;";
      for(const auto &node : cluster.second)
        os << ' ' << intptr_t(node.get());
      os << " }" << std::endl;
      os << "  }" << std::endl;
    }

    if(ranks.size() > 1) {
      auto prev = ranks.begin();
      for(auto cur = ++ranks.begin(); cur != ranks.end(); prev = cur, ++cur) {
        for(const auto &node0 : prev->second) {
          for(const auto &node1 : cur->second) {
            os << "  " << intptr_t(node0.get()) << " -> ";
            if(node1->data->attached_parent())
              os << intptr_t(node1->parent_left().get());
            else
              os << intptr_t(node1.get());
            os << " [style=\"invis\"]" << std::endl;
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
