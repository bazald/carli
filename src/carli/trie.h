#ifndef ZENI_TRIE_H
#define ZENI_TRIE_H

#include "map.h"
#include "memory_pool.h"

#include <limits>
#include <utility>

namespace Zeni {

  template <typename KEY, typename TYPE, typename COMPARE = std::less<KEY>>
  class Trie;

  template <typename KEY, typename TYPE, typename COMPARE>
  class Trie : private Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>, public Zeni::Pool_Allocator<Trie<KEY, TYPE, COMPARE>> {
    friend Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>;

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
    typedef typename map_value_type::iterator_const iterator_const;
    typedef typename map_value_type::iterator iterator;

    trie_pointer_type list_prev() const {
      return static_cast<trie_pointer_type>(map_value_type::list_prev());
    }
    trie_pointer_type list_next() const {
      return static_cast<trie_pointer_type>(map_value_type::list_next());
    }

    void list_insert_after(const trie_pointer_type &ptr) {
      map_value_type::list_insert_after(ptr);
    }
    void list_insert_before(trie_pointer_type &ptr) {
      auto mp = static_cast<map_pointer_type>(ptr);
      map_value_type::list_insert_before(mp);
      ptr = static_cast<trie_pointer_type>(mp);
    }
    void list_destroy(const trie_pointer_type &ptr_) {
      trie_pointer_type ptr = ptr_;
      list_destroy(ptr);
    }
    void list_destroy(trie_pointer_type &ptr) {
      auto mp = static_cast<map_pointer_type>(ptr);
      map_value_type::list_destroy(mp);
      ptr = static_cast<trie_pointer_type>(mp);
    }
    void list_erase() {
      map_value_type::list_erase();
    }
    void list_erase_hard() {
      map_value_type::list_erase_hard();
    }

    trie_pointer_type map_insert_into_unique(trie_pointer_type &root) {
      map_pointer_type mp = static_cast<map_pointer_type>(root);
      const map_pointer_type rv = map_value_type::insert_into_unique(mp);
      root = static_cast<trie_pointer_type>(mp);
      return static_cast<trie_pointer_type>(rv);
    }

#ifndef NDEBUG
    template <typename VISITOR>
    VISITOR map_debug_print_visit(std::ostream &os, VISITOR visitor) const {
      return map_value_type::debug_print_visit(os, visitor);
    }
#endif

    Trie(const key_type &key_ = key_type())
     : Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>(this, key_)
    {
    }

    Trie(key_type &&key_ = key_type())
     : Map<KEY, Trie<KEY, TYPE, COMPARE>, COMPARE>(this, std::forward<key_type>(key_))
    {
    }

    ~Trie() {
      delete m_value;
    }

    /** Destroy a Trie-list, returning a pointer to the leaf if no offset is specified
     *  If an offset is specified, return a pointer to the most general match with a value first,
     *    and treat the value+offset as a Linked_List, stringing together general-to-specific values.
     */
    trie_pointer_type insert(trie_pointer_type &ptr, const ptrdiff_t &offset = std::numeric_limits<ptrdiff_t>::max()) {
      const trie_pointer_type next_ = list_next();
      list_erase();

      auto mp = static_cast<map_pointer_type>(ptr);
      const trie_pointer_type thisp = static_cast<trie_pointer_type>(map_value_type::insert_into_unique(mp)); ///< this possibly deleted
      ptr = static_cast<trie_pointer_type>(mp);

      return thisp->finish_insert(next_, offset);
    }

//    template <typename DEPTH_TEST, typename TERMINAL_TEST, typename GENERATE_FRINGE, typename COLLAPSE_FRINGE>
//    trie_pointer_type insert(trie_pointer_type &ptr, const bool &null_q_values, const DEPTH_TEST &depth_test, const TERMINAL_TEST &terminal_test, const GENERATE_FRINGE &generate_fringe, const COLLAPSE_FRINGE &collapse_fringe, const ptrdiff_t &offset, const size_t &depth, const double &value, const bool &use_value, size_t &value_count) {
//      const trie_pointer_type next_ = list_next();
//      list_erase();
//
//      const trie_pointer_type inserted = this;
//
//      auto mp = static_cast<map_pointer_type>(ptr);
//      const trie_pointer_type thisp = static_cast<trie_pointer_type>(map_value_type::insert_into_unique(mp)); ///< this possibly deleted
//      ptr = static_cast<trie_pointer_type>(mp);
//
//      return thisp->finish_insert(null_q_values, depth_test, terminal_test, generate_fringe, collapse_fringe, offset, depth, value, use_value, value_count, thisp == inserted, next_);
//    }

    template <typename TYPE2, typename COMPARE2 = COMPARE>
    trie_pointer_type find(const TYPE2 &value, const COMPARE2 &compare = COMPARE2()) {
      return this ? static_cast<trie_pointer_type>(map_value_type::find(value, compare)) : nullptr;
    }

    const key_type & get_key() const {
      return map_value_type::get_key();
    }

    static void destroy(const trie_pointer_type &ptr_) {
      trie_pointer_type ptr = ptr_;
      destroy(ptr);
    }
    static void destroy(trie_pointer_type &ptr_) {
      if(ptr_) {
        destroy(ptr_->m_deeper);
        map_value_type::destroy(ptr_);
        ptr_ = nullptr;
      }
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

    trie_pointer_type prev() const {
      return static_cast<trie_pointer_type>(map_value_type::prev());
    }
    trie_pointer_type next() const {
      return static_cast<trie_pointer_type>(map_value_type::next());
    }

    trie_pointer_type first() const {
      return this ? static_cast<trie_pointer_type>(map_value_type::first()) : nullptr;
    }

    /// return an iterator_const pointing to this list entry; only the beginning if !prev()
    iterator_const begin() const {
      return this ? iterator_const(first()) : iterator_const();
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    iterator begin() {
      return this ? iterator(const_cast<trie_pointer_type>(first())) : iterator();
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    iterator_const end() const {
      return this ? iterator_const(map_value_type::offset()) : iterator_const();
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    iterator end() {
      return this ? iterator(map_value_type::offset()) : iterator();
    }

    const trie_value_type * get_deeper() const {return m_deeper;}
    trie_pointer_type & get_deeper() {return m_deeper;}

    trie_pointer_type offset_insert_before(const ptrdiff_t &offset, const trie_pointer_type &rhs_) {
      const bool rhs_check = rhs_ && rhs_->m_value && offset != std::numeric_limits<ptrdiff_t>::max();
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

    void offset_erase(const ptrdiff_t &offset) {
      if(m_value && offset != std::numeric_limits<ptrdiff_t>::max()) {
        auto lhs = value_to_Linked_List(m_value, offset);
        assert(lhs->offset() == offset);
        lhs->erase();
      }
    }

    void offset_erase_hard(const ptrdiff_t &offset) {
      if(m_value && offset != std::numeric_limits<ptrdiff_t>::max()) {
        auto lhs = value_to_Linked_List(m_value, offset);
        assert(lhs->offset() == offset);
        lhs->erase_hard();
      }
    }

  private:
    trie_pointer_type finish_insert(const trie_pointer_type &next, const ptrdiff_t &offset) {
      offset_erase_hard(offset);

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

//    template <typename DEPTH_TEST, typename TERMINAL_TEST, typename GENERATE_FRINGE, typename COLLAPSE_FRINGE>
//    trie_pointer_type finish_insert(const bool &null_q_values, const DEPTH_TEST &depth_test, const TERMINAL_TEST &terminal_test, const GENERATE_FRINGE &generate_fringe, const COLLAPSE_FRINGE &collapse_fringe, const ptrdiff_t &offset, const size_t &depth, const double &value, const bool &use_value, size_t &value_count, const bool &force, const trie_pointer_type &next) {
//      offset_erase_hard(offset);
//
//      if(!null_q_values && !m_value) {
//        m_value = new value_type(use_value ? value : 0.0);
//        ++value_count;
//      }
//
//      const bool dtr = depth_test(m_value, depth);
//      const double value_next = value + (m_value ? m_value->value * m_value->weight : 0.0);
//
//      if(next && dtr) {
//        collapse_fringe(m_deeper, next);
//        static_cast<trie_pointer_type>(next)->insert(m_deeper, null_q_values, depth_test, terminal_test, generate_fringe, collapse_fringe, offset, depth + 1, value_next, use_value, value_count);
//        return offset_insert_before(offset, m_deeper);
//      }
//      else {
//        try {
//          terminal_test(m_value, depth, force);
//        }
//        catch(/*Again &*/...) {
//          if(!dtr)
//            next->list_destroy(next);
//          throw;
//        }
//
//        if(null_q_values && !m_value) {
//          m_value = new value_type(use_value ? value : 0.0);
//          ++value_count;
//        }
//
//        if(!dtr) {
//          auto deeper = generate_fringe(m_deeper, next, offset, value_next);
//          return offset_insert_before(offset, deeper);
//        }
//
//        return this;
//      }
//    }

    trie_pointer_type m_deeper = nullptr;
    value_pointer_type m_value = nullptr;
  };

}

#endif
