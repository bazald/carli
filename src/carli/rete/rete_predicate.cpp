#include "rete_predicate.h"

#include "rete_existential.h"
#include "rete_negation.h"

namespace Rete {

  Rete_Predicate::Rete_Predicate(const Predicate &predicate_, const WME_Token_Index lhs_index_, const WME_Token_Index rhs_index_)
   : m_predicate(predicate_),
   m_lhs_index(lhs_index_),
   m_rhs_index(rhs_index_)
  {
  }

  Rete_Predicate::Rete_Predicate(const Predicate &predicate_, const WME_Token_Index lhs_index_, const Symbol_Ptr_C &rhs_)
   : m_predicate(predicate_),
   m_lhs_index(lhs_index_),
   m_rhs(rhs_)
  {
  }

  void Rete_Predicate::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs.empty())
      input->destroy(filters, shared());
  }

  void Rete_Predicate::insert_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                 from
#endif
                                                                                     ) {
    assert(from == input);

    if(m_rhs) {
      if(!test_predicate((*wme_token)[m_lhs_index], m_rhs))
        return;
    }
    else {
      if(!test_predicate((*wme_token)[m_lhs_index], (*wme_token)[m_rhs_index]))
        return;
    }

    tokens.push_back(wme_token);

    for(auto &output : outputs)
      output->insert_wme_token(wme_token, this);
  }

  void Rete_Predicate::remove_wme_token(const WME_Token_Ptr_C &wme_token, const Rete_Node * const &
#ifndef NDEBUG
                                                                                 from
#endif
                                                                                     ) {
    assert(from == input);

    auto found = find(tokens, wme_token);
    if(found != tokens.end()) {
      tokens.erase(found);
      for(auto &output : outputs)
        output->remove_wme_token(wme_token, this);
    }
  }

  void Rete_Predicate::pass_tokens(const Rete_Node_Ptr &output) {
    if(is_iterating())
      return;
    for(auto &wme_token : tokens)
      output->insert_wme_token(wme_token, this);
  }

  bool Rete_Predicate::operator==(const Rete_Node &rhs) const {
    if(auto predicate = dynamic_cast<const Rete_Predicate *>(&rhs)) {
      return m_predicate == predicate->m_predicate &&
             m_lhs_index == predicate->m_rhs_index &&
             m_rhs_index == predicate->m_rhs_index &&
             *m_rhs == *predicate->m_rhs &&
             input == predicate->input;
    }
    return false;
  }

  Rete_Predicate_Ptr Rete_Predicate::find_existing(const Predicate &predicate, const WME_Token_Index &lhs_index, const WME_Token_Index &rhs_index, const Rete_Node_Ptr &out) {
    for(auto &o : out->get_outputs()) {
      if(auto existing_predicate = std::dynamic_pointer_cast<Rete_Predicate>(o)) {
        if(predicate == existing_predicate->m_predicate &&
           lhs_index == existing_predicate->m_lhs_index &&
           rhs_index == existing_predicate->m_rhs_index)
        {
          return existing_predicate;
        }
      }
    }

    return nullptr;
  }

  Rete_Predicate_Ptr Rete_Predicate::find_existing(const Predicate &predicate, const WME_Token_Index &lhs_index, const Symbol_Ptr_C &rhs, const Rete_Node_Ptr &out) {
    for(auto &o : out->get_outputs()) {
      if(auto existing_predicate = std::dynamic_pointer_cast<Rete_Predicate>(o)) {
        if(predicate == existing_predicate->m_predicate &&
           lhs_index == existing_predicate->m_lhs_index &&
           *rhs == *existing_predicate->m_rhs)
        {
          return existing_predicate;
        }
      }
    }

    return nullptr;
  }

  bool Rete_Predicate::test_predicate(const Symbol_Ptr_C &lhs, const Symbol_Ptr_C &rhs) const {
    switch(m_predicate) {
      case EQ: return *lhs == *rhs;
      case NEQ: return *lhs != *rhs;
      case GT: return *lhs > *rhs;
      case GTE: return *lhs >= *rhs;
      case LT: return *lhs < *rhs;
      case LTE: return *lhs <= *rhs;
      default: abort();
    }
  }

  void bind_to_predicate(const Rete_Predicate_Ptr &predicate, const Rete_Node_Ptr &out) {
    assert(predicate);
    assert(!std::dynamic_pointer_cast<Rete_Existential>(out));
    assert(!std::dynamic_pointer_cast<Rete_Negation>(out));
    predicate->input = out.get();

    out->insert_output(predicate);
    out->pass_tokens(predicate);
  }

}
