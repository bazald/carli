#ifndef ZENI_SHARED_SUFFIX_LIST_H
#define ZENI_SHARED_SUFFIX_LIST_H

#include <cstddef>
#include <algorithm>
#include <functional>
#include <memory>
#include <inttypes.h>

namespace Zeni {

  template <typename TYPE>
  class Shared_Suffix_List : public std::enable_shared_from_this<Shared_Suffix_List<TYPE>> {
  public:
    typedef TYPE value_type;
    typedef const TYPE * const_value_pointer_type;
    typedef TYPE * value_pointer_type;
    typedef const value_type & const_value_reference_type;
    typedef value_type & value_reference_type;
    typedef const Shared_Suffix_List<TYPE> const_list_value_type;
    typedef Shared_Suffix_List<TYPE> list_value_type;
    typedef std::shared_ptr<const_list_value_type> const_list_pointer_type;
    typedef std::shared_ptr<list_value_type> list_pointer_type;
    typedef std::less<value_type> compare_default;

    class iterator : public std::iterator<std::forward_iterator_tag, value_type> {
    public:
      iterator()
        : m_pointer(nullptr)
      {
      }

      iterator(const list_pointer_type &ptr)
        : m_pointer(ptr)
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

      iterator next() const {
        return iterator(m_pointer->m_next);
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
        return m_pointer == rhs.m_pointer;
      }
      bool operator!=(const typename Linked_List<value_type>::iterator &rhs) const {
        return m_pointer != rhs.m_pointer;
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
      list_pointer_type m_pointer;
    };

    class iterator_const : public std::iterator<std::forward_iterator_tag, value_type> {
    public:
      iterator_const()
        : m_pointer(nullptr)
      {
      }

      iterator_const(const const_list_pointer_type &ptr)
        : m_pointer(ptr)
      {
      }

      iterator_const(const iterator &rhs)
        : m_pointer(rhs.m_pointer)
      {
      }

      iterator_const & operator=(const iterator &rhs) {
        m_pointer = rhs.m_pointer;
        return *this;
      }

      const_value_pointer_type get() const {
        return m_pointer->get();
      }
      const_value_reference_type operator*() const {
        return *get();
      }
      const_value_pointer_type operator->() const {
        return get();
      }

      iterator_const next() const {
        return iterator_const(m_pointer->m_next);
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
        return m_pointer == rhs.m_pointer;
      }
      bool operator!=(const typename Linked_List<value_type>::iterator_const &rhs) const {
        return m_pointer != rhs.m_pointer;
      }
      bool operator==(const const_list_pointer_type &rhs) const {
        return m_pointer == rhs;
      }
      bool operator!=(const const_list_pointer_type &rhs) const {
        return m_pointer != rhs;
      }

      operator const_list_pointer_type () const {
        return m_pointer;
      }
      operator const_list_pointer_type & () {
        return m_pointer;
      }

    private:
      const_list_pointer_type m_pointer;
    };

    Shared_Suffix_List() : m_value(TYPE()) {}
    Shared_Suffix_List(const TYPE &value_) : m_value(value_) {}
    Shared_Suffix_List(TYPE &&value_) : m_value(std::move(value_)) {}
    Shared_Suffix_List(const TYPE &value_, const list_pointer_type &next_) : m_value(value_), m_next(next_) {}
    Shared_Suffix_List(const TYPE &value_, list_pointer_type &&next_) : m_value(value_), m_next(std::move(next_)) {}
    Shared_Suffix_List(TYPE &&value_, const list_pointer_type &next_) : m_value(std::move(value_)), m_next(next_) {}
    Shared_Suffix_List(TYPE &&value_, list_pointer_type &&next_) : m_value(std::move(value_)), m_next(std::move(next_)) {}

    const_value_pointer_type get() const {
      return &m_value;
    }
    value_pointer_type get() {
      return &m_value;
    }
    const_value_reference_type operator*() const {
      return m_value;
    }
    value_reference_type operator*() {
      return m_value;
    }
    value_pointer_type operator->() const {
      return &m_value;
    }
    value_pointer_type operator->() {
      return &m_value;
    }

    list_pointer_type next() const {
      return m_next;
    }

    /// return an iterator_const pointing to this list entry; only the beginning if !prev()
    iterator_const begin() const {
      return iterator_const(shared_from_this());
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    iterator begin() {
      return iterator(shared_from_this());
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    iterator_const end() const {
      return iterator_const();
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    iterator end() {
      return iterator();
    }

    bool operator<(const_list_value_type &rhs) const { return compare(rhs) < 0; }
    bool operator<=(const_list_value_type &rhs) const { return compare(rhs) <= 0; }
    bool operator>(const_list_value_type &rhs) const { return compare(rhs) > 0; }
    bool operator>=(const_list_value_type &rhs) const { return compare(rhs) >= 0; }
    bool operator==(const_list_value_type &rhs) const { return compare(rhs) == 0; }
    bool operator!=(const_list_value_type &rhs) const { return compare(rhs) != 0; }

    int64_t compare(const_list_value_type &rhs) const {
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
    TYPE m_value;
    list_pointer_type m_next;
  };

}

namespace std {
  template <typename TYPE>
  struct hash<Zeni::Shared_Suffix_List<TYPE>> {
    size_t operator()(const Zeni::Shared_Suffix_List<TYPE> &ssl) const {
      size_t h = std::hash<TYPE>()(*ssl);
      for(auto st = ssl.next(); st; st = st->next()) {
        h = Rete::hash_combine(h, std::hash<TYPE>()(**st));
      }
      return h;
    }
  };
}

#endif
