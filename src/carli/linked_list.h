#ifndef ZENI_LINKED_LIST_H
#define ZENI_LINKED_LIST_H

#include <cassert>
#include <algorithm>

namespace Zeni {

  template<typename TYPE>
  class Linked_List {
  public:
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef value_type & value_reference_type;
    typedef const Linked_List<TYPE> const_list_value_type;
    typedef Linked_List<TYPE> list_value_type;
    typedef const_list_value_type * const_list_pointer_type;
    typedef list_value_type * list_pointer_type;

    class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      iterator()
        : offset(0),
        pointer(nullptr)
      {
      }
      
      iterator(const size_t &offset_)
        : offset(offset_),
        pointer(nullptr)
      {
      }

      iterator(const list_pointer_type &ptr)
        : offset(ptr->m_offset),
        pointer(ptr)
      {
      }

      value_pointer_type get() const {
        return pointer->get();
      }
      value_reference_type operator*() const {
        return *get();
      }
      value_pointer_type operator->() const {
        return get();
      }
      
      iterator prev() const {
        if(pointer->m_prev)
          return iterator(pointer->m_prev->get());
        else
          return iterator();
      }
      iterator next() const {
        if(pointer->m_next)
          return iterator(pointer->m_prev->get());
        else
          return iterator();
      }
      
      iterator operator--() {
        pointer = pointer->m_prev;
        return *this;
      }
      iterator operator--(int) {
        iterator rv(*this);
        pointer = pointer->m_prev;
        return rv;
      }
      iterator operator++() {
        pointer = pointer->m_next;
        return *this;
      }
      iterator operator++(int) {
        iterator rv(*this);
        pointer = pointer->m_next;
        return rv;
      }
      
      bool operator==(const typename Linked_List<value_type>::iterator &rhs) const {
        assert(offset == rhs.offset);
        return offset == rhs.offset &&
               pointer == rhs.pointer;
      }
      bool operator!=(const typename Linked_List<value_type>::iterator &rhs) const {
        assert(offset == rhs.offset);
        return offset != rhs.offset ||
               pointer != rhs.pointer;
      }

      operator list_pointer_type () const {
        return pointer;
      }
      operator list_pointer_type & () {
        return pointer;
      }

    private:
      size_t offset;
      list_pointer_type pointer;
    };

    class iterator_const : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef const value_type * value_pointer_type;
      typedef const value_type & value_reference_type;
      typedef const list_value_type * list_pointer_type;

      iterator_const()
        : offset(0),
        pointer(nullptr)
      {
      }
      
      iterator_const(const size_t &offset_)
        : offset(offset_),
        pointer(nullptr)
      {
      }

      iterator_const(const list_pointer_type &ptr)
        : offset(ptr->m_offset),
        pointer(ptr)
      {
      }

      iterator_const(const iterator &rhs)
        : offset(rhs.offset),
        pointer(rhs.pointer)
      {
      }

      iterator_const & operator=(const iterator &rhs) {
        offset = rhs.offset;
        pointer = rhs.pointer;
        return *this;
      }

      value_pointer_type get() const {
        return pointer->get();
      }
      value_reference_type operator*() const {
        return *get();
      }
      value_pointer_type operator->() const {
        return get();
      }
      
      iterator_const prev() const {
        if(pointer->m_prev)
          return iterator_const(pointer->m_prev->get());
        else
          return iterator_const();
      }
      iterator_const next() const {
        if(pointer->m_next)
          return iterator_const(pointer->m_prev->get());
        else
          return iterator_const();
      }
      
      iterator_const operator--() {
        pointer = pointer->m_prev;
        return *this;
      }
      iterator_const operator--(int) {
        iterator_const rv(*this);
        pointer = pointer->m_prev;
        return rv;
      }
      iterator_const operator++() {
        pointer = pointer->m_next;
        return *this;
      }
      iterator_const operator++(int) {
        iterator_const rv(*this);
        pointer = pointer->m_next;
        return rv;
      }
      
      bool operator==(const typename Linked_List<value_type>::iterator_const &rhs) const {
        assert(offset == rhs.offset);
        return offset == rhs.offset &&
               pointer == rhs.pointer;
      }
      bool operator!=(const typename Linked_List<value_type>::iterator_const &rhs) const {
        assert(offset == rhs.offset);
        return offset != rhs.offset ||
               pointer != rhs.pointer;
      }

      operator list_pointer_type () const {
        return pointer;
      }
      operator list_pointer_type & () {
        return pointer;
      }

    private:
      size_t offset;
      list_pointer_type pointer;
    };

    Linked_List(value_pointer_type value)
      : m_offset(reinterpret_cast<char *>(this) - reinterpret_cast<char *>(value)),
      m_prev(nullptr),
      m_next(nullptr)
    {
      assert(m_offset < sizeof(value_type));
    }

    value_pointer_type get() const {
      return reinterpret_cast<value_pointer_type>((reinterpret_cast<char *>(const_cast<Zeni::Linked_List<value_type> *>(this)) - m_offset));
    }
    const value_reference_type operator*() const {
      return *get();
    }
    value_reference_type operator*() {
      return *get();
    }
    const value_pointer_type operator->() const {
      return get();
    }
    value_pointer_type operator->() {
      return get();
    }

    size_t offset() const {
      return m_offset;
    }
    Linked_List * prev() const {
      return m_prev;
    }
    Linked_List * next() const {
      return m_next;
    }
    
    /// return an iterator_const pointing to this list entry; only the beginning if !prev()
    iterator_const begin() const {
      return iterator_const(this);
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    iterator begin() {
      return iterator(this);
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    iterator_const end() const {
      return iterator_const(m_offset);
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    iterator end() {
      return iterator(m_offset);
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

    template <typename COMPARE = std::less<TYPE> >
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

    template <typename COMPARE = std::less<TYPE> >
    list_pointer_type find(const value_type &value, const COMPARE &compare = COMPARE(), list_pointer_type * const &pptr = nullptr) {
      const list_pointer_type pp = find_gte(value, compare, pptr);

      if(pp && !compare(value, **pp))
        return pp;

      return nullptr;
    }

    /** insert this list entry into the list; requires this to have !prev() && !next()
     *  return pointer to list entry, and true if inserted : false if not inserted
     */ 
    template <typename COMPARE = std::less<TYPE> >
    std::pair<list_pointer_type, bool> insert_in_order(list_pointer_type &ptr, const bool &duplicate = true, const COMPARE &compare = COMPARE()) {
      if(ptr) {
        assert(m_offset == ptr->m_offset);
        assert(!m_prev && !m_next);

        list_pointer_type pptr = nullptr;
        const list_pointer_type pp = ptr->find_gte(**this, compare, &pptr);

        if(pp && !duplicate && !compare(**this, **pp))
          return std::make_pair(this, false);

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

      return std::make_pair(this, true);
    }

    /// erase this entry from this list
    void erase() {
      if(m_prev)
        m_prev->m_next = m_next;
      if(m_next)
        m_next->m_prev = m_prev;

      m_prev = nullptr;
      m_next = nullptr;
    }

    /// Swap two list nodes
    void swap(list_value_type &rhs) {
      splice_prev(rhs);
      splice_next(rhs);
    }

    void splice_prev(list_value_type &rhs) {
      assert(m_offset == rhs.m_offset);

      if(m_prev)
        m_prev->m_next = &rhs;
      std::swap(m_prev, rhs.m_prev);
      if(m_prev)
        m_prev->m_next = this;
    }

    void splice_next(list_value_type &rhs) {
      assert(m_offset == rhs.m_offset);

      if(m_next)
        m_next->m_prev = &rhs;
      std::swap(m_next, rhs.m_next);
      if(m_next)
        m_next->m_prev = this;
    }

    /// delete every entry in the list between begin() and end(), inclusive
    void destroy() {
      if(this) {
        auto it = begin();
        auto iend = end();
        while(it != iend) {
          auto ptr = it.get();
          ++it;
          delete ptr;
        }
      }
    }

    Linked_List<TYPE> * clone() const {
      Linked_List<TYPE> * head = nullptr;

      if(this) {
        auto it = begin();
        auto iend = end();

        if(it != iend) {
          head = reinterpret_cast<list_pointer_type>(reinterpret_cast<char *>(it->clone()) + m_offset);
          auto tail = head;
          ++it;

          while(it != iend) {
            auto ptr = reinterpret_cast<list_pointer_type>(reinterpret_cast<char *>(it->clone()) + m_offset);
            ptr->insert_after(tail);
            tail = ptr;
            ++it;
          }
        }
      }

      return head;
    }

    bool operator<(const Linked_List<TYPE> &rhs) const {return compare(rhs) < 0;}
    bool operator<=(const Linked_List<TYPE> &rhs) const {return compare(rhs) <= 0;}
    bool operator>(const Linked_List<TYPE> &rhs) const {return compare(rhs) > 0;}
    bool operator>=(const Linked_List<TYPE> &rhs) const {return compare(rhs) >= 0;}
    bool operator==(const Linked_List<TYPE> &rhs) const {return compare(rhs) == 0;}
    bool operator!=(const Linked_List<TYPE> &rhs) const {return compare(rhs) != 0;}

    int compare(const Linked_List<TYPE> &rhs) const {
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

  private:
    size_t m_offset;
    list_pointer_type m_prev;
    list_pointer_type m_next;
  };

}

#endif
