#ifndef ZENI_LINKED_LIST_H
#define ZENI_LINKED_LIST_H

#include <cassert>

namespace Zeni {

  template<typename TYPE>
  class Linked_List {
  public:
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef value_type & value_reference_type;
    typedef const Linked_List<value_type> const_list_value_type;
    typedef Linked_List<value_type> list_value_type;
    typedef const_list_value_type * const_list_pointer_type;
    typedef list_value_type * list_pointer_type;

    class iterator {
    public:
      iterator()
        : offset(0),
        pointer(0)
      {
      }
      
      iterator(const size_t &offset_)
        : offset(offset_),
        pointer(0)
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

      operator bool () const {
        return offset != 0 || pointer != 0;
      }

    private:
      size_t offset;
      list_pointer_type pointer;
    };

    Linked_List(value_pointer_type value)
      : m_offset(reinterpret_cast<char *>(this) - reinterpret_cast<char *>(value)),
      m_prev(0),
      m_next(0)
    {
      assert(m_offset < sizeof(value_type));
    }

    value_pointer_type get() const {
      return reinterpret_cast<value_pointer_type>((reinterpret_cast<char *>(const_cast<Zeni::Linked_List<value_type> *>(this)) - m_offset));
    }

    Linked_List * prev() const {
      return m_prev;
    }
    Linked_List * next() const {
      return m_next;
    }
    
    iterator begin() {
      return iterator(this);
    }
    iterator end() {
      return iterator(m_offset);
    }

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
    void insert_before(const list_pointer_type &ptr) {
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
    }

    void erase() {
      if(m_prev)
        m_prev->m_next = m_next;
      if(m_next)
        m_next->m_prev = m_prev;

      m_prev = 0;
      m_next = 0;
    }

  private:
    size_t m_offset;
    list_pointer_type m_prev;
    list_pointer_type m_next;
  };

}

#endif
