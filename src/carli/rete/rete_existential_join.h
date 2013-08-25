#ifndef RETE_EXISTENTIAL_JOIN_H
#define RETE_EXISTENTIAL_JOIN_H

#include "rete_node.h"

namespace Rete {

  class Rete_Existential_Join : public Rete_Node {
    Rete_Existential_Join(const Rete_Existential_Join &);
    Rete_Existential_Join & operator=(const Rete_Existential_Join &);

    friend void bind_to_existential_join(const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  public:
    Rete_Existential_Join(WME_Bindings bindings_);

    void destroy(Filters &filters, const Rete_Node_Ptr &output);

    Rete_Node_Ptr_C parent() const {return input0->shared();}
    Rete_Node_Ptr parent() {return input0->shared();}

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from);
    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from);

    bool operator==(const Rete_Node &rhs) const;

    static Rete_Existential_Join_Ptr find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  private:
    void join_tokens(std::pair<WME_Token_Ptr_C, size_t> &lhs, const WME_Token_Ptr_C &rhs);
    void unjoin_tokens(std::pair<WME_Token_Ptr_C, size_t> &lhs, const WME_Token_Ptr_C &rhs);

    void pass_tokens(const Rete_Node_Ptr &output);

    WME_Bindings bindings;
    Rete_Node * input0 = nullptr;
    Rete_Node * input1 = nullptr;
    std::list<std::pair<WME_Token_Ptr_C, size_t>, Zeni::Pool_Allocator<std::pair<WME_Token_Ptr_C, size_t>>> input0_tokens;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input1_tokens;
  };

  void bind_to_existential_join(const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

}

#endif
