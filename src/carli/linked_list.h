#ifndef ZENI_LINKED_LIST_H
#define ZENI_LINKED_LIST_H

#include <cassert>
#include <algorithm>
#include <functional>

namespace Zeni {

  template <typename TYPE>
  class Linked_List;

  template <typename TYPE>
  Linked_List<const TYPE> * value_to_Linked_List(const TYPE * const type, const size_t &offset);

  template <typename TYPE>
  Linked_List<TYPE> * value_to_Linked_List(TYPE * const type, const size_t &offset);

  template <typename TYPE>
  typename Linked_List<TYPE>::iterator_const begin(const TYPE * const type, const size_t &offset);

  template <typename TYPE>
  typename Linked_List<TYPE>::iterator begin(TYPE * const type, const size_t &offset);

  template <typename TYPE>
  class Linked_List {
  public:
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef value_type & value_reference_type;
    typedef const Linked_List<TYPE> const_list_value_type;
    typedef Linked_List<TYPE> list_value_type;
    typedef const_list_value_type * const_list_pointer_type;
    typedef list_value_type * list_pointer_type;
    typedef std::less<value_type> compare_default;

    class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef TYPE * pointer;
      typedef TYPE & reference;

      iterator()
        : m_offset(0),
        m_pointer(nullptr)
      {
      }
      
      iterator(const size_t &m_offset_)
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
          return iterator(m_pointer->m_prev->get());
        else
          return iterator();
      }
      iterator next() const {
        if(m_pointer->m_next)
          return iterator(m_pointer->m_prev->get());
        else
          return iterator();
      }
      
      iterator operator--() {
        m_pointer = m_pointer->m_prev;
        return *this;
      }
      iterator operator--(int) {
        iterator rv(*this);
        m_pointer = m_pointer->m_prev;
        return rv;
      }
      iterator operator++() {
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
      size_t m_offset;
      list_pointer_type m_pointer;
    };

    class iterator_const : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef TYPE * pointer;
      typedef TYPE & reference;

      typedef const value_type * value_pointer_type;
      typedef const value_type & value_reference_type;
      typedef const list_value_type * list_pointer_type;

      iterator_const()
        : m_offset(0),
        m_pointer(nullptr)
      {
      }
      
      iterator_const(const size_t &m_offset_)
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
          return iterator_const(m_pointer->m_prev->get());
        else
          return iterator_const();
      }
      iterator_const next() const {
        if(m_pointer->m_next)
          return iterator_const(m_pointer->m_prev->get());
        else
          return iterator_const();
      }
      
      iterator_const operator--() {
        m_pointer = m_pointer->m_prev;
        return *this;
      }
      iterator_const operator--(int) {
        iterator_const rv(*this);
        m_pointer = m_pointer->m_prev;
        return rv;
      }
      iterator_const operator++() {
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
      size_t m_offset;
      list_pointer_type m_pointer;
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
    value_pointer_type operator->() const {
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
    template <typename DERIVED>
    static iterator_const begin(const DERIVED * const &ptr) {
      return ptr ? iterator_const(ptr) : iterator_const();
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    template <typename DERIVED>
    static iterator begin(DERIVED * const &ptr) {
      return ptr ? iterator(ptr) : iterator();
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    template <typename DERIVED>
    static iterator_const end(const DERIVED * const &ptr) {
      return ptr ? iterator_const(ptr->offset()) : iterator_const();
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    template <typename DERIVED>
    static iterator end(DERIVED * const &ptr) {
      return ptr ? iterator(ptr->offset()) : iterator();
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

    template <typename COMPARE>
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

    template <typename COMPARE>
    list_pointer_type find(const value_type &value, const COMPARE &compare = COMPARE(), list_pointer_type * const &pptr = nullptr) {
      const list_pointer_type pp = find_gte(value, compare, pptr);

      if(pp && !compare(value, **pp))
        return pp;

      return nullptr;
    }

    /** insert this list entry into the list; requires this to have !prev() && !next()
     *  return same pointer if inserted, different pointer if already exists, duplicate == false, and deleted
     */ 
    template <typename COMPARE>
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
    void erase() {
      if(m_prev)
        m_prev->m_next = m_next;
      if(m_next)
        m_next->m_prev = m_prev;

      m_prev = nullptr;
      m_next = nullptr;
    }

    void erase_prev() {
      if(m_prev)
        m_prev->m_next = m_next;

      m_prev = nullptr;
    }

    void erase_next() {
      if(m_next)
        m_next->m_prev = m_prev;

      m_next = nullptr;
    }

    /// delete every entry in the list between begin() and end(), inclusive
    template <typename DERIVED>
    static void destroy(DERIVED * const &ptr_) {
      auto ptr = ptr_;
      destroy(ptr);
    }
    template <typename DERIVED>
    static void destroy(DERIVED * &ptr_) {
      auto it = begin(ptr_);
      auto iend = end(ptr_);
      while(it != iend) {
        auto ptr = it.get();
        ++it;
        delete ptr;
      }
      ptr_ = nullptr;
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

  template <typename TYPE>
  Linked_List<const TYPE> * value_to_Linked_List(const TYPE * const type, const size_t &offset) {
    return type ? reinterpret_cast<Linked_List<const TYPE> *>(reinterpret_cast<char *>(type) + offset) : nullptr;
  }

  template <typename TYPE>
  Linked_List<TYPE> * value_to_Linked_List(TYPE * const type, const size_t &offset) {
    return type ? reinterpret_cast<Linked_List<TYPE> *>(reinterpret_cast<char *>(type) + offset) : nullptr;
  }

  template <typename TYPE>
  typename Linked_List<TYPE>::iterator_const begin(const TYPE * const type, const size_t &offset) {
    auto list = value_to_Linked_List(type, offset);
    return list->begin(list);
  }

  template <typename TYPE>
  typename Linked_List<TYPE>::iterator begin(TYPE * const type, const size_t &offset) {
    auto list = value_to_Linked_List(type, offset);
    return list->begin(list);
  }

}

#endif
