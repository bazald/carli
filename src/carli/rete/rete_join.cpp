#include "rete_join.h"

#include "rete_existential.h"
#include "rete_negation.h"

#include <sstream>

namespace Rete {

  Rete_Join::Rete_Join(WME_Bindings bindings_) : bindings(bindings_) {}

  void Rete_Join::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs.empty() && !outputs_disabled) {
      auto i0 = input0;
      auto i1 = input1;
      i0->destroy(filters, shared());
      if(i0 != i1)
        i1->destroy(filters, shared());
    }
  }

  void Rete_Join::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) {
    assert(from == input0 || from == input1);

    if(from == input0 && find(input0_tokens, wme_token) == input0_tokens.end()) {
//#ifdef DEBUG_OUTPUT
//      std::cerr << this << " Joining left: " << *wme_token << std::endl;
//#endif
      if(!data.connected1) {
//#ifdef DEBUG_OUTPUT
//        std::cerr << this << " Connecting right" << std::endl;
//#endif
        assert(input1_tokens.empty());
        data.connected1 = true;
        input1->enable_output(shared());
      }

//      assert(find(input0_tokens, wme_token) == input0_tokens.end());
      input0_tokens.push_back(wme_token);

      for(const auto &other : input1_tokens)
        join_tokens(wme_token, other);
    }
    if(from == input1 && find(input1_tokens, wme_token) == input1_tokens.end()) {
//#ifdef DEBUG_OUTPUT
//      std::cerr << this << " Joining right: " << *wme_token << std::endl;
//#endif
      if(!data.connected0) {
//#ifdef DEBUG_OUTPUT
//        std::cerr << this << " Connecting left" << std::endl;
//#endif
        assert(input0_tokens.empty());
        data.connected0 = true;
        input0->enable_output(shared());
      }

//      assert(find(input1_tokens, wme_token) == input1_tokens.end());
      input1_tokens.push_back(wme_token);

      for(const auto &other : input0_tokens)
        join_tokens(other, wme_token);
    }
  }

  bool Rete_Join::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) {
    assert(from == input0 || from == input1);

    bool emptied = false;

    if(from == input0) {
      auto found = find(input0_tokens, wme_token);
      if(found != input0_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input0_tokens.erase(found);
        for(const auto &other : input1_tokens) {
          auto found_output = find_deref(output_tokens, join_wme_tokens(wme_token, other));
          if(found_output != output_tokens.end()) {
            for(auto ot = outputs.begin(), oend = outputs.end(); ot != oend; ) {
              if((*ot)->remove_wme_token(*found_output, this))
                (*ot++)->disconnect(this);
              else
                ++ot;
            }
            output_tokens.erase(found_output);
          }
        }

        emptied ^= input0_tokens.empty();
      }
    }
    if(from == input1) {
      auto found = find(input1_tokens, wme_token);
      if(found != input1_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input1_tokens.erase(found);
        for(const auto &other : input0_tokens) {
          auto found_output = find_deref(output_tokens, join_wme_tokens(other, wme_token));
          if(found_output != output_tokens.end()) {
            for(auto ot = outputs.begin(), oend = outputs.end(); ot != oend; ) {
              if((*ot)->remove_wme_token(*found_output, this))
                (*ot++)->disconnect(this);
              else
                ++ot;
            }
            output_tokens.erase(found_output);
          }
        }

        emptied ^= input1_tokens.empty();
      }
    }

    return emptied;
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

  std::string Rete_Join::generate_name() const {
    std::ostringstream oss;
    oss << "j(" << bindings << ',';
    if(input0)
      oss << input0->generate_name();
    oss << ',';
    if(input1)
      oss << input1->generate_name();
    oss << ')';
    return oss.str();
  }

  void Rete_Join::join_tokens(const WME_Token_Ptr_C &lhs, const WME_Token_Ptr_C &rhs) {
    for(auto &binding : bindings) {
      if(*(*lhs)[binding.first] != *(*rhs)[binding.second])
        return;
    }

//#ifdef DEBUG_OUTPUT
//    std::cerr << "Joining " << *lhs << " and " << *rhs << std::endl;
//#endif

    assert(find_deref(output_tokens, join_wme_tokens(lhs, rhs)) == output_tokens.end());

    output_tokens.push_back(join_wme_tokens(lhs, rhs));
    for(auto &output : outputs)
      output->insert_wme_token(output_tokens.back(), this);
  }

  WME_Token_Ptr_C Rete_Join::join_wme_tokens(const WME_Token_Ptr_C lhs, const WME_Token_Ptr_C &rhs) {
    if(rhs->size())
      return std::make_shared<WME_Token>(lhs, rhs);
    else
      return lhs;
  }

  void Rete_Join::disconnect(const Rete_Node * const &from) {
    assert(input0 != input1);
    if(from == input0) {
//#ifdef DEBUG_OUTPUT
//      std::cerr << this << " Disconnecting right" << std::endl;
//#endif
      assert(data.connected1);
      data.connected1 = false;
      input1->disable_output(shared());
    }
    else {
//#ifdef DEBUG_OUTPUT
//      std::cerr << this << " Disconnecting left" << std::endl;
//#endif
      assert(data.connected0);
      data.connected0 = false;
      input0->disable_output(shared());
    }
    assert(data.connected0 || data.connected1);
  }

  void Rete_Join::pass_tokens(const Rete_Node_Ptr &output) {
    for(auto &wme_token : output_tokens)
      output->insert_wme_token(wme_token, this);
  }

  void Rete_Join::unpass_tokens(const Rete_Node_Ptr &output) {
    for(auto &wme_token : output_tokens)
      output->remove_wme_token(wme_token, this);
  }

  void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(join && !join->input0 && !join->input1);
    assert(!std::dynamic_pointer_cast<Rete_Existential>(out0));
    assert(!std::dynamic_pointer_cast<Rete_Negation>(out0));
    join->input0 = out0.get();
    join->input1 = out1.get();

    out0->insert_output(join);
    out0->pass_tokens(join);
    if(out0 != out1)
      ++out1->outputs_disabled;
    else
      join->data.connected1 = true;

  }

}
