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

//  std::pair<Variable_Indices_Ptr_C, Variable_Indices_Ptr_C> split_Variable_Indices(const WME_Bindings &bindings, const Variable_Indices_Ptr_C &indices, const int64_t &offset) {
////    std::pair<Variable_Indices_Ptr, Variable_Indices_Ptr> rv =
////      std::make_pair(std::make_shared<Variable_Indices>(), std::make_shared<Variable_Indices>());
//    std::pair<Variable_Indices_Ptr_C, Variable_Indices_Ptr> rv =
//      std::make_pair(indices, std::make_shared<Variable_Indices>());
//
//    for(const auto &index : *indices) {
//      if(index.second.first < offset)
//        ;// (*rv.first)[index.first] = index.second;
//      else
//        rv.second->insert(std::make_pair(index.first, index.second));
//    }
//
//    for(const auto &binding : bindings) {
//      const auto found = find_if(rv.first->begin(), rv.first->end(), [&binding](const std::pair<std::string, WME_Token_Index> &index)->bool {
//        return index.second == binding.second;
//      });
//      assert(found != rv.first->end());
//      rv.second->insert(std::make_pair(found->first, binding.second));
//    }
//
//    return rv;
//  }

  Variable_Indices_Ptr_C bind_Variable_Indices(const WME_Bindings &bindings, const Variable_Indices_Ptr_C &indices, const int64_t &offset) {
    Variable_Indices_Ptr closure = std::make_shared<Variable_Indices>();

    for(const auto &index : *indices) {
      if(index.second.first >= offset)
        closure->insert(std::make_pair(index.first, WME_Token_Index(index.second.first - offset, index.second.second)));
    }

    for(const auto &binding : bindings) {
      auto found = std::find_if(indices->begin(), indices->end(), [binding,offset](const std::pair<std::string, WME_Token_Index> &ind)->bool {
        return ind.second == binding.first;
      });
      if(found != indices->end()) {
        closure->insert(std::make_pair(found->first, WME_Token_Index(binding.second.first, binding.second.second)));
        continue;
      }

      found = std::find_if(closure->begin(), closure->end(), [binding,offset](const std::pair<std::string, WME_Token_Index> &ind)->bool {
        return ind.second == binding.first;
      });
      if(found != closure->end())
        closure->insert(std::make_pair(found->first, WME_Token_Index(binding.second.first, binding.second.second)));
    }

    return closure;
  }

  std::string get_Variable_name(const Variable_Indices_Ptr_C &indices, const WME_Token_Index &index) {
    const auto found = std::find_if(indices->begin(), indices->end(), [index](const std::pair<std::string, WME_Token_Index> &ind)->bool {
      return ind.second.first == index.first && ind.second.second == index.second;
    });
    assert(found != indices->end());
    if(found != indices->end())
      return found->first;

    std::ostringstream oss;
    oss << index;
    return oss.str();
  }

}

std::ostream & operator<<(std::ostream &os, const Rete::WME_Binding &binding) {
  return os << binding.first << ':' << binding.second;
}

std::ostream & operator<<(std::ostream &os, const Rete::WME_Bindings &bindings) {
  if(bindings.empty())
    return os << "[]";
  os << '[';
  auto bt = bindings.begin();
  os << *bt++;
  while(bt != bindings.end())
    os << ',' << *bt++;
  os << ']';
  return os;
}

std::ostream & operator<<(std::ostream &os, const Rete::WME_Token &wme_token) {
  os << '{';
  wme_token.print(os);
  os << '}';
  return os;
}
