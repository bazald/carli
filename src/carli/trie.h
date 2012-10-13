#ifndef ZENI_TRIE_H
#define ZENI_TRIE_H

#include "map.h"
#include "memory_pool.h"

namespace Zeni {

  template<typename KEY, typename TYPE, typename COMPARE = std::less<KEY> >
  class Trie;

  template<typename KEY, typename TYPE, typename COMPARE>
  class Trie : public Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>, public Zeni::Pool_Allocator<Trie<KEY, TYPE, COMPARE> > {
    Trie(const Trie &);
    Trie operator=(const Trie &);

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
    trie_pointer_type list_insert_in_order(trie_pointer_type &ptr) {
      auto lp = static_cast<list_pointer_type>(ptr);
      auto rv = list_value_type::insert_in_order(lp, false, COMPARE());
      ptr = static_cast<trie_pointer_type>(lp);
      return static_cast<trie_pointer_type>(rv);
    }

    /** Destroy a Trie-list, returning a pointer to the leaf if no offset is specified
     *  If an offset is specified, return a pointer to the most general match with a value first,
     *    and treat the value+offset as a Linked_List, stringing together general-to-specific values.
     */
    trie_pointer_type insert(trie_pointer_type &ptr, const size_t &offset = size_t(-1)) {
      if(!this)
        return ptr;

      auto next = this->next();
      this->erase();

      auto mp = static_cast<map_pointer_type>(ptr);
      auto thisp = static_cast<trie_pointer_type>(this->map_value_type::insert(mp)); ///< this possibly deleted
      ptr = static_cast<trie_pointer_type>(mp);

      return thisp->finish_insert(ptr, offset, next);
    }

    value_pointer_type get() const {
      return m_value;
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
    trie_pointer_type finish_insert(trie_pointer_type &ptr, const size_t &offset, const list_pointer_type &next) {
      if(next) {
        auto deeper = static_cast<trie_pointer_type>(next)->insert(m_deeper, offset);
        if(offset != size_t(-1) && m_value) {
          auto lhs = reinterpret_cast<Linked_List<value_type> *>(reinterpret_cast<char *>(m_value) + offset);
          assert(lhs->offset() == offset);
          auto rhs = reinterpret_cast<Linked_List<value_type> *>(reinterpret_cast<char *>(deeper->m_value) + offset);
          assert(rhs->offset() == offset);
          lhs->erase();
          lhs->insert_before(rhs);
        }
        else
          return deeper;
      }
      else {
        if(!m_value)
          m_value = new value_type;
        if(offset != size_t(-1)) {
          auto lhs = reinterpret_cast<Linked_List<value_type> *>(reinterpret_cast<char *>(m_value) + offset);
          assert(lhs->offset() == offset);
          lhs->erase();
        }
      }

      return this;
    }

    trie_pointer_type m_deeper;
    value_pointer_type m_value;
  };

}

#endif
