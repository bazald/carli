#ifndef RETE_NEGATION_H
#define RETE_NEGATION_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Negation : public Rete_Node {
    Rete_Negation(const Rete_Negation &);
    Rete_Negation & operator=(const Rete_Negation &);

    friend void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

  public:
    Rete_Negation();

    void destroy(Filters &filters, const Rete_Node_Ptr &output);

    Rete_Node_Ptr_C parent() const {return input->shared();}
    Rete_Node_Ptr parent() {return input->shared();}

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from);
    bool remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from);

    void pass_tokens(const Rete_Node_Ptr &output);
    void unpass_tokens(const Rete_Node_Ptr &output);

    bool operator==(const Rete_Node &rhs) const;

    static Rete_Negation_Ptr find_existing(const Rete_Node_Ptr &out);

    void output_name(std::ostream &os) const;

  private:
    Rete_Node * input = nullptr;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input_tokens;
    WME_Token_Ptr_C output_token;
  };

  RETE_LINKAGE void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out);

}

#endif
