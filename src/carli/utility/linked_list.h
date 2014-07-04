#ifndef ZENI_LINKED_LIST_H
#define ZENI_LINKED_LIST_H

#include "tracked_ptr.h"

#include <cstddef>
#include <algorithm>
#include <functional>
#include <inttypes.h>

namespace Zeni {

  template <typename TYPE>
  class Linked_List;

  template <typename TYPE>
  Linked_List<const TYPE> * value_to_Linked_List(const TYPE * const type, const ptrdiff_t &offset);

  template <typename TYPE>
  Linked_List<TYPE> * value_to_Linked_List(TYPE * const type, const ptrdiff_t &offset);

  template <typename TYPE>
  class Linked_List {
    Linked_List(const Linked_List &) = delete;
    Linked_List operator=(const Linked_List &) = delete;

  public:
    static const tracked_ptr_mode tpm = TPM_UNDEL_UNTRA;
    typedef TYPE value_type;
    typedef tracked_ptr<value_type, null_delete<value_type>, tpm> value_pointer_type;
    typedef const value_type & const_value_reference_type;
    typedef value_type & value_reference_type;
    typedef const Linked_List<TYPE> const_list_value_type;
    typedef Linked_List<TYPE> list_value_type;
    typedef tracked_ptr<const_list_value_type, null_delete<const_list_value_type>, tpm> const_list_pointer_type;
    typedef tracked_ptr<list_value_type, null_delete<const_list_value_type>, tpm> list_pointer_type;
    typedef std::less<value_type> compare_default;

    class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef TYPE * pointer;
      typedef TYPE & reference;

      iterator(const ptrdiff_t &m_offset_ = 0lu)
        : m_offset(m_offset_),
        m_pointer(nullptr)
      {
      }

      iterator(const list_pointer_type &ptr)
        : m_offset(ptr->m_offset),
        m_pointer(ptr)
      {
      }

      value_pointer_type get() const {
        return m_pointer->get();
      }
      value_reference_type operator*() const {
        return *get();
      }
      value_pointer_type operator->() const {
        return get();
      }

      iterator prev() const {
        if(m_pointer->m_prev)
          return iterator(m_pointer->m_prev);
        else
          return iterator(m_offset);
      }
      iterator next() const {
        if(m_pointer->m_next)
          return iterator(m_pointer->m_prev);
        else
          return iterator(m_offset);
      }

      iterator & operator--() {
        m_pointer = m_pointer->m_prev;
        return *this;
      }
      iterator operator--(int) {
        iterator rv(*this);
        m_pointer = m_pointer->m_prev;
        return rv;
      }
      iterator & operator++() {
        m_pointer = m_pointer->m_next;
        return *this;
      }
      iterator operator++(int) {
        iterator rv(*this);
        m_pointer = m_pointer->m_next;
        return rv;
      }

      bool operator==(const typename Linked_List<value_type>::iterator &rhs) const {
        assert(m_offset == rhs.m_offset);
        return m_offset == rhs.m_offset &&
               m_pointer == rhs.m_pointer;
      }
      bool operator!=(const typename Linked_List<value_type>::iterator &rhs) const {
        assert(m_offset == rhs.m_offset);
        return m_offset != rhs.m_offset ||
               m_pointer != rhs.m_pointer;
      }
      bool operator==(const list_pointer_type &rhs) const {
        return m_pointer == rhs;
      }
      bool operator!=(const list_pointer_type &rhs) const {
        return m_pointer != rhs;
      }

      operator list_pointer_type () const {
        return m_pointer;
      }
      operator list_pointer_type & () {
        return m_pointer;
      }

    private:
      ptrdiff_t m_offset;
      list_pointer_type m_pointer;
    };

    class iterator_const : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef tracked_ptr<value_type> pointer;
      typedef TYPE & reference;

      typedef tracked_ptr<const value_type> value_pointer_type;
      typedef const value_type & value_reference_type;
      typedef tracked_ptr<list_value_type> list_pointer_type;

      iterator_const(const ptrdiff_t &m_offset_ = 0lu)
        : m_offset(m_offset_),
        m_pointer(nullptr)
      {
      }

      iterator_const(const list_pointer_type &ptr)
        : m_offset(ptr->m_offset),
        m_pointer(ptr)
      {
      }

      iterator_const(const iterator &rhs)
        : m_offset(rhs.m_offset),
        m_pointer(rhs.m_pointer)
      {
      }

      iterator_const & operator=(const iterator &rhs) {
        m_offset = rhs.m_offset;
        m_pointer = rhs.m_pointer;
        return *this;
      }

      value_pointer_type get() const {
        return m_pointer->get();
      }
      value_reference_type operator*() const {
        return *get();
      }
      value_pointer_type operator->() const {
        return get();
      }

      iterator_const prev() const {
        if(m_pointer->m_prev)
          return iterator_const(m_pointer->m_prev);
        else
          return iterator_const(m_offset);
      }
      iterator_const next() const {
        if(m_pointer->m_next)
          return iterator_const(m_pointer->m_prev);
        else
          return iterator_const(m_offset);
      }

      iterator_const & operator--() {
        m_pointer = m_pointer->m_prev;
        return *this;
      }
      iterator_const operator--(int) {
        iterator_const rv(*this);
        m_pointer = m_pointer->m_prev;
        return rv;
      }
      iterator_const & operator++() {
        m_pointer = m_pointer->m_next;
        return *this;
      }
      iterator_const operator++(int) {
        iterator_const rv(*this);
        m_pointer = m_pointer->m_next;
        return rv;
      }

      bool operator==(const typename Linked_List<value_type>::iterator_const &rhs) const {
        assert(m_offset == rhs.m_offset);
        return m_offset == rhs.m_offset &&
               m_pointer == rhs.m_pointer;
      }
      bool operator!=(const typename Linked_List<value_type>::iterator_const &rhs) const {
        assert(m_offset == rhs.m_offset);
        return m_offset != rhs.m_offset ||
               m_pointer != rhs.m_pointer;
      }
      bool operator==(const list_pointer_type &rhs) const {
        return m_pointer == rhs;
      }
      bool operator!=(const list_pointer_type &rhs) const {
        return m_pointer != rhs;
      }

      operator list_pointer_type () const {
        return m_pointer;
      }
      operator list_pointer_type & () {
        return m_pointer;
      }

    private:
      ptrdiff_t m_offset;
      list_pointer_type m_pointer;
    };

    Linked_List(value_type * value)
      : m_offset(reinterpret_cast<char *>(this) - reinterpret_cast<char *>(value)),
      m_prev(nullptr),
      m_next(nullptr)
    {
//      assert(m_offset < sizeof(value_type));
    }

    ~Linked_List() {
#ifndef NDEBUG
      const auto count = pointer_tracker<value_pointer_type::tracked>::count(this);
      assert(!count);
#endif
    }

    value_pointer_type get() const {
      return reinterpret_cast<value_type *>((reinterpret_cast<char *>(const_cast<Zeni::Linked_List<value_type> *>(this)) - m_offset));
    }
    const_value_reference_type operator*() const {
      return *get();
    }
    value_reference_type operator*() {
      return *get();
    }
    value_pointer_type operator->() const {
      return get();
    }
    value_pointer_type operator->() {
      return get();
    }

    ptrdiff_t offset() const {
      return m_offset;
    }
    list_pointer_type prev() const {
      return m_prev;
    }
    list_pointer_type next() const {
      return m_next;
    }

    /// return an iterator_const pointing to this list entry; only the beginning if !prev()
    iterator_const begin() const {
      return this ? iterator_const(this) : iterator_const();
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    iterator begin() {
      return this ? iterator(this) : iterator();
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    iterator_const end() const {
      return this ? iterator_const(offset()) : iterator_const();
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    iterator end() {
      return this ? iterator(offset()) : iterator();
    }

    /// insert this list entry after ptr; requires this or ptr to have !next()
    void insert_after(const list_pointer_type &ptr) {
      if(ptr) {
        assert(m_offset == ptr->m_offset);
        assert(!m_prev);
        assert(!m_next || !ptr->m_next);

        if(ptr->m_next) {
          ptr->m_next->m_prev = this;
          m_next = ptr->m_next;
        }

        ptr->m_next = this;
        m_prev = ptr;
      }
    }
    /// insert this list entry before ptr; requires this or ptr to have !prev()
    void insert_before(list_pointer_type &ptr) {
      if(ptr) {
        assert(m_offset == ptr->m_offset);
        assert(!m_next);
        assert(!m_prev || !ptr->m_prev);

        if(ptr->m_prev) {
          ptr->m_prev->m_next = this;
          m_prev = ptr->m_prev;
        }

        ptr->m_prev = this;
        m_next = ptr;
      }

      ptr = this;
    }

    template <typename COMPARE = std::less<TYPE>>
    list_pointer_type find_gte(const value_type &value, const COMPARE &compare = COMPARE(), list_pointer_type * const &pptr = nullptr) {
      if(pptr)
        *pptr = nullptr;
      list_pointer_type pp = this;
      while(pp && compare(**pp, value)) {
        if(pptr)
          *pptr = pp;
        pp = pp->m_next;
      }

      return pp;
    }

    template <typename COMPARE = std::less<TYPE>>
    list_pointer_type find_in_order(const value_type &value, const COMPARE &compare = COMPARE(), list_pointer_type * const &pptr = nullptr) {
      const list_pointer_type pp = find_gte(value, compare, pptr);

      if(pp && !compare(value, **pp))
        return pp;

      return nullptr;
    }

    template <typename COMPARE = std::less<TYPE>>
    list_pointer_type find(const value_type &value, const COMPARE &compare = COMPARE()) {
      if(this)
        if(compare(value, **this) || compare(**this, value))
          return m_next->find(value, compare);

      return this;
    }

    /** insert this list entry into the list; requires this to have !prev() && !next()
     *  return same pointer if inserted, different pointer (moved to front) if already exists, and deleted
     */
    template <typename COMPARE = std::less<TYPE>>
    list_pointer_type insert_before_unique(list_pointer_type &ptr, const COMPARE &compare = COMPARE()) {
      assert(!m_prev && !m_next);

      if(ptr) {
        assert(m_offset == ptr->m_offset);

        const list_pointer_type pp = ptr->find(**this, compare);

        if(pp && !compare(**this, **pp)) {
          destroy(this);

          if(pp->m_prev) {
            pp->erase();
            pp->m_next = ptr;
            ptr->m_prev = pp;
            ptr = pp;
          }

          return pp;
        }
        else {
          m_next = ptr;
          ptr->m_prev = this;
        }
      }

      ptr = this;

      return this;
    }

    /** insert this list entry into the list; requires this to have !prev() && !next()
     *  return same pointer if inserted, different pointer if already exists, duplicate == false, and deleted
     */
    template <typename COMPARE = std::less<TYPE>>
    list_pointer_type insert_in_order(list_pointer_type &ptr, const bool &duplicate = true, const COMPARE &compare = COMPARE()) {
      assert(!m_prev && !m_next);

      if(ptr) {
        assert(m_offset == ptr->m_offset);

        list_pointer_type pptr = nullptr;
        const list_pointer_type pp = ptr->find_gte(**this, compare, &pptr);

        if(pp && !duplicate && !compare(**this, **pp)) {
          destroy(this);
          return pp;
        }

        if(pptr)
          pptr->m_next = this;
        if(pp)
          pp->m_prev = this;
        m_prev = pptr;
        m_next = pp;

        if(ptr == pp)
          ptr = this;
      }
      else
        ptr = this;

      return this;
    }

    /// erase this entry from this list
    void erase_from(list_pointer_type &ptr) {
      if(ptr == this)
        ptr = m_next;
      
      if(m_prev)
        m_prev->m_next = m_next;
      if(m_next)
        m_next->m_prev = m_prev;

      erase_hard();
    }

    void erase_hard() {
      m_prev.zero();
      m_next.zero();
    }

    /// delete every entry in the list between begin() and end(), inclusive
    static void destroy(list_pointer_type const &ptr_) {
      auto ptr = ptr_;
      destroy(ptr);
    }
    static void destroy(list_pointer_type &ptr_) {
      if(ptr_) {
        list_pointer_type ptr = ptr_;
        while(ptr) {
          auto tptr = ptr->get();
          ptr = ptr->m_next;
          tptr.delete_and_zero();
        }
        ptr_ = nullptr;
      }
    }

    bool operator<(const Linked_List<TYPE> &rhs) const {return compare(rhs) < 0;}
    bool operator<=(const Linked_List<TYPE> &rhs) const {return compare(rhs) <= 0;}
    bool operator>(const Linked_List<TYPE> &rhs) const {return compare(rhs) > 0;}
    bool operator>=(const Linked_List<TYPE> &rhs) const {return compare(rhs) >= 0;}
    bool operator==(const Linked_List<TYPE> &rhs) const {return compare(rhs) == 0;}
    bool operator!=(const Linked_List<TYPE> &rhs) const {return compare(rhs) != 0;}

    int64_t compare(const Linked_List<TYPE> &rhs) const {
      auto it = begin();
      auto iend = end();
      auto jt = rhs.begin();
      auto jend = rhs.end();

      while(it != iend) {
        if(jt != jend) {
          if(*it < *jt)
            return -1;
          else if(*jt < *it)
            return 1;
        }
        else
          return 1;

        ++it;
        ++jt;
      }

      return jt != jend ? -1 : 0;
    }

  protected:
    list_pointer_type & get_prev() {
      return m_prev;
    }
    list_pointer_type & get_next() {
      return m_next;
    }

    void set_prev(const list_pointer_type &prev_) {
//      assert(!prev_ || prev_->m_offset == m_offset);
      m_prev = prev_;
    }
    void set_next(const list_pointer_type &next_) {
//      assert(!next_ || next_->m_offset == m_offset);
      m_next = next_;
    }

  private:
    ptrdiff_t m_offset;
    list_pointer_type m_prev;
    list_pointer_type m_next;
  };

  template <typename TYPE>
  Linked_List<const TYPE> * value_to_Linked_List(const TYPE * const type, const ptrdiff_t &offset) {
    return type ? reinterpret_cast<Linked_List<const TYPE> *>(reinterpret_cast<char *>(type) + offset) : nullptr;
  }

  template <typename TYPE>
  Linked_List<TYPE> * value_to_Linked_List(TYPE * const type, const ptrdiff_t &offset) {
    return type ? reinterpret_cast<Linked_List<TYPE> *>(reinterpret_cast<char *>(type) + offset) : nullptr;
  }

}

#endif
