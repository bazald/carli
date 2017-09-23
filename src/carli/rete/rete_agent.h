#ifndef RETE_AGENT_H
#define RETE_AGENT_H

#include "agenda.h"
#include "rete.h"
#include "wme_set.h"
#include <chrono>
#include <unordered_map>

namespace Rete {

  class RETE_LINKAGE Rete_Agent {
    Rete_Agent(Rete_Agent &);
    Rete_Agent & operator=(Rete_Agent &);

  public:
    class RETE_LINKAGE CPU_Accumulator {
      CPU_Accumulator(const CPU_Accumulator &);
      CPU_Accumulator operator=(const CPU_Accumulator &);

    public:
      CPU_Accumulator(Rete_Agent &agent_);
      ~CPU_Accumulator();

    private:
      Rete_Agent &agent;
    };

    Rete_Agent();
    ~Rete_Agent();

    Rete_Action_Ptr make_action(const std::string &name, const bool &user_action, const Rete_Action::Action &action, const Rete_Node_Ptr &out, const Variable_Indices_Ptr_C &variables);
    Rete_Action_Ptr make_action_retraction(const std::string &name, const bool &user_action, const Rete_Action::Action &action, const Rete_Action::Action &retraction, const Rete_Node_Ptr &out, const Variable_Indices_Ptr_C &variables);
    Rete_Existential_Ptr make_existential(const Rete_Node_Ptr &out);
    Rete_Existential_Join_Ptr make_existential_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    Rete_Filter_Ptr make_filter(const WME &wme);
    Rete_Join_Ptr make_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    Rete_Negation_Ptr make_negation(const Rete_Node_Ptr &out);
    Rete_Negation_Join_Ptr make_negation_join(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);
    Rete_Predicate_Ptr make_predicate_vc(const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out);
    Rete_Predicate_Ptr make_predicate_vv(const Rete_Predicate::Predicate &pred, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out);

    Agenda & get_agenda() {return agenda;}
    Rete_Action_Ptr get_rule(const std::string &name);
    std::set<std::string> get_rule_names() const;
    int64_t get_rule_name_index() const {return rule_name_index;}
    void set_rule_name_index(const int64_t &rule_name_index_) {rule_name_index = rule_name_index_;}

    void excise_all();
    void excise_filter(const Rete_Filter_Ptr &filter);
    void excise_rule(const std::string &name, const bool &user_command);
    std::string next_rule_name(const std::string &prefix);
    Rete_Action_Ptr unname_rule(const std::string &name, const bool &user_command);

    void insert_wme(const WME_Ptr_C &wme);
    void remove_wme(const WME_Ptr_C &wme);
    void clear_wmes();

    double rete_cpu_time() const {return m_cpu_time;}
    size_t rete_size() const;
    void rete_print(std::ostream &os) const; ///< Formatted for dot: http://www.graphviz.org/content/dot-language
    void rete_print_rules(std::ostream &os) const;
    void rete_print_firing_counts(std::ostream &os) const;
    void rete_print_matches(std::ostream &os) const;

    template <typename VISITOR>
    VISITOR visit_preorder(VISITOR visitor, const bool &strict) {
      visitor_value = visitor_value != 1 ? 1 : 2;

      for(auto &o : filters)
        visitor = o->visit_preorder(visitor, strict, visitor_value);
      return visitor;
    }

  protected:
    void destroy();

    Agenda agenda;

  private:
    void source_rule(const Rete_Action_Ptr &action, const bool &user_command);

    Rete_Node::Filters filters;
    std::unordered_map<std::string, Rete_Action_Ptr> rules;
    int64_t rule_name_index = 0;
    WME_Set working_memory;
    intptr_t visitor_value = 0;

    double m_cpu_time = 0.0;
    size_t m_rete_depth = 0;
    std::chrono::high_resolution_clock::time_point m_start;
  };

}

#endif
