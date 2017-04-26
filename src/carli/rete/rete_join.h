#ifndef RETE_JOIN_H
#define RETE_JOIN_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Join : public Rete_Node {
    Rete_Join(const Rete_Join &);
    Rete_Join & operator=(const Rete_Join &);

    friend RETE_LINKAGE void bind_to_join(Rete_Agent &agent, const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  public:
    Rete_Join(WME_Bindings bindings_);

    void destroy(Rete_Agent &agent, const Rete_Node_Ptr &output) override;

    Rete_Node_Ptr_C parent_left() const override {return input0->shared();}
    Rete_Node_Ptr_C parent_right() const override {return input1->shared();}
    Rete_Node_Ptr parent_left() override {return input0->shared();}
    Rete_Node_Ptr parent_right() override {return input1->shared();}

    Rete_Filter_Ptr_C get_filter(const int64_t &index) const override;

    const Tokens & get_output_tokens() const override;
    bool has_output_tokens() const override;

    void insert_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;
    bool remove_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;

    bool operator==(const Rete_Node &rhs) const override;

    bool disabled_input(const Rete_Node_Ptr &input) override;

    void print_details(std::ostream &os) const override; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    void print_rule(std::ostream &os, const Variable_Indices_Ptr_C &indices) const override;

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

    std::vector<WME> get_filter_wmes() const override;

    static Rete_Join_Ptr find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

    virtual const WME_Bindings * get_bindings() const override {return &bindings;}

  private:
    void join_tokens(Rete_Agent &agent, const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs);

    WME_Token_Ptr_C join_wme_tokens(const WME_Token_Ptr_C lhs, const WME_Token_Ptr_C &rhs) const;

    void disconnect(Rete_Agent &agent, const Rete_Node * const &from) override;

    void pass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;
    void unpass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;

    WME_Bindings bindings;
    Rete_Node * input0 = nullptr;
    Rete_Node * input1 = nullptr;
    int64_t input0_count = 0;
    int64_t input1_count = 0;

    std::unordered_map<std::list<Symbol_Ptr_C>, std::pair<Tokens, Tokens>, Rete::hash_container_deref<Symbol>, Rete::compare_container_deref_eq> matching;
    Tokens output_tokens;

    struct Connected {
      Connected() : connected0(true), connected1(true) {}

      bool connected0 : 1;
      bool connected1 : 1;
    } data;
  };

  RETE_LINKAGE void bind_to_join(Rete_Agent &agent, const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

}

#endif
