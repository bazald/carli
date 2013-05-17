#ifndef ZENI_TRIE_H
#define ZENI_TRIE_H

#include "map.h"
#include "memory_pool.h"

namespace Zeni {

  template <typename KEY, typename TYPE, typename COMPARE = std::less<KEY>>
  class Trie;

  template <typename KEY, typename TYPE, typename COMPARE>
  class Trie : public Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>, public Zeni::Pool_Allocator<Trie<KEY, TYPE, COMPARE>> {
    Trie(const Trie &) = delete;
    Trie operator=(const Trie &) = delete;

  public:
    typedef KEY key_type;
    typedef KEY * key_pointer_type;
    typedef TYPE value_type;
    typedef TYPE * value_pointer_type;
    typedef const TYPE * const_value_pointer_type;
    typedef TYPE & value_reference_type;
    typedef Trie<KEY, TYPE, COMPARE> trie_value_type;
    typedef trie_value_type * trie_pointer_type;
    typedef Map<KEY, trie_value_type, COMPARE> map_value_type;
    typedef map_value_type * map_pointer_type;
    typedef Linked_List<trie_value_type> list_value_type;
    typedef list_value_type * list_pointer_type;

  public:
    Trie(const key_type &key_ = key_type())
     : Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>(this, key_)
    {
    }

    ~Trie() {
      m_deeper->destroy(m_deeper);
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
    trie_pointer_type list_insert_unique(trie_pointer_type &ptr) {
      auto lp = static_cast<list_pointer_type>(ptr);
      auto rv = list_value_type::insert_before_unique(lp, map_value_type::comparator());
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

      auto next = static_cast<trie_pointer_type>(this->next());
      this->erase();

      trie_pointer_type inserted = this;
      auto thisp = list_insert_unique(ptr); ///< this possibly deleted
      if(thisp != inserted)
        inserted = nullptr; ///< now holds non-zero value if the match was actually inserted into the function

      return thisp->finish_insert(next, offset);
    }

    template <typename DEPTH_TEST, typename TERMINAL_TEST, typename GENERATE_FRINGE, typename COLLAPSE_FRINGE>
    trie_pointer_type insert(trie_pointer_type &ptr, const bool &null_q_values, const DEPTH_TEST &depth_test, const TERMINAL_TEST &terminal_test, const GENERATE_FRINGE &generate_fringe, const COLLAPSE_FRINGE &collapse_fringe, const size_t &offset, const size_t &depth, const double &value, const bool &use_value, size_t &value_count) {
      if(!this)
        return ptr;

      auto next = static_cast<trie_pointer_type>(this->next());
      this->erase();

      trie_pointer_type inserted = this;
      auto thisp = list_insert_unique(ptr); ///< this possibly deleted
      if(thisp != inserted)
        inserted = nullptr; ///< now holds non-zero value if the match was actually inserted into the function

      return thisp->finish_insert(null_q_values, depth_test, terminal_test, generate_fringe, collapse_fringe, offset, depth, value, use_value, value_count, inserted != nullptr, next);
    }

    value_pointer_type get() const {
      return m_value;
    }
    value_pointer_type & get() {
      return m_value;
    }
    const value_reference_type operator*() const {
      return *m_value;
    }
    value_reference_type operator*() {
      return *m_value;
    }
    const_value_pointer_type operator->() const {
      return m_value;
    }
    value_pointer_type operator->() {
      return m_value;
    }

    /// return an iterator_const pointing to this list entry; only the beginning if !prev()
    typename list_value_type::iterator_const begin() const {
      return this ? typename list_value_type::iterator_const(this) : typename list_value_type::iterator_const();
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    typename list_value_type::iterator begin() {
      return this ? typename list_value_type::iterator(const_cast<trie_pointer_type>(this)) : typename list_value_type::iterator();
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    typename list_value_type::iterator_const end() const {
      return this ? typename list_value_type::iterator_const(list_value_type::offset()) : typename list_value_type::iterator_const();
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    typename list_value_type::iterator end() {
      return this ? typename list_value_type::iterator(list_value_type::offset()) : typename list_value_type::iterator();
    }

    const trie_value_type * get_deeper() const {return m_deeper;}
    trie_pointer_type & get_deeper() {return m_deeper;}

    trie_pointer_type offset_insert_before(const size_t &offset, trie_pointer_type &rhs_) {
      const bool rhs_check = rhs_ && rhs_->m_value && offset != size_t(-1);
      if(m_value) {
        if(rhs_check) {
          auto lhs = value_to_Linked_List(m_value, offset);
          assert(lhs->offset() == offset);
          auto rhs = value_to_Linked_List(rhs_->m_value, offset);
          assert(rhs->offset() == offset);
          lhs->insert_before(rhs);
          return this;
        }
        else
          return this;
      }
      else
        return rhs_check ? rhs_ : nullptr;
    }

    void offset_erase(const size_t &offset) {
      if(m_value && offset != size_t(-1)) {
        auto lhs = value_to_Linked_List(m_value, offset);
        assert(lhs->offset() == offset);
        lhs->erase();
      }
    }

  private:
    trie_pointer_type finish_insert(const trie_pointer_type &next, const size_t &offset) {
      offset_erase(offset);

      if(next) {
        static_cast<trie_pointer_type>(next)->insert(m_deeper, offset);
        return offset_insert_before(offset, m_deeper);
      }
      else {
        if(!m_value)
          m_value = new TYPE;
        return this;
      }
    }

    template <typename DEPTH_TEST, typename TERMINAL_TEST, typename GENERATE_FRINGE, typename COLLAPSE_FRINGE>
    trie_pointer_type finish_insert(const bool &null_q_values, const DEPTH_TEST &depth_test, const TERMINAL_TEST &terminal_test, const GENERATE_FRINGE &generate_fringe, const COLLAPSE_FRINGE &collapse_fringe, const size_t &offset, const size_t &depth, const double &value, const bool &use_value, size_t &value_count, const bool &force, const trie_pointer_type &next) {
      offset_erase(offset);

      if(!null_q_values && !m_value) {
        m_value = new value_type(use_value ? value : 0.0);
        ++value_count;
      }

      const bool dtr = depth_test(m_value, depth);
      const double value_next = value + (m_value ? m_value->value * m_value->weight : 0.0);

      if(next && dtr) {
        collapse_fringe(m_deeper, next);
        static_cast<trie_pointer_type>(next)->insert(m_deeper, null_q_values, depth_test, terminal_test, generate_fringe, collapse_fringe, offset, depth + 1, value_next, use_value, value_count);
        return offset_insert_before(offset, m_deeper);
      }
      else {
        try {
          terminal_test(m_value, depth, force);
        }
        catch(/*Again &*/...) {
          if(!dtr)
            next->destroy(next);
          throw;
        }

        if(null_q_values && !m_value) {
          m_value = new value_type(use_value ? value : 0.0);
          ++value_count;
        }

        if(!dtr) {
          generate_fringe(m_deeper, next, offset, value_next);
          return offset_insert_before(offset, m_deeper);
        }

        return this;
      }
    }

    trie_pointer_type m_deeper = nullptr;
    value_pointer_type m_value = nullptr;
  };

}

#endif
