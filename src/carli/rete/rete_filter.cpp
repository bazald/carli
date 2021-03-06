#include "rete_filter.h"

#include "rete_action.h"
#include "rete_agent.h"

namespace Rete {

  Rete_Filter::Rete_Filter(const WME &wme_)
    : m_wme(wme_)
  {
    for(int i = 0; i != 3; ++i)
      m_variable[i] = std::dynamic_pointer_cast<const Symbol_Variable>(m_wme.symbols[i]);
  }

  const WME & Rete_Filter::get_wme() const {
    return m_wme;
  }

  void Rete_Filter::destroy(Rete_Agent &agent, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(!destruction_suppressed && outputs_all.empty())
      agent.excise_filter(std::static_pointer_cast<Rete_Filter>(shared()));
  }

  Rete_Filter_Ptr_C Rete_Filter::get_filter(const int64_t &
#ifndef NDEBUG
                                                           index
#endif
                                                                ) const {
    assert(index == 0);
    return std::dynamic_pointer_cast<const Rete_Filter>(shared_from_this());
  }

  const Rete_Node::Tokens & Rete_Filter::get_output_tokens() const {
    return tokens;
  }

  bool Rete_Filter::has_output_tokens() const {
    return !tokens.empty();
  }

  void Rete_Filter::insert_wme(Rete_Agent &agent, const WME_Ptr_C &wme) {
    for(int i = 0; i != 3; ++i)
      if(!m_variable[i] && *m_wme.symbols[i] != *wme->symbols[i])
        return;

    if(m_variable[0] && m_variable[1] && *m_variable[0] == *m_variable[1] && *wme->symbols[0] != *wme->symbols[1])
      return;
    if(m_variable[0] && m_variable[2] && *m_variable[0] == *m_variable[2] && *wme->symbols[0] != *wme->symbols[2])
      return;
    if(m_variable[1] && m_variable[2] && *m_variable[1] == *m_variable[2] && *wme->symbols[1] != *wme->symbols[2])
      return;

    const auto inserted = tokens.insert(std::make_shared<WME_Token>(wme));
    if(inserted.second) {
      for(auto &output : *outputs_enabled)
        output.ptr->insert_wme_token(agent, *inserted.first, this);
    }
  }

  void Rete_Filter::remove_wme(Rete_Agent &agent, const WME_Ptr_C &wme) {
    auto found = tokens.find(std::make_shared<WME_Token>(wme));
    if(found != tokens.end()) {
      for(auto ot = outputs_enabled->begin(), oend = outputs_enabled->end(); ot != oend; ) {
        if((*ot)->remove_wme_token(agent, *found, this))
          (*ot++)->disconnect(agent, this);
        else
          ++ot;
      }
      tokens.erase(found);
    }
  }

  void Rete_Filter::insert_wme_token(Rete_Agent &, const WME_Token_Ptr_C &, const Rete_Node * const &) {
    abort();
  }

  bool Rete_Filter::remove_wme_token(Rete_Agent &, const WME_Token_Ptr_C &, const Rete_Node * const &) {
    abort();
  }

  void Rete_Filter::pass_tokens(Rete_Agent &agent, Rete_Node * const &output) {
    for(auto &token : tokens)
      output->insert_wme_token(agent, token, this);
  }

  void Rete_Filter::unpass_tokens(Rete_Agent &agent, Rete_Node * const &output) {
    for(auto &token : tokens)
      output->remove_wme_token(agent, token, this);
  }

  bool Rete_Filter::operator==(const Rete_Node &rhs) const {
    if(auto filter = dynamic_cast<const Rete_Filter *>(&rhs)) {
      for(int i = 0; i != 3; ++i) {
        if((m_variable[i] != nullptr) ^ (filter->m_variable[i] != nullptr))
          return false;
        if(!m_variable[i] && *m_wme.symbols[i] != *filter->m_wme.symbols[i])
          return false;
      }

      if(m_variable[0] && m_variable[1] && ((*m_variable[0] == *m_variable[1]) ^ (*filter->m_variable[0] == *filter->m_variable[1])))
        return false;
      if(m_variable[0] && m_variable[2] && ((*m_variable[0] == *m_variable[2]) ^ (*filter->m_variable[0] == *filter->m_variable[2])))
        return false;
      if(m_variable[1] && m_variable[2] && ((*m_variable[1] == *m_variable[2]) ^ (*filter->m_variable[1] == *filter->m_variable[2])))
        return false;

      return true;
    }
    return false;
  }

  void Rete_Filter::print_details(std::ostream &os) const {
    os << "  " << intptr_t(this) << " [label=\"F" << m_wme << "\"];" << std::endl;
  }

  void Rete_Filter::print_rule(std::ostream &os, const Variable_Indices_Ptr_C &indices, const Rete_Node_Ptr_C &suppress) const {
    if(suppress && this == suppress->parent_left().get()) {
      os << '&' << dynamic_cast<const Rete_Action *>(suppress.get())->get_name();
      return;
    }

    m_wme.print(os, indices);
  }

  void Rete_Filter::output_name(std::ostream &os, const int64_t &) const {
    os << 'f' << m_wme;
  }

  bool Rete_Filter::is_active() const {
    return !tokens.empty();
  }

  std::vector<WME> Rete_Filter::get_filter_wmes() const {
    return std::vector<WME>(1, m_wme);
  }

  void bind_to_filter(Rete_Agent &/*agent*/, const Rete_Filter_Ptr &filter) {
    assert(filter);
    filter->height = 1;
    filter->token_owner = filter;
    filter->size = 1;
    filter->token_size = 1;
  }

}
