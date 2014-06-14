#include "rete_existential_join.h"

#include "rete_existential.h"
#include "rete_negation.h"

namespace Rete {

  Rete_Existential_Join::Rete_Existential_Join(WME_Bindings bindings_) : bindings(bindings_) {}

  void Rete_Existential_Join::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs_all.empty()) {
      //std::cerr << "Destroying: ";
      //output_name(std::cerr, 3);
      //std::cerr << std::endl;

      auto i0 = input0->shared();
      auto i1 = input1->shared();
      auto o = shared();
      i0->destroy(filters, o);
      if(i0 != i1)
        i1->destroy(filters, o);
    }
  }

  void Rete_Existential_Join::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) {
    assert(from == input0 || from == input1);

    if(from == input0 && find_key(input0_tokens, wme_token) == input0_tokens.end()) {
      if(!data.connected1) {
        input1->enable_output(shared());
        data.connected1 = true;
      }

//      assert(find_key(input0_tokens, wme_token) == input0_tokens.end());
      input0_tokens.emplace_back(wme_token, 0u);

      for(const auto &other : input1_tokens)
        join_tokens(input0_tokens.back(), other);
    }
    if(from == input1 && find(input1_tokens, wme_token) == input1_tokens.end()) {
      if(!data.connected0) {
        input0->enable_output(shared());
        data.connected0 = true;
      }

//      assert(find(input1_tokens, wme_token) == input1_tokens.end());
      input1_tokens.push_back(wme_token);

      for(auto &other : input0_tokens)
        join_tokens(other, wme_token);
    }
  }

  bool Rete_Existential_Join::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &from) {
    assert(from == input0 || from == input1);

    bool emptied = false;

    if(from == input0) {
      auto found = find_key(input0_tokens, wme_token);
      if(found != input0_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        for(const auto &other : input1_tokens)
          unjoin_tokens(*found, other);
        input0_tokens.erase(found);

        emptied ^= input0_tokens.empty();
      }
    }
    if(from == input1) {
      auto found = find(input1_tokens, wme_token);
      if(found != input1_tokens.end()) {
        // TODO: Avoid looping through non-existent pairs?
        input1_tokens.erase(found);
        for(auto &other : input0_tokens)
          unjoin_tokens(other, wme_token);

        emptied ^= input1_tokens.empty();
      }
    }

    return emptied;
  }

  bool Rete_Existential_Join::operator==(const Rete_Node &rhs) const {
    if(auto join = dynamic_cast<const Rete_Existential_Join *>(&rhs))
      return bindings == join->bindings && input0 == join->input0 && input1 == join->input1;
    return false;
  }

  bool Rete_Existential_Join::disabled_input(const Rete_Node_Ptr &input) {
    if(input.get() == input0)
      return !data.connected0;
    else {
      assert(input.get() == input1);
      return !data.connected1;
    }
  }

  void Rete_Existential_Join::print_details(std::ostream &os) const {
    os << "  " << intptr_t(this) << " [label=\"&exist;J\\n";
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

  void Rete_Existential_Join::output_name(std::ostream &os, const int64_t &depth) const {
    os << "ej(" << bindings << ',';
    if(input0 && depth)
      input0->output_name(os, depth - 1);
    os << ',';
    if(input1 && depth)
      input1->output_name(os, depth - 1);
    os << ')';
  }
  
  bool Rete_Existential_Join::is_active() const {
    for(auto &wme_token : input0_tokens)
      if(wme_token.second)
        return true;
    return false;
  }
  
  Rete_Existential_Join_Ptr Rete_Existential_Join::find_existing(const WME_Bindings &bindings, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    for(auto &o0 : out0->get_outputs_all()) {
      if(auto existing_existential_join = std::dynamic_pointer_cast<Rete_Existential_Join>(o0)) {
        if(std::find(out1->get_outputs_all().begin(), out1->get_outputs_all().end(), existing_existential_join) != out1->get_outputs_all().end()) {
          if(bindings == existing_existential_join->bindings)
            return existing_existential_join;
        }
      }
    }

    return nullptr;
  }

  void Rete_Existential_Join::join_tokens(std::pair<WME_Token_Ptr_C, size_t> &lhs, const WME_Token_Ptr_C &rhs) {
    for(auto &binding : bindings) {
      if(*(*lhs.first)[binding.first] != *(*rhs)[binding.second])
        return;
    }

    if(++lhs.second == 1u) {
      for(auto &output : outputs_enabled)
        output->insert_wme_token(lhs.first, this);
    }
  }

  void Rete_Existential_Join::unjoin_tokens(std::pair<WME_Token_Ptr_C, size_t> &lhs, const WME_Token_Ptr_C &rhs) {
    for(auto &binding : bindings) {
      if(*(*lhs.first)[binding.first] != *(*rhs)[binding.second])
        return;
    }

    if(--lhs.second == 0) {
      for(auto ot = outputs_enabled.begin(), oend = outputs_enabled.end(); ot != oend; ) {
        if((*ot)->remove_wme_token(lhs.first, this))
          (*ot++)->disconnect(this);
        else
          ++ot;
      }
    }
  }

  void Rete_Existential_Join::pass_tokens(const Rete_Node_Ptr &output) {
    for(auto &wme_token : input0_tokens) {
      if(wme_token.second)
        output->insert_wme_token(wme_token.first, this);
    }
  }

  void Rete_Existential_Join::unpass_tokens(const Rete_Node_Ptr &output) {
    for(auto &wme_token : input0_tokens) {
      if(wme_token.second)
        output->remove_wme_token(wme_token.first, this);
    }
  }

  void Rete_Existential_Join::disconnect(const Rete_Node * const &from) {
    if(input0 != input1) {
      if(from == input0) {
        assert(data.connected1);
        input1->disable_output(shared());
        data.connected1 = false;
      }
      else {
        assert(data.connected0);
        input0->disable_output(shared());
        data.connected0 = false;
      }
    }
    assert(data.connected0 || data.connected1);
  }

  void bind_to_existential_join(const Rete_Existential_Join_Ptr &join, const Rete_Node_Ptr &out0, const Rete_Node_Ptr &out1) {
    assert(join && !join->input0 && !join->input1);
    assert(!std::dynamic_pointer_cast<Rete_Existential>(out0));
    assert(!std::dynamic_pointer_cast<Rete_Negation>(out0));
    join->input0 = out0.get();
    join->input1 = out1.get();

    out0->insert_output(join);
    if(out0 != out1)
      out1->insert_output_disabled(join);
    else
      join->data.connected1 = true;

    out0->pass_tokens(join);
  }

}
