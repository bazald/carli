#include "rete_negation.h"

namespace Rete {

  Rete_Negation::Rete_Negation() : output_token(std::make_shared<WME_Token>()) {}

  void Rete_Negation::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs_all.empty()) {
      //std::cerr << "Destroying: ";
      //output_name(std::cerr, 3);
      //std::cerr << std::endl;

      input->destroy(filters, shared());
    }
  }

  void Rete_Negation::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                 from
#endif
                                                                                     ) {
    assert(from == input);

    input_tokens.push_back(wme_token);

    if(input_tokens.size() == 1) {
      for(auto ot = outputs_enabled->begin(), oend = outputs_enabled->end(); ot != oend; ) {
        if((*ot)->remove_wme_token(output_token, this))
          (*ot++)->disconnect(this);
        else
          ++ot;
      }
    }
  }

  bool Rete_Negation::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                 from
#endif
                                                                                     ) {
    assert(from == input);

    auto found = find(input_tokens, wme_token);
    if(found != input_tokens.end()) {
      input_tokens.erase(found);
      if(input_tokens.empty()) {
        for(auto &output : *outputs_enabled)
          output.ptr->insert_wme_token(output_token, this);
      }
    }

    return input_tokens.empty();
  }

  void Rete_Negation::pass_tokens(Rete_Node * const &output) {
    if(input_tokens.empty())
      output->insert_wme_token(output_token, this);
  }

  void Rete_Negation::unpass_tokens(Rete_Node * const &output) {
    if(input_tokens.empty())
      output->remove_wme_token(output_token, this);
  }

  bool Rete_Negation::operator==(const Rete_Node &rhs) const {
    if(auto negation = dynamic_cast<const Rete_Negation *>(&rhs))
      return input == negation->input;
    return false;
  }
  
  void Rete_Negation::print_details(std::ostream &os) const {
    os << "  " << intptr_t(this) << " [label=\"&not;&exist;\"];" << std::endl;
    os << "  " << intptr_t(input) << " -> " << intptr_t(this) << " [color=red];" << std::endl;
  }

  void Rete_Negation::output_name(std::ostream &os, const int64_t &depth) const {
    os << "n(";
    if(input && depth)
      input->output_name(os, depth - 1);
    os << ')';
  }
  
  bool Rete_Negation::is_active() const {
    return input_tokens.empty();
  }
  
  Rete_Negation_Ptr Rete_Negation::find_existing(const Rete_Node_Ptr &out) {
    for(auto &o : out->get_outputs_all()) {
      if(auto existing_negation = std::dynamic_pointer_cast<Rete_Negation>(o))
        return existing_negation;
    }

    return nullptr;
  }

  void bind_to_negation(const Rete_Negation_Ptr &negation, const Rete_Node_Ptr &out) {
    assert(negation);
    negation->input = out.get();

    out->insert_output_enabled(negation);
    out->pass_tokens(negation.get());
  }

}
