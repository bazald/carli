#ifndef RETE_NEGATION_H
#define RETE_NEGATION_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Negation : public Rete_Node {
    Rete_Negation(const Rete_Negation &);
    Rete_Negation & operator=(const Rete_Negation &);

    friend RETE_LINKAGE void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

  public:
    Rete_Negation();

    void destroy(Filters &filters, const Rete_Node_Ptr &output) override;
    
    Rete_Node_Ptr_C parent_left() const override {return input->shared();}
    Rete_Node_Ptr_C parent_right() const override {return input->shared();}
    Rete_Node_Ptr parent_left() override {return input->shared();}
    Rete_Node_Ptr parent_right() override {return input->shared();}

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;
    bool remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;

    void pass_tokens(const Rete_Node_Ptr &output) override;
    void unpass_tokens(const Rete_Node_Ptr &output) override;

    bool operator==(const Rete_Node &rhs) const override;

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

    static Rete_Negation_Ptr find_existing(const Rete_Node_Ptr &out);

  private:
    Rete_Node * input = nullptr;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input_tokens;
    WME_Token_Ptr_C output_token;
  };

  RETE_LINKAGE void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

}

#endif
