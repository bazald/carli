#ifndef RETE_FILTER_H
#define RETE_FILTER_H

#include "rete_node.h"

namespace Rete {

  class Rete_Filter : public Rete_Node {
    Rete_Filter(const Rete_Filter &);
    Rete_Filter & operator=(const Rete_Filter &);

  public:
    Rete_Filter(const WME &wme_)
      : m_wme(wme_)
    {
      for(int i = 0; i != 3; ++i)
        m_variable[i] = std::dynamic_pointer_cast<const Symbol_Variable>(m_wme.symbols[i]);
    }

    const WME & get_wme() const {
      return m_wme;
    }

    void destroy(std::unordered_set<Rete_Filter_Ptr> &filters, const Rete_Node_Ptr &output) {
      erase_output(output);
      if(outputs.empty())
        filters.erase(std::static_pointer_cast<Rete_Filter>(shared()));
    }

    void insert_wme(const WME_Ptr_C &wme) {
      for(int i = 0; i != 3; ++i)
        if(!m_variable[i] && *m_wme.symbols[i] != *wme->symbols[i])
          return;

      if(m_variable[0] && m_variable[1] && *m_variable[0] == *m_variable[1] && *wme->symbols[0] != *wme->symbols[1])
        return;
      if(m_variable[0] && m_variable[2] && *m_variable[0] == *m_variable[2] && *wme->symbols[0] != *wme->symbols[2])
        return;
      if(m_variable[1] && m_variable[2] && *m_variable[1] == *m_variable[2] && *wme->symbols[1] != *wme->symbols[2])
        return;

      if(tokens.find(wme) == tokens.end()) {
        auto wme_token = tokens[wme] = std::make_shared<WME_Token>(wme);
        for(auto &output : outputs)
          output->insert_wme_token(wme_token, shared());
      }
    }

    void remove_wme(const WME_Ptr_C &wme) {
      auto found = tokens.find(wme);
      if(found != tokens.end()) {
        for(auto &output : outputs)
          output->remove_wme_token(found->second, shared());
        tokens.erase(found);
      }
    }

    void insert_wme_token(const WME_Token_Ptr_C &, const Rete_Node_Ptr_C &) {
      abort();
    }

    void remove_wme_token(const WME_Token_Ptr_C &, const Rete_Node_Ptr_C &) {
      abort();
    }

    void pass_tokens(const Rete_Node_Ptr &output) {
      for(auto &token : tokens)
        output->insert_wme_token(token.second, shared());
    }

    bool operator==(const Rete_Node &rhs) const {
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

  private:
    WME m_wme;
    std::array<Symbol_Variable_Ptr_C, 3> m_variable;
    std::weak_ptr<Rete_Node> input;
    std::unordered_map<WME_Ptr_C, WME_Token_Ptr_C> tokens;
  };

}

#endif
