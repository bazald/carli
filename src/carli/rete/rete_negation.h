#ifndef RETE_NEGATION_H
#define RETE_NEGATION_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Negation : public Rete_Node {
    Rete_Negation(const Rete_Negation &);
    Rete_Negation & operator=(const Rete_Negation &);

    friend RETE_LINKAGE void bind_to_negation(Rete_Agent &agent, const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

  public:
    Rete_Negation();

    void destroy(Rete_Agent &agent, const Rete_Node_Ptr &output) override;

    Rete_Node_Ptr_C parent_left() const override {return input->shared();}
    Rete_Node_Ptr_C parent_right() const override {return input->shared();}
    Rete_Node_Ptr parent_left() override {return input->shared();}
    Rete_Node_Ptr parent_right() override {return input->shared();}

    Rete_Filter_Ptr_C get_filter(const int64_t &index) const override;

    const Tokens & get_output_tokens() const override;
    bool has_output_tokens() const override;

    void insert_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;
    bool remove_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;

    void pass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;
    void unpass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;

    bool operator==(const Rete_Node &rhs) const override;

    void print_details(std::ostream &os) const override; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    void print_rule(std::ostream &os, const Variable_Indices_Ptr_C &indices, const bool &suppress_parent_left) const override;

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

    std::vector<WME> get_filter_wmes() const override;

    static Rete_Negation_Ptr find_existing(const Rete_Node_Ptr &out);

  private:
    Rete_Node * input = nullptr;
    Tokens input_tokens;
    Tokens output_tokens;
    WME_Token_Ptr_C output_token;
  };

  RETE_LINKAGE void bind_to_negation(Rete_Agent &agent, const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

}

#endif
