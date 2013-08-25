#ifndef RETE_EXISTENTIAL_H
#define RETE_EXISTENTIAL_H

#include "rete_node.h"

namespace Rete {

  class Rete_Existential : public Rete_Node {
    Rete_Existential(const Rete_Existential &);
    Rete_Existential & operator=(const Rete_Existential &);

    friend void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);

  public:
    Rete_Existential();

    void destroy(Filters &filters, const Rete_Node_Ptr &output);

    Rete_Node_Ptr_C parent() const {return input->shared();}
    Rete_Node_Ptr parent() {return input->shared();}

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from);
    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from);

    void pass_tokens(const Rete_Node_Ptr &output);

    bool operator==(const Rete_Node &rhs) const;

    static Rete_Existential_Ptr find_existing(const Rete_Node_Ptr &out);

  private:
    Rete_Node * input = nullptr;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input_tokens;
    WME_Token_Ptr_C output_token;
  };

  void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out);

}

#endif
