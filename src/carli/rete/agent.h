#ifndef RETE_AGENT_H
#define RETE_AGENT_H

#include "rete.h"
#include "wme_set.h"

namespace Rete {

  class Agent {
    Agent(Agent &);
    Agent & operator=(Agent &);

  public:
    const std::function<Rete_Action_Ptr (const Rete_Action::Action &action, const Rete_Node_Ptr &out)> make_action;
    const std::function<Rete_Action_Ptr (const Rete_Action::Action &action, const Rete_Action::Action &retraction, const Rete_Node_Ptr &out)> make_action_retraction;
    const std::function<Rete_Existential_Ptr (const Rete_Node_Ptr &out)> make_existential;
    const std::function<Rete_Existential_Join_Ptr (const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1)> make_existential_join;
    const std::function<Rete_Filter_Ptr (const WME &wme)> make_filter;
    const std::function<Rete_Join_Ptr (const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1)> make_join;
    const std::function<Rete_Negation_Ptr (const Rete_Node_Ptr &out)> make_negation;
    const std::function<Rete_Negation_Join_Ptr (const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1)> make_negation_join;
    const std::function<Rete_Predicate_Ptr (const Rete_Predicate::Predicate &predicate, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out)> make_predicate_vc;
    const std::function<Rete_Predicate_Ptr (const Rete_Predicate::Predicate &predicate, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out)> make_predicate_vv;

    Agent()
      : make_action([](const Rete_Action::Action &action, const Rete_Node_Ptr &out)->Rete_Action_Ptr {
          if(auto existing = Rete_Action::find_existing(action, [](const Rete_Action &, const WME_Token &){}, out))
            return existing;
          auto action_fun = std::make_shared<Rete_Action>(action);
          bind_to_action(action_fun, out);
          return action_fun;
      }),
      make_action_retraction([](const Rete_Action::Action &action, const Rete_Action::Action &retraction, const Rete_Node_Ptr &out)->Rete_Action_Ptr {
        if(auto existing = Rete_Action::find_existing(action, retraction, out))
          return existing;
        auto action_fun = std::make_shared<Rete_Action>(action, retraction);
        bind_to_action(action_fun, out);
        return action_fun;
      }),
      make_existential([](const Rete_Node_Ptr &out)->Rete_Existential_Ptr {
        if(auto existing = Rete_Existential::find_existing(out))
          return existing;
        auto existential = std::make_shared<Rete_Existential>();
        bind_to_existential(existential, out);
        return existential;
      }),
      make_existential_join([](const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1)->Rete_Existential_Join_Ptr {
        if(auto existing = Rete_Existential_Join::find_existing(bindings, out0, out1))
          return existing;
        auto existential_join = std::make_shared<Rete_Existential_Join>(bindings);
        bind_to_existential_join(existential_join, out0, out1);
        return existential_join;
      }),
      make_filter([this](const WME &wme)->Rete_Filter_Ptr {
        auto filter = std::make_shared<Rete_Filter>(wme);

        for(auto &existing_filter : filters) {
          if(*existing_filter == *filter)
            return existing_filter;
        }

        this->filters.push_back(filter);
        for(auto &wme : this->working_memory.wmes)
          filter->insert_wme(wme);
        return filter;
      }),
      make_join([](const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1)->Rete_Join_Ptr {
        if(auto existing = Rete_Join::find_existing(bindings, out0, out1))
          return existing;
        auto join = std::make_shared<Rete_Join>(bindings);
        bind_to_join(join, out0, out1);
        return join;
      }),
      make_negation([](const Rete_Node_Ptr &out)->Rete_Negation_Ptr {
        if(auto existing = Rete_Negation::find_existing(out))
          return existing;
        auto negation = std::make_shared<Rete_Negation>();
        bind_to_negation(negation, out);
        return negation;
      }),
      make_negation_join([](const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1)->Rete_Negation_Join_Ptr {
        if(auto existing = Rete_Negation_Join::find_existing(bindings, out0, out1))
          return existing;
        auto negation_join = std::make_shared<Rete_Negation_Join>(bindings);
        bind_to_negation_join(negation_join, out0, out1);
        return negation_join;
      }),
      make_predicate_vc([](const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out)->Rete_Predicate_Ptr {
        if(auto existing = Rete_Predicate::find_existing(pred, lhs_index, rhs, out))
          return existing;
        auto predicate = std::make_shared<Rete_Predicate>(pred, lhs_index, rhs);
        bind_to_predicate(predicate, out);
        return predicate;
      }),
      make_predicate_vv([](const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out)->Rete_Predicate_Ptr {
        if(auto existing = Rete_Predicate::find_existing(pred, lhs_index, rhs_index, out))
          return existing;
        auto predicate = std::make_shared<Rete_Predicate>(pred, lhs_index, rhs_index);
        bind_to_predicate(predicate, out);
        return predicate;
      })
    {
    }

    void source_rule(const std::string &name, const Rete_Action_Ptr &action) {
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

    void excise_rule(const std::string &name) {
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

    void excise_rule(const Rete_Action_Ptr &action) {
      action->destroy(filters);
    }

    void insert_wme(const WME_Ptr_C &wme) {
      assert(working_memory.wmes.find(wme) == working_memory.wmes.end());
      working_memory.wmes.insert(wme);
      for(auto &filter : filters)
        filter->insert_wme(wme);
    }

    void remove_wme(const WME_Ptr_C &wme) {
      auto found = working_memory.wmes.find(wme);
      assert(found != working_memory.wmes.end());
      working_memory.wmes.erase(found);
      for(auto &filter : filters)
        filter->remove_wme(wme);
    }

    size_t rete_size() const {
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

  private:
    std::list<Rete_Filter_Ptr> filters;
    std::unordered_map<std::string, Rete_Action_Ptr> rules;
    WME_Set working_memory;
  };

}

#endif
