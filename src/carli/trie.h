#ifndef ZENI_TRIE_H
#define ZENI_TRIE_H

#include "map.h"
#include "memory_pool.h"

namespace Zeni {

  template<typename KEY, typename TYPE, typename COMPARE = std::less<KEY> >
  class Trie;

  template<typename KEY, typename TYPE, typename COMPARE>
  class Trie : public Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>, public Zeni::Pool_Allocator<Trie<KEY, TYPE, COMPARE> > {
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
     : Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>(this, key_),
     m_deeper(nullptr),
     m_value(nullptr)
    {
    }

    ~Trie() {
      if(m_deeper)
        m_deeper->destroy();
      delete m_value;
    }

    void list_insert_after(const trie_pointer_type &ptr) {
      list_value_type::insert_after(ptr);
    }
    void list_insert_before(trie_pointer_type &ptr) {
      auto lp = static_cast<list_pointer_type>(ptr);
      list_value_type::insert_before(lp);
      ptr = static_cast<trie_pointer_type>(lp);
    }

    trie_pointer_type insert(trie_pointer_type &ptr) {
      auto next = this->next();
      this->erase();

      auto mp = static_cast<map_pointer_type>(ptr);
      auto thisp = static_cast<trie_pointer_type>(this->Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>::insert(mp)); ///< this possibly deleted
      ptr = static_cast<trie_pointer_type>(mp);

      if(next)
        return static_cast<trie_pointer_type>(next)->insert(thisp->m_deeper);
      else {
        if(!thisp->m_value)
          thisp->m_value = new value_type;
        return thisp;
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
    trie_pointer_type m_deeper;
    value_pointer_type m_value;
  };

}

#endif
