#include "rete_join.h"

#include "rete_existential.h"
#include "rete_negation.h"

namespace Rete {

  Rete_Join::Rete_Join(WME_Bindings bindings_) : bindings(bindings_) {}

  void Rete_Join::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs_all.empty()) {
      //std::cerr << "Destroying: ";
      //output_name(std::cerr, 3);
      //std::cerr << std::endl;

      auto i0 = input0;
      auto i1 = input1;
      auto o = shared();
      i0->destroy(filters, o);
      if(i0 != i1)
        i1->destroy(filters, o);
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
        input1->enable_output(this);
        data.connected1 = true;
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
        input0->enable_output(this);
        data.connected0 = true;
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
            for(auto ot = outputs_all.begin(), oend = outputs_all.end(); ot != oend; ) {
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
            for(auto ot = outputs_all.begin(), oend = outputs_all.end(); ot != oend; ) {
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
  
  bool Rete_Join::disabled_input(const Rete_Node_Ptr &input) {
    if(input.get() == input0)
      return !data.connected0;
    else {
      assert(input.get() == input1);
      return !data.connected1;
    }
  }

  void Rete_Join::print_details(std::ostream &os) const {
    os << "  " << intptr_t(this) << " [label=\"";
    if(bindings.empty())
      os << "&empty;";
    else {
      auto bt = bindings.begin();
      os << *bt++;
      while(bt != bindings.end())
        os << "\\n" << *bt++;
    }
    os << "\"];" << std::endl;

    os << "  " << intptr_t(input0) << " -> " << intptr_t(this) << " [color=red];" << std::endl;
    os << "  " << intptr_t(input1) << " -> " << intptr_t(this) << " [color=blue];" << std::endl;
  }

  void Rete_Join::output_name(std::ostream &os, const int64_t &depth) const {
    os << "j(" << bindings << ',';
    if(input0 && depth)
      input0->output_name(os, depth - 1);
    os << ',';
    if(input1 && depth)
      input1->output_name(os, depth - 1);
    os << ')';
  }
  
  bool Rete_Join::is_active() const {
    return !output_tokens.empty();
  }
  
  Rete_Join_Ptr Rete_Join::find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    for(auto &o0 : out0->get_outputs_all()) {
      if(auto existing_join = std::dynamic_pointer_cast<Rete_Join>(o0)) {
        if(std::find(out1->get_outputs_all().begin(), out1->get_outputs_all().end(), existing_join) != out1->get_outputs_all().end()) {
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

//#ifdef DEBUG_OUTPUT
//    std::cerr << "Joining " << *lhs << " and " << *rhs << std::endl;
//#endif

    assert(find_deref(output_tokens, join_wme_tokens(lhs, rhs)) == output_tokens.end());

    output_tokens.push_back(join_wme_tokens(lhs, rhs));
    for(auto &output : *outputs_enabled)
      output.ptr->insert_wme_token(output_tokens.back(), this);
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
      input1->disable_output(this);
      data.connected1 = false;
    }
    else {
//#ifdef DEBUG_OUTPUT
//      std::cerr << this << " Disconnecting left" << std::endl;
//#endif
      assert(data.connected0);
      input0->disable_output(this);
      data.connected0 = false;
    }
    assert(data.connected0 || data.connected1);
  }

  void Rete_Join::pass_tokens(Rete_Node * const &output) {
    for(auto &wme_token : output_tokens)
      output->insert_wme_token(wme_token, this);
  }

  void Rete_Join::unpass_tokens(Rete_Node * const &output) {
    for(auto &wme_token : output_tokens)
      output->remove_wme_token(wme_token, this);
  }

  void bind_to_join(const Rete_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(join && !join->input0 && !join->input1);
    assert(!std::dynamic_pointer_cast<Rete_Existential>(out0));
    assert(!std::dynamic_pointer_cast<Rete_Negation>(out0));
    join->input0 = out0.get();
    join->input1 = out1.get();

    out0->insert_output_enabled(join);
    if(out0 != out1)
      out1->insert_output_disabled(join);
    else
      join->data.connected1 = true;

    out0->pass_tokens(join.get());
  }

}
