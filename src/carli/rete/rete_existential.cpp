#include "rete_existential.h"

namespace Rete {

  Rete_Existential::Rete_Existential() : output_token(std::make_shared<WME_Token>()) {}

  void Rete_Existential::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs_all.empty()) {
      //std::cerr << "Destroying: ";
      //output_name(std::cerr, 3);
      //std::cerr << std::endl;

      input->destroy(filters, shared());
    }
  }

  void Rete_Existential::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                    from
#endif
                                                                                        ) {
    assert(from == input);

    input_tokens.push_back(wme_token);

    if(input_tokens.size() == 1) {
      for(auto &output : outputs_enabled)
        output->insert_wme_token(output_token, this);
    }

    std::cerr << "input_tokens.size() == " << input_tokens.size() << std::endl;
  }

  bool Rete_Existential::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                 from
#endif
                                                                                     ) {
    assert(from == input);

    auto found = find(input_tokens, wme_token);
    if(found != input_tokens.end()) {
      input_tokens.erase(found);
      if(input_tokens.empty()) {
        for(auto ot = outputs_enabled.begin(), oend = outputs_enabled.end(); ot != oend; ) {
          if((*ot)->remove_wme_token(output_token, this))
            (*ot++)->disconnect(this);
          else
            ++ot;
        }
      }
    }

    std::cerr << "input_tokens.size() == " << input_tokens.size() << std::endl;

    return input_tokens.empty();
  }

  void Rete_Existential::pass_tokens(Rete_Node * const &output) {
    if(!input_tokens.empty())
      output->insert_wme_token(output_token, this);
  }

  void Rete_Existential::unpass_tokens(Rete_Node * const &output) {
    if(!input_tokens.empty())
      output->remove_wme_token(output_token, this);
  }

  bool Rete_Existential::operator==(const Rete_Node &rhs) const {
    if(auto existential = dynamic_cast<const Rete_Existential *>(&rhs))
      return input == existential->input;
    return false;
  }

  void Rete_Existential::print_details(std::ostream &os) const {
    os << "  " << intptr_t(this) << " [label=\"&exist;\"];" << std::endl;
    os << "  " << intptr_t(input) << " -> " << intptr_t(this) << " [color=red];" << std::endl;
  }

  void Rete_Existential::output_name(std::ostream &os, const int64_t &depth) const {
    os << "e(";
    if(input && depth)
      input->output_name(os, depth - 1);
    os << ')';
  }

  bool Rete_Existential::is_active() const {
    return !input_tokens.empty();
  }

  Rete_Existential_Ptr Rete_Existential::find_existing(const Rete_Node_Ptr &out) {
    for(auto &o : out->get_outputs_all()) {
      if(auto existing_existential = std::dynamic_pointer_cast<Rete_Existential>(o))
        return existing_existential;
    }

    return nullptr;
  }

  void bind_to_existential(const Rete_Existential_Ptr &existential, const Rete_Node_Ptr &out) {
    assert(existential);
    existential->input = out.get();

    out->insert_output(existential);
    out->pass_tokens(existential.get());
  }

}
