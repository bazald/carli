#include "rete_existential_join.h"

#include "rete_existential.h"
#include "rete_negation.h"

namespace Rete {

  Rete_Existential_Join::Rete_Existential_Join(WME_Bindings bindings_) : bindings(bindings_) {}

  void Rete_Existential_Join::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs.empty()) {
      auto i0 = input0.lock();
      auto i1 = input1.lock();
      i0->destroy(filters, shared());
      if(i0 != i1)
        i1->destroy(filters, shared());
    }
  }

  void Rete_Existential_Join::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from) {
    assert(from == input0.lock() || from == input1.lock());

    if(from == input0.lock()) {
      input0_tokens.push_back(wme_token);

      for(const auto &other : input1_tokens)
        join_tokens(wme_token, other);
    }
    if(from == input1.lock()) {
      input1_tokens.push_back(wme_token);

      for(const auto &other : input0_tokens)
        join_tokens(other, wme_token);
    }
  }

  void Rete_Existential_Join::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from) {
    assert(from == input0.lock() || from == input1.lock());

    if(from == input0.lock()) {
      auto found = find(input0_tokens, wme_token);
      if(found != input0_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input0_tokens.erase(found);
        for(const auto &other : input1_tokens)
          unjoin_tokens(wme_token, other);
      }
    }
    if(from == input1.lock()) {
      auto found = find(input1_tokens, wme_token);
      if(found != input1_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input1_tokens.erase(found);
        for(const auto &other : input0_tokens)
          unjoin_tokens(other, wme_token);
      }
    }
  }

  bool Rete_Existential_Join::operator==(const Rete_Node &rhs) const {
    if(auto join = dynamic_cast<const Rete_Existential_Join *>(&rhs))
      return bindings == join->bindings && input0.lock() == join->input0.lock() && input1.lock() == join->input1.lock();
    return false;
  }

  Rete_Existential_Join_Ptr Rete_Existential_Join::find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    for(auto &o0 : out0->get_outputs()) {
      if(auto existing_existential_join = std::dynamic_pointer_cast<Rete_Existential_Join>(o0)) {
        if(std::find(out1->get_outputs().begin(), out1->get_outputs().end(), existing_existential_join) != out1->get_outputs().end()) {
          if(bindings == existing_existential_join->bindings)
            return existing_existential_join;
        }
      }
    }

    return nullptr;
  }

  void Rete_Existential_Join::join_tokens(const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs) {
    for(auto &binding : bindings) {
      if(*(*lhs)[binding.first] != *(*rhs)[binding.second])
        return;
    }

    auto found = find_key(output_tokens, lhs);
    if(found == output_tokens.end()) {
      output_tokens.push_back(std::make_pair(lhs, 1u));

      for(outputs_iterator = outputs.begin(); outputs_iterator != outputs.end(); )
        (*outputs_iterator++)->insert_wme_token(lhs, shared());
    }
    else
      ++found->second;
  }

  void Rete_Existential_Join::unjoin_tokens(const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs) {
    for(auto &binding : bindings) {
      if(*(*lhs)[binding.first] != *(*rhs)[binding.second])
        return;
    }

    auto found = find_key(output_tokens, lhs);
    assert(found != output_tokens.end());
    if(--found->second == 0) {
      for(outputs_iterator = outputs.begin(); outputs_iterator != outputs.end(); )
        (*outputs_iterator++)->remove_wme_token(lhs, shared());

      output_tokens.erase(found);
    }
  }

  void Rete_Existential_Join::pass_tokens(const Rete_Node_Ptr &output) {
    if(is_iterating())
      return;
    for(auto &wme_token : output_tokens)
      output->insert_wme_token(wme_token.first, shared());
  }

  void bind_to_existential_join(const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(join && !join->input0.lock() && !join->input1.lock());
    assert(!std::dynamic_pointer_cast<Rete_Existential>(out0));
    assert(!std::dynamic_pointer_cast<Rete_Negation>(out0));
    join->input0 = out0;
    join->input1 = out1;

    out0->insert_output(join);
    out0->pass_tokens(join);
    if(out0 != out1) {
      out1->insert_output(join);
      out1->pass_tokens(join);
    }
  }

}
