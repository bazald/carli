#include "rete_filter.h"

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

  void Rete_Filter::destroy(Filters &filters, const Rete_Node_Ptr &output) {
    erase_output(output);
    if(outputs.empty())
      filters.erase(std::find(filters.begin(), filters.end(), std::static_pointer_cast<Rete_Filter>(shared())));
  }

  void Rete_Filter::insert_wme(const WME_Ptr_C &wme) {
    for(int i = 0; i != 3; ++i)
      if(!m_variable[i] && *m_wme.symbols[i] != *wme->symbols[i])
        return;

    if(m_variable[0] && m_variable[1] && *m_variable[0] == *m_variable[1] && *wme->symbols[0] != *wme->symbols[1])
      return;
    if(m_variable[0] && m_variable[2] && *m_variable[0] == *m_variable[2] && *wme->symbols[0] != *wme->symbols[2])
      return;
    if(m_variable[1] && m_variable[2] && *m_variable[1] == *m_variable[2] && *wme->symbols[1] != *wme->symbols[2])
      return;

    if(find_key(tokens, wme) == tokens.end()) {
      auto wme_token = std::make_pair(wme, std::make_shared<WME_Token>(wme));
      tokens.push_back(wme_token);
      for(auto &output : outputs)
        output->insert_wme_token(wme_token.second, shared());
    }
  }

  void Rete_Filter::remove_wme(const WME_Ptr_C &wme) {
    auto found = find_key(tokens, wme);
    if(found != tokens.end()) {
      for(auto &output : outputs)
        output->remove_wme_token(found->second, shared());
      tokens.erase(found);
    }
  }

  void Rete_Filter::insert_wme_token(const WME_Token_Ptr_C &, const Rete_Node_Ptr_C &) {
    abort();
  }

  void Rete_Filter::remove_wme_token(const WME_Token_Ptr_C &, const Rete_Node_Ptr_C &) {
    abort();
  }

  void Rete_Filter::pass_tokens(const Rete_Node_Ptr &output) {
    for(auto &token : tokens)
      output->insert_wme_token(token.second, shared());
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

}
