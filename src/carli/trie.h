#ifndef ZENI_TRIE_H
#define ZENI_TRIE_H

#include "map.h"
#include "memory_pool.h"

namespace Zeni {

  template<typename KEY, typename TYPE, typename COMPARE = std::less<KEY> >
  class Trie;

  template<typename KEY, typename TYPE, typename COMPARE>
  class Trie /*: public Zeni::Pool_Allocator<Trie<KEY, TYPE, COMPARE> >*/ {
  public:
    typedef KEY key_type;
    typedef KEY * key_pointer_type;
    typedef TYPE value_type;
    typedef TYPE * value_pointer_type;
    typedef TYPE & value_reference_type;
    typedef Trie<KEY, TYPE, COMPARE> trie_value_type;
    typedef trie_value_type * trie_pointer_type;
    typedef Map<KEY, trie_value_type, COMPARE> map_value_type;
    typedef map_value_type * map_pointer_type;
    typedef Linked_List<trie_value_type> list_value_type;
    typedef list_value_type * list_pointer_type;

  public:
    Trie(const key_type &key_ = key_type())
     : m_map(this, key_),
     m_deeper(nullptr),
     m_value(nullptr)
    {
    }

    ~Trie() {
      m_deeper->m_map.destroy();
      delete m_value;
    }

    trie_pointer_type insert(trie_pointer_type &ptr) {
      auto next = m_map.next();
      m_map.erase();

      auto thisp = m_map.insert(reinterpret_cast<map_pointer_type &>(ptr)); /// this possibly deleted

      if(next)
        return reinterpret_cast<trie_pointer_type>(next)->insert(m_deeper);
      else {
        if(!reinterpret_cast<trie_pointer_type>(thisp)->m_value)
          reinterpret_cast<trie_pointer_type>(thisp)->m_value = new value_type;
        return reinterpret_cast<trie_pointer_type>(thisp);
      }
    }

    const value_reference_type operator*() const {
      return *m_value;
    }
    value_reference_type operator*() {
      return *m_value;
    }
    const value_pointer_type operator->() const {
      return m_value;
    }
    value_pointer_type operator->() {
      return m_value;
    }

  private:
    Map<KEY, trie_value_type, COMPARE> m_map;
    trie_pointer_type m_deeper;
    value_pointer_type m_value;
  };

}

#endif
