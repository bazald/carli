#include "wme_token.h"

namespace Rete {

  WME_Token::WME_Token()
   : m_size(0)
  {
  }

  WME_Token::WME_Token(const WME_Ptr_C &wme)
   : m_size(1),
   m_wme(wme)
  {
  }

  WME_Token::WME_Token(const WME_Token_Ptr_C &first, const WME_Token_Ptr_C &second)
   : m_wme_token(first, second),
   m_size(first->m_size + second->m_size)
  {
    assert(first->m_size);
    assert(second->m_size);
  }

  bool WME_Token::operator==(const WME_Token &rhs) const {
    return m_wme_token == rhs.m_wme_token /*&& m_size == rhs.m_size*/ && m_wme == rhs.m_wme;
  }

  size_t WME_Token::hash() const {
    if(m_wme)
      return std::hash<WME>()(*m_wme);
    else {
      std::hash<WME_Token_Ptr_C> hasher;
      return hash_combine(hasher(m_wme_token.first), hasher(m_wme_token.second));
//        return hash_combine(m_wme_token.first->hash(), m_wme_token.second->hash());
    }
  }

  std::ostream & WME_Token::print(std::ostream &os) const {
    if(m_size == 1) {
      assert(m_wme);
      os << *m_wme;
    }
    else if(m_size > 1){
      assert(m_wme_token.first);
      assert(m_wme_token.second);
      m_wme_token.first->print(os);
      m_wme_token.second->print(os);
    }

    return os;
  }

  const Symbol_Ptr_C & WME_Token::operator[](const WME_Token_Index &index) const {
    if(m_wme) {
      assert(index.first == 0);
      assert(index.second < 3);

      return m_wme->symbols[index.second];
    }
    else {
      assert(index.first < m_size);

      const auto first_size = m_wme_token.first->m_size;
      if(index.first < first_size)
        return (*m_wme_token.first)[index];
      else
        return (*m_wme_token.second)[WME_Token_Index(index.first - first_size, index.second)];
    }
  }

}

std::ostream & operator<<(std::ostream &os, const Rete::WME_Token_Index &index) {
  return os << '(' << index.first << ',' << int(index.second) << ')';
}

std::ostream & operator<<(std::ostream &os, const Rete::WME_Token &wme_token) {
  os << '{';
  wme_token.print(os);
  os << '}';
  return os;
}
