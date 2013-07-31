#ifndef WME_TOKEN_H
#define WME_TOKEN_H

#include "wme.h"
#include "utility.h"
#include "../memory_pool.h"
#include <cassert>

namespace Rete {

  class WME_Token;
  typedef std::shared_ptr<const WME_Token> WME_Token_Ptr_C;
  typedef std::shared_ptr<WME_Token> WME_Token_Ptr;
  typedef std::pair<size_t, uint8_t> WME_Token_Index;
  typedef std::pair<WME_Token_Index, WME_Token_Index> WME_Binding;
  typedef std::unordered_set<WME_Binding> WME_Bindings;

  class WME_Token : public std::enable_shared_from_this<WME_Token>, public Zeni::Pool_Allocator<WME_Token> {
  public:
    WME_Token()
     : m_size(0)
    {
    }

    WME_Token(const WME_Ptr_C &wme)
     : m_size(1),
     m_wme(wme)
    {
    }

    WME_Token(const WME_Token_Ptr_C &first, const WME_Token_Ptr_C &second)
     : m_wme_token(first, second),
     m_size(first->m_size + second->m_size)
    {
      assert(first->m_size);
      assert(second->m_size);
    }

    size_t size() const {
      return m_size;
    }

    bool operator==(const WME_Token &rhs) const {
      return m_wme_token == rhs.m_wme_token /*&& m_size == rhs.m_size*/ && m_wme == rhs.m_wme;
    }

    size_t hash() const {
      if(m_wme)
        return std::hash<WME>()(*m_wme);
      else {
        std::hash<WME_Token_Ptr_C> hasher;
        return hash_combine(hasher(m_wme_token.first), hasher(m_wme_token.second));
//        return hash_combine(m_wme_token.first->hash(), m_wme_token.second->hash());
      }
    }

    std::ostream & print(std::ostream &os) const {
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

    const Symbol_Ptr_C & operator[](const WME_Token_Index &index) const {
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

  private:
    std::pair<WME_Token_Ptr_C, WME_Token_Ptr_C> m_wme_token;
    size_t m_size;

    WME_Ptr_C m_wme;
  };

}

inline std::ostream & operator<<(std::ostream &os, const Rete::WME_Token_Index &index) {
  return os << '(' << index.first << ',' << int(index.second) << ')';
}

inline std::ostream & operator<<(std::ostream &os, const Rete::WME_Token &wme_token) {
  os << '{';
  wme_token.print(os);
  os << '}';
  return os;
}

namespace std {
  template <> struct hash<Rete::WME_Token> {
    size_t operator()(const Rete::WME_Token &wme_token) const {
      return wme_token.hash();
    }
  };
}

#endif
