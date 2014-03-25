#ifndef RETE_EXISTENTIAL_H
#define RETE_EXISTENTIAL_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Existential : public Rete_Node {
    Rete_Existential(const Rete_Existential &);
    Rete_Existential & operator=(const Rete_Existential &);

    friend RETE_LINKAGE void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);

  public:
    Rete_Existential();

    void destroy(Filters &filters, const Rete_Node_Ptr &output);

    Rete_Node_Ptr_C parent() const {return input->shared();}
    Rete_Node_Ptr parent() {return input->shared();}

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from);
    bool remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from);

    void pass_tokens(const Rete_Node_Ptr &output);
    void unpass_tokens(const Rete_Node_Ptr &output);

    bool operator==(const Rete_Node &rhs) const;

    static Rete_Existential_Ptr find_existing(const Rete_Node_Ptr &out);

    void output_name(std::ostream &os) const;

  private:
    Rete_Node * input = nullptr;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input_tokens;
    WME_Token_Ptr_C output_token;
  };

  RETE_LINKAGE void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);

}

#endif
