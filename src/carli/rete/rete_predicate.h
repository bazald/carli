#ifndef RETE_PREDICATE_H
#define RETE_PREDICATE_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Predicate : public Rete_Node {
    Rete_Predicate(const Rete_Predicate &);
    Rete_Predicate & operator=(const Rete_Predicate &);

    friend RETE_LINKAGE void bind_to_predicate(Rete_Agent &agent, const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out);

  public:
    enum Predicate {EQ, NEQ, GT, GTE, LT, LTE};

    Rete_Predicate(const Predicate &predicate_, const WME_Token_Index lhs_index_, const WME_Token_Index rhs_index_);
    Rete_Predicate(const Predicate &predicate_, const WME_Token_Index lhs_index_, const Symbol_Ptr_C &rhs_);

    void destroy(Rete_Agent &agent, const Rete_Node_Ptr &output) override;

    Rete_Node_Ptr_C parent_left() const override {return input->shared();}
    Rete_Node_Ptr_C parent_right() const override {return input->shared();}
    Rete_Node_Ptr parent_left() override {return input->shared();}
    Rete_Node_Ptr parent_right() override {return input->shared();}

    void insert_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;
    bool remove_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;

    void pass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;
    void unpass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;

    bool operator==(const Rete_Node &rhs) const override;

    void print_details(std::ostream &os) const override; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    void print_rule(std::ostream &os) const override;

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

    std::vector<WME> get_filter_wmes() const;

    static Rete_Predicate_Ptr find_existing(const Predicate &predicate, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out);
    static Rete_Predicate_Ptr find_existing(const Predicate &predicate, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out);

    const WME_Token_Index & get_lhs_index() const {return m_lhs_index;}
    const Predicate & get_predicate() const {return m_predicate;}
    const WME_Token_Index & get_rhs_index() const {return m_rhs_index;}
    const Symbol_Ptr_C & get_rhs() const {return m_rhs;}

  private:
    bool test_predicate(const Symbol_Ptr_C &lhs, const Symbol_Ptr_C &rhs) const;

    Predicate m_predicate;
    WME_Token_Index m_lhs_index;
    WME_Token_Index m_rhs_index;
    Symbol_Ptr_C m_rhs;
    Rete_Node * input = nullptr;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> tokens;
  };

  RETE_LINKAGE void bind_to_predicate(Rete_Agent &agent, const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out);

}

#endif
