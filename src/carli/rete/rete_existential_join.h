#ifndef RETE_EXISTENTIAL_JOIN_H
#define RETE_EXISTENTIAL_JOIN_H

#include "rete_node.h"

namespace Rete {

  class RETE_LINKAGE Rete_Existential_Join : public Rete_Node {
    Rete_Existential_Join(const Rete_Existential_Join &);
    Rete_Existential_Join & operator=(const Rete_Existential_Join &);

    friend RETE_LINKAGE void bind_to_existential_join(Rete_Agent &agent, const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  public:
    Rete_Existential_Join(WME_Bindings bindings_, const bool &match_tokens_);

    void destroy(Rete_Agent &agent, const Rete_Node_Ptr &output) override;

    Rete_Node_Ptr_C parent_left() const override {return input0->shared();}
    Rete_Node_Ptr_C parent_right() const override {return input1->shared();}
    Rete_Node_Ptr parent_left() override {return input0->shared();}
    Rete_Node_Ptr parent_right() override {return input1->shared();}

    void insert_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;
    bool remove_wme_token(Rete_Agent &agent, const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) override;

    bool operator==(const Rete_Node &rhs) const override;

    bool disabled_input(const Rete_Node_Ptr &input) override;

    void print_details(std::ostream &os) const override; ///< Formatted for dot: http://www.graphviz.org/content/dot-language

    void print_rule(std::ostream &os, const Variable_Indices_Ptr_C &indices, const int64_t &offset) const override;

    void output_name(std::ostream &os, const int64_t &depth) const override;

    bool is_active() const override;

    std::vector<WME> get_filter_wmes() const;

    static Rete_Existential_Join_Ptr find_existing(const WME_Bindings &bindings, const bool &match_tokens, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

    virtual const WME_Bindings * get_bindings() const override {return &bindings;}

  private:
    void join_tokens(Rete_Agent &agent, std::pair<WME_Token_Ptr_C, size_t> &lhs, const WME_Token_Ptr_C &rhs);
    void unjoin_tokens(Rete_Agent &agent, std::pair<WME_Token_Ptr_C, size_t> &lhs, const WME_Token_Ptr_C &rhs);

    void disconnect(Rete_Agent &agent, const Rete_Node * const &from);

    void pass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;
    void unpass_tokens(Rete_Agent &agent, Rete_Node * const &output) override;

    WME_Bindings bindings;
    Rete_Node * input0 = nullptr;
    Rete_Node * input1 = nullptr;
    std::list<std::pair<WME_Token_Ptr_C, size_t>, Zeni::Pool_Allocator<std::pair<WME_Token_Ptr_C, size_t>>> input0_tokens;
    std::list<WME_Token_Ptr_C, Zeni::Pool_Allocator<WME_Token_Ptr_C>> input1_tokens;

    struct Data {
      Data(const bool &match_tokens_) : connected0(true), connected1(false), match_tokens(match_tokens_) {}

      bool connected0 : 1;
      bool connected1 : 1;
      bool match_tokens : 1;
    } data;
  };

  RETE_LINKAGE void bind_to_existential_join(Rete_Agent &agent, const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

}

#endif
