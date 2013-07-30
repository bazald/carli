#ifndef RETE_NEGATION_JOIN_H
#define RETE_NEGATION_JOIN_H

#include "rete_node.h"
#include <unordered_map>

namespace Rete {

  class Rete_Negation_Join : public Rete_Node {
    Rete_Negation_Join(const Rete_Negation_Join &);
    Rete_Negation_Join & operator=(const Rete_Negation_Join &);

    friend void bind_to_negation_join(const Rete_Negation_Join_Ptr &negation_join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1);

  public:
    Rete_Negation_Join(WME_Bindings bindings_) : bindings(bindings_) {}

    void destroy(std::list<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &output) {
      erase_output(output);
      if(outputs.empty()) {
        auto i0 = input0.lock();
        auto i1 = input1.lock();
        i0->destroy(filters, shared());
        if(i0 != i1)
          i1->destroy(filters, shared());
      }
    }

    void insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from) {
      assert(from == input0.lock() || from == input1.lock());

      if(from == input0.lock()) {
        bool fresh;
        if(find_key(input0_tokens, wme_token) == input0_tokens.end()) {
          input0_tokens.push_back(std::make_pair(wme_token, 0lu));
          fresh = true;
        }
        else
          fresh = false;

        for(const auto &other : input1_tokens)
          join_tokens(wme_token, other);

        if(fresh && find_key(input0_tokens, wme_token)->second == 0) {
          output_tokens.push_back(wme_token);
          for(auto &output : outputs)
            output->insert_wme_token(wme_token, shared());
        }
      }
      if(from == input1.lock()) {
        input1_tokens.push_back(wme_token);

        for(const auto &other : input0_tokens)
          join_tokens(other.first, wme_token);
      }
    }

    void remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from) {
      assert(from == input0.lock() || from == input1.lock());

      if(from == input0.lock()) {
        auto found = find_key(input0_tokens, wme_token);
        if(found != input0_tokens.end()) {
          // TODO: Avoid looping through non-existent pairs?
          input0_tokens.erase(found);
          for(auto &output : outputs)
            output->remove_wme_token(wme_token, shared());
          output_tokens.erase(find(output_tokens, wme_token));
        }
      }
      if(from == input1.lock()) {
        auto found = find(input1_tokens, wme_token);
        if(found != input1_tokens.end()) {
          // TODO: Avoid looping through non-existent pairs?
          input1_tokens.erase(found);
          for(const auto &other : input0_tokens) {
            for(auto &binding : bindings) {
              if(*(*other.first)[binding.first] != *(*wme_token)[binding.second])
                continue;
            }
            if(--find_key(input0_tokens, other.first)->second == 0) {
              output_tokens.push_back(other.first);
              for(auto &output : outputs)
                output->insert_wme_token(other.first, shared());
            }
          }
        }
      }
    }

    bool operator==(const Rete_Node &rhs) const {
      if(auto join = dynamic_cast<const Rete_Negation_Join *>(&rhs))
        return bindings == join->bindings && input0.lock() == join->input0.lock() && input1.lock() == join->input1.lock();
      return false;
    }

    static Rete_Negation_Join_Ptr find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
      for(auto &o0 : out0->get_outputs()) {
        if(auto existing_join = std::dynamic_pointer_cast<Rete_Negation_Join>(o0)) {
          if(std::find(out1->get_outputs().begin(), out1->get_outputs().end(), o0) != out1->get_outputs().end()) {
            if(bindings == existing_join->bindings)
              return existing_join;
          }
        }
      }

      return nullptr;
    }

  private:
    void join_tokens(const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs) {
      for(auto &binding : bindings) {
        if(*(*lhs)[binding.first] != *(*rhs)[binding.second])
          return;
      }

      if(++find_key(input0_tokens, lhs)->second == 1) {
        for(auto &output : outputs)
          output->remove_wme_token(lhs, shared());
        output_tokens.erase(find(output_tokens, lhs));
      }
    }

    void pass_tokens(const Rete_Node_Ptr &output) {
      for(auto &wme_token : output_tokens)
        output->insert_wme_token(wme_token, shared());
    }

    WME_Bindings bindings;
    std::weak_ptr<Rete_Node> input0;
    std::weak_ptr<Rete_Node> input1;
    std::list<std::pair<WME_Token_Ptr_C, size_t>> input0_tokens;
    std::list<WME_Token_Ptr_C> input1_tokens;
    std::list<WME_Token_Ptr_C> output_tokens;
  };

  inline void bind_to_negation_join(const Rete_Negation_Join_Ptr &existential_join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(existential_join && !existential_join->input0.lock() && !existential_join->input1.lock());
    existential_join->input0 = out0;
    existential_join->input1 = out1;

    out0->outputs.push_back(existential_join);
    out1->outputs.push_back(existential_join);
    out0->pass_tokens(existential_join);
    out1->pass_tokens(existential_join);
  }

}

#endif
