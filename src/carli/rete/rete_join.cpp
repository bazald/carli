#include "rete_join.h"

#include "rete_existential.h"
#include "rete_negation.h"

namespace Rete {

  Rete_Join::Rete_Join(WME_Bindings bindings_) : bindings(bindings_) {}

  void Rete_Join::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs.empty()) {
      auto i0 = input0;
      auto i1 = input1;
      i0->destroy(filters, shared());
      if(i0 != i1)
        i1->destroy(filters, shared());
    }
  }

  void Rete_Join::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from) {
    assert(from.get() == input0 || from.get() == input1);

    if(from.get() == input0) {
      input0_tokens.push_back(wme_token);

      for(const auto &other : input1_tokens)
        join_tokens(wme_token, other);
    }
    if(from.get() == input1) {
      input1_tokens.push_back(wme_token);

      for(const auto &other : input0_tokens)
        join_tokens(other, wme_token);
    }
  }

  void Rete_Join::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node_Ptr_C &from) {
    assert(from.get() == input0 || from.get() == input1);

    if(from.get() == input0) {
      auto found = find(input0_tokens, wme_token);
      if(found != input0_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input0_tokens.erase(found);
        for(const auto &other : input1_tokens) {
          auto found_output = find_deref(output_tokens, join_wme_tokens(wme_token, other));
          if(found_output != output_tokens.end()) {
            for(auto &output : outputs)
              output->remove_wme_token(*found_output, shared());
            output_tokens.erase(found_output);
          }
        }
      }
    }
    if(from.get() == input1) {
      auto found = find(input1_tokens, wme_token);
      if(found != input1_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input1_tokens.erase(found);
        for(const auto &other : input0_tokens) {
          auto found_output = find_deref(output_tokens, join_wme_tokens(other, wme_token));
          if(found_output != output_tokens.end()) {
            for(auto &output : outputs)
              output->remove_wme_token(*found_output, shared());
            output_tokens.erase(found_output);
          }
        }
      }
    }
  }

  bool Rete_Join::operator==(const Rete_Node &rhs) const {
    if(auto join = dynamic_cast<const Rete_Join *>(&rhs))
      return bindings == join->bindings && input0 == join->input0 && input1 == join->input1;
    return false;
  }

  Rete_Join_Ptr Rete_Join::find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    for(auto &o0 : out0->get_outputs()) {
      if(auto existing_join = std::dynamic_pointer_cast<Rete_Join>(o0)) {
        if(std::find(out1->get_outputs().begin(), out1->get_outputs().end(), existing_join) != out1->get_outputs().end()) {
          if(bindings == existing_join->bindings)
            return existing_join;
        }
      }
    }

    return nullptr;
  }

  void Rete_Join::join_tokens(const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs) {
    for(auto &binding : bindings) {
      if(*(*lhs)[binding.first] != *(*rhs)[binding.second])
        return;
    }

    output_tokens.push_back(join_wme_tokens(lhs, rhs));
    for(auto &output : outputs)
      output->insert_wme_token(output_tokens.back(), shared());
  }

  WME_Token_Ptr_C Rete_Join::join_wme_tokens(const WME_Token_Ptr_C lhs, const WME_Token_Ptr_C &rhs) {
    if(rhs->size())
      return std::make_shared<WME_Token>(lhs, rhs);
    else
      return lhs;
  }

  void Rete_Join::pass_tokens(const Rete_Node_Ptr &output) {
    if(is_iterating())
      return;
    for(auto &wme_token : output_tokens)
      output->insert_wme_token(wme_token, shared());
  }

  void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(join && !join->input0 && !join->input1);
    assert(!std::dynamic_pointer_cast<Rete_Existential>(out0));
    assert(!std::dynamic_pointer_cast<Rete_Negation>(out0));
    join->input0 = out0.get();
    join->input1 = out1.get();

    out0->insert_output(join);
    out0->pass_tokens(join);
    if(out0 != out1) {
      out1->insert_output(join);
      out1->pass_tokens(join);
    }
  }

}
