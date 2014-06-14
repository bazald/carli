#ifndef RETE_FILTER_H
#define RETE_FILTER_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Filter : public Rete_Node {
    Rete_Filter(const Rete_Filter &);
    Rete_Filter & operator=(const Rete_Filter &);

  public:
    Rete_Filter(const WME &wme_);

    const WME & get_wme() const;

    void destroy(Filters &filters, const Rete_Node_Ptr &output) override;
    
    Rete_Node_Ptr_C parent_left() const override {abort();}
    Rete_Node_Ptr_C parent_right() const override {abort();}
    Rete_Node_Ptr parent_left() override {abort();}
    Rete_Node_Ptr parent_right() override {abort();}

    int64_t height() const override {return 1;}

    void insert_wme(const WME_Ptr_C &wme);
    void remove_wme(const WME_Ptr_C &wme);

    void insert_wme_token(const WME_Token_Ptr_C &, const Rete_Node * const &) override;
    bool remove_wme_token(const WME_Token_Ptr_C &, const Rete_Node * const &) override;

    void pass_tokens(Rete_Node * const &output) override;
    void unpass_tokens(Rete_Node * const &output) override;

    bool operator==(const Rete_Node &rhs) const override;

    void print_details(std::ostream &os) const override; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

  private:
    WME m_wme;
    std::array<Symbol_Variable_Ptr_C, 3> m_variable;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> tokens;
  };

}

#endif
