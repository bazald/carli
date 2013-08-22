#include "rete_agent.h"

namespace Rete {

  Rete_Agent::Rete_Agent()
    : actions(&retractions)
  {
  }

  Rete_Action_Ptr Rete_Agent::make_action(const Rete_Action::Action &action, const Rete_Node_Ptr &out, const bool &attach_immediately) {
    if(auto existing = Rete_Action::find_existing(action, [](const Rete_Action &, const WME_Token &){}, out))
      return existing;
//      std::cerr << "DEBUG: make_action" << std::endl;
    auto action_fun = std::make_shared<Rete_Action>(this->actions, this->retractions, action, [](const Rete_Action &, const WME_Token &){}, attach_immediately);
    bind_to_action(action_fun, out);
//      std::cerr << "END: make_action" << std::endl;
    return action_fun;
  }

  Rete_Action_Ptr Rete_Agent::make_action_retraction(const Rete_Action::Action &action, const Rete_Action::Action &retraction, const Rete_Node_Ptr &out, const bool &attach_immediately) {
    if(auto existing = Rete_Action::find_existing(action, retraction, out))
      return existing;
    auto action_fun = std::make_shared<Rete_Action>(this->actions, this->retractions, action, retraction, attach_immediately);
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

  Rete_Join_Ptr Rete_Agent::make_existential_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    auto join0 = Rete_Join::find_existing(bindings, out0, out1);
    if(!join0)
      join0 = this->make_join(bindings, out0, out1);
    auto existential = Rete_Existential::find_existing(join0);
    if(!existential)
      existential = this->make_existential(join0);
    auto join1 = Rete_Join::find_existing(WME_Bindings(), out0, existential);
    if(!join1)
      join1 = this->make_join(WME_Bindings(), out0, existential);
    return join1;
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

  Rete_Join_Ptr Rete_Agent::make_negation_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    auto join0 = Rete_Join::find_existing(bindings, out0, out1);
    if(!join0)
      join0 = this->make_join(bindings, out0, out1);
    auto negation = Rete_Negation::find_existing(join0);
    if(!negation)
      negation = this->make_negation(join0);
    auto join1 = Rete_Join::find_existing(WME_Bindings(), out0, negation);
    if(!join1)
      join1 = this->make_join(WME_Bindings(), out0, negation);
    return join1;
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
    finish_agenda();
  }

  void Rete_Agent::excise_rule(const Rete_Action_Ptr &action) {
    action->destroy(filters);
  }

  void Rete_Agent::insert_wme(const WME_Ptr_C &wme) {
    assert(working_memory.wmes.find(wme) == working_memory.wmes.end());
    working_memory.wmes.insert(wme);
#ifdef DEBUG_OUTPUT
    std::cerr << "rete.insert" << *wme << std::endl;
#endif
    for(auto ft = filters.begin(), fend = filters.end(); ft != fend; )
      (*ft++)->insert_wme(wme);
    finish_agenda();
  }

  void Rete_Agent::remove_wme(const WME_Ptr_C &wme) {
    auto found = working_memory.wmes.find(wme);
    assert(found != working_memory.wmes.end());
    working_memory.wmes.erase(found);
#ifdef DEBUG_OUTPUT
    std::cerr << "rete.remove" << *wme << std::endl;
#endif
    for(auto ft = filters.begin(), fend = filters.end(); ft != fend; )
      (*ft++)->remove_wme(wme);
    finish_agenda();
  }

  void Rete_Agent::clear_wmes() {
    for(auto &wme : working_memory.wmes) {
      for(auto &filter : filters)
        filter->remove_wme(wme);
    }
    working_memory.wmes.clear();
    finish_agenda();
  }

  size_t Rete_Agent::rete_size() const {
    std::unordered_set<Rete_Node_Ptr_C> nodes;
    std::function<void (const Rete_Node_Ptr_C &node)> grow_rete_size;
    grow_rete_size = [&](const Rete_Node_Ptr_C &node) {
      nodes.insert(node);
      for(const auto &n : node->get_outputs())
        grow_rete_size(n);
    };
    for(const auto &filter : filters)
      grow_rete_size(filter);
    return nodes.size();
  }

  void Rete_Agent::destroy() {
    filters.clear();
  }

  void Rete_Agent::finish_agenda() {
    for(;;) {
      if(!retractions.run() && !actions.run())
        break;
    }
  }

}
