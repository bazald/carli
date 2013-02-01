#ifndef ZENI_TRIE_H
#define ZENI_TRIE_H

#include "map.h"
#include "memory_pool.h"

namespace Zeni {

  template <typename KEY, typename TYPE, typename COMPARE = std::less<KEY> >
  class Trie;

  template <typename KEY, typename TYPE, typename COMPARE>
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
    trie_pointer_type map_insert(trie_pointer_type &ptr) {
      auto mp = static_cast<map_pointer_type>(ptr);
      auto rv = map_value_type::insert(mp);
      ptr = static_cast<trie_pointer_type>(mp);
      return static_cast<trie_pointer_type>(rv);
    }

    /** Destroy a Trie-list, returning a pointer to the leaf if no offset is specified
     *  If an offset is specified, return a pointer to the most general match with a value first,
     *    and treat the value+offset as a Linked_List, stringing together general-to-specific values.
     */
    template <typename DEPTH_TEST, typename TERMINAL_TEST, typename GENERATE_FRINGE, typename COLLAPSE_FRINGE>
    trie_pointer_type insert(trie_pointer_type &ptr, const bool &null_q_values, const DEPTH_TEST &depth_test, const TERMINAL_TEST &terminal_test, const GENERATE_FRINGE &generate_fringe, const COLLAPSE_FRINGE &collapse_fringe, const size_t &offset = size_t(-1), const size_t &depth = size_t(), const double &value = double()) {
      if(!this)
        return ptr;

      auto next = static_cast<trie_pointer_type>(this->next());
      this->erase();

      trie_pointer_type inserted = this;
      auto thisp = map_insert(ptr); ///< this possibly deleted
      if(thisp != inserted)
        inserted = nullptr; ///< now holds non-zero value if the match was actually inserted into the function

      return thisp->finish_insert(null_q_values, depth_test, terminal_test, generate_fringe, collapse_fringe, offset, depth, value, inserted != nullptr, next);
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
    const value_pointer_type operator->() const {
      return m_value;
    }
    value_pointer_type operator->() {
      return m_value;
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
    template <typename DEPTH_TEST, typename TERMINAL_TEST, typename GENERATE_FRINGE, typename COLLAPSE_FRINGE>
    trie_pointer_type finish_insert(const bool &null_q_values, const DEPTH_TEST &depth_test, const TERMINAL_TEST &terminal_test, const GENERATE_FRINGE &generate_fringe, const COLLAPSE_FRINGE &collapse_fringe, const size_t &offset, const size_t &depth, const double &value, const bool &force, const trie_pointer_type &next) {
      if(!null_q_values && !m_value)
        m_value = new value_type;

      const bool dtr = depth_test(m_value, depth);
      trie_pointer_type deeper = nullptr;
      const double value_next = value + (m_value ? m_value->value : 0.0);

      if(next && dtr) {
        collapse_fringe(m_deeper, next);
        deeper = static_cast<trie_pointer_type>(next)->insert(m_deeper, null_q_values, depth_test, terminal_test, generate_fringe, collapse_fringe, offset, depth + 1, value_next);
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

        if(null_q_values && !m_value)
          m_value = new value_type;

        if(!dtr) {
          generate_fringe(m_deeper, next, offset, value_next);
          deeper = m_deeper;
        }
      }

      offset_erase(offset);
      return offset_insert_before(offset, deeper);
    }

    trie_pointer_type m_deeper;
    value_pointer_type m_value;
  };

}

#endif
