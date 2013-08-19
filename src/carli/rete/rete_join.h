#ifndef RETE_JOIN_H
#define RETE_JOIN_H

#include "rete_node.h"

namespace Rete {

  class Rete_Join : public Rete_Node {
    Rete_Join(const Rete_Join &);
    Rete_Join & operator=(const Rete_Join &);

    friend void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  public:
    Rete_Join(WME_Bindings bindings_);

    void destroy(Filters &filters, const Rete_Node_Ptr &output);

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from);
    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from);

    bool operator==(const Rete_Node &rhs) const;

    static Rete_Join_Ptr find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  private:
    void join_tokens(const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs);

    WME_Token_Ptr_C join_wme_tokens(const WME_Token_Ptr_C lhs, const WME_Token_Ptr_C &rhs);

    void pass_tokens(const Rete_Node_Ptr &output);

    WME_Bindings bindings;
    std::weak_ptr<Rete_Node> input0;
    std::weak_ptr<Rete_Node> input1;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input0_tokens;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input1_tokens;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> output_tokens;
  };

  void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

}

#endif
