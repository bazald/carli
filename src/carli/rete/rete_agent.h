#ifndef RETE_AGENT_H
#define RETE_AGENT_H

#include "rete.h"
#include "wme_set.h"
#include <unordered_map>

namespace Rete {

  class RETE_LINKAGE Rete_Agent {
    Rete_Agent(Rete_Agent &);
    Rete_Agent & operator=(Rete_Agent &);

  public:
    Rete_Agent();

    Rete_Action_Ptr make_action(const Rete_Action::Action &action, const Rete_Node_Ptr &out);
    Rete_Action_Ptr make_action_retraction(const Rete_Action::Action &action, const Rete_Action::Action &retraction, const Rete_Node_Ptr &out);
    Rete_Existential_Ptr make_existential(const Rete_Node_Ptr &out);
    Rete_Existential_Join_Ptr make_existential_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    Rete_Filter_Ptr make_filter(const WME &wme);
    Rete_Join_Ptr make_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    Rete_Negation_Ptr make_negation(const Rete_Node_Ptr &out);
    Rete_Negation_Join_Ptr make_negation_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    Rete_Predicate_Ptr make_predicate_vc(const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out);
    Rete_Predicate_Ptr make_predicate_vv(const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out);

    void source_rule(const std::string &name, const Rete_Action_Ptr &action);

    void excise_rule(const std::string &name);
    void excise_rule(const Rete_Action_Ptr &action);

    void insert_wme(const WME_Ptr_C &wme);
    void remove_wme(const WME_Ptr_C &wme);
    void clear_wmes();

    size_t rete_size() const;

    template <typename VISITOR>
    VISITOR visit_preorder(VISITOR visitor) {
      visitor_value = visitor_value != 1 ? 1 : 2;

      for(auto &o : filters)
        visitor = o->visit_preorder(visitor, visitor_value);
      return visitor;
    }

  protected:
    void destroy();

  private:
    Rete_Node::Filters filters;
    std::unordered_map<std::string, Rete_Action_Ptr> rules;
    WME_Set working_memory;
    Agenda agenda;
    intptr_t visitor_value = 0;
  };

}

#endif
