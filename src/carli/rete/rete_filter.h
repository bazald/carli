#ifndef RETE_FILTER_H
#define RETE_FILTER_H

#include "rete_node.h"

namespace Rete {

  class Rete_Filter : public Rete_Node {
    Rete_Filter(const Rete_Filter &);
    Rete_Filter & operator=(const Rete_Filter &);

  public:
    Rete_Filter(const WME &wme_);

    const WME & get_wme() const;

    void destroy(Filters &filters, const Rete_Node_Ptr &output);

    Rete_Node_Ptr_C parent() const {abort();}
    Rete_Node_Ptr parent() {abort();}

    void insert_wme(const WME_Ptr_C &wme);
    void remove_wme(const WME_Ptr_C &wme);

    void insert_wme_token(const WME_Token_Ptr_C &, const Rete_Node_Ptr_C &);
    void remove_wme_token(const WME_Token_Ptr_C &, const Rete_Node_Ptr_C &);

    void pass_tokens(const Rete_Node_Ptr &output);

    bool operator==(const Rete_Node &rhs) const;

  private:
    WME m_wme;
    std::array<Symbol_Variable_Ptr_C, 3> m_variable;
    std::list<std::pair<WME_Ptr_C, WME_Token_Ptr_C>, Zeni::Pool_Allocator<std::pair<WME_Ptr_C, WME_Token_Ptr_C>>> tokens;
  };

}

#endif
