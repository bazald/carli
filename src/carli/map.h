#ifndef ZENI_MAP_H
#define ZENI_MAP_H

#include "linked_list.h"
#include <stdexcept>

namespace Zeni {

  template <typename KEY, typename TYPE, typename COMPARE = std::less<KEY>>
  class Map : private Linked_List<TYPE> {
  public:
    Map(const Map &) = delete;
    Map operator=(const Map &) = delete;

    typedef KEY key_type;
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef value_type & value_reference_type;
    typedef Linked_List<TYPE> list_value_type;
    typedef list_value_type * list_pointer_type;
    typedef Map<KEY, TYPE, COMPARE> map_value_type;
    typedef map_value_type * map_pointer_type;

    value_pointer_type get() const {return list_value_type::get();}
    const value_reference_type operator*() const {return list_value_type::operator*();}
    value_reference_type operator*() {return list_value_type::operator*();}
    value_pointer_type operator->() const {return list_value_type::operator->();}
    value_pointer_type operator->() {return list_value_type::operator->();}
    size_t offset() const {return list_value_type::offset();}

    map_pointer_type list_prev() const {
      return static_cast<map_pointer_type>(list_value_type::prev());
    }
    map_pointer_type list_next() const {
      return static_cast<map_pointer_type>(list_value_type::next());
    }
    void list_insert_after(const map_pointer_type &ptr) {
      list_value_type::insert_after(ptr);
    }
    void list_insert_before(map_pointer_type &ptr) {
      auto lp = static_cast<list_pointer_type>(ptr);
      list_value_type::insert_before(lp);
      ptr = static_cast<map_pointer_type>(lp);
    }
    void list_destroy(const map_pointer_type &ptr_) {
      map_pointer_type ptr = ptr_;
      list_destroy(ptr);
    }
    void list_destroy(map_pointer_type &ptr) {
      auto lp = static_cast<list_pointer_type>(ptr);
      list_value_type::destroy(lp);
      ptr = static_cast<map_pointer_type>(lp);
    }
    void list_erase() {
      list_value_type::erase();
    }
    void list_erase_hard() {
      list_value_type::erase_hard();
    }

    class iterator : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef TYPE * pointer;
      typedef TYPE & reference;

      iterator(const size_t &m_offset_ = 0lu)
        : m_offset(m_offset_),
        m_pointer(nullptr)
      {
      }

      iterator(const map_pointer_type &ptr)
        : m_offset(ptr->offset()),
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
        map_pointer_type * const ptr = m_pointer->prev();
        return ptr ? iterator(ptr) : iterator(m_offset);
      }
      iterator next() const {
        map_pointer_type * const ptr = m_pointer->next();
        return ptr ? iterator(ptr) : iterator(m_offset);
      }

      iterator & operator--() {
        m_pointer = m_pointer->prev();
        return *this;
      }
      iterator operator--(int) {
        iterator rv(*this);
        m_pointer = m_pointer->prev();
        return rv;
      }
      iterator & operator++() {
        m_pointer = m_pointer->next();
        return *this;
      }
      iterator operator++(int) {
        iterator rv(*this);
        m_pointer = m_pointer->next();
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
      bool operator==(const map_pointer_type &rhs) const {
        return m_pointer == rhs;
      }
      bool operator!=(const map_pointer_type &rhs) const {
        return m_pointer != rhs;
      }

      operator map_pointer_type () const {
        return m_pointer;
      }
      operator map_pointer_type & () {
        return m_pointer;
      }

    private:
      size_t m_offset;
      map_pointer_type m_pointer;
    };

    class iterator_const : public std::iterator<std::bidirectional_iterator_tag, value_type> {
    public:
      typedef TYPE * pointer;
      typedef TYPE & reference;

      typedef const value_type * value_pointer_type;
      typedef const value_type & value_reference_type;
      typedef const map_value_type * map_pointer_type;

      iterator_const(const size_t &m_offset_ = 0lu)
        : m_offset(m_offset_),
        m_pointer(nullptr)
      {
      }

      iterator_const(const map_pointer_type &ptr)
        : m_offset(ptr->offset()),
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
        map_pointer_type * const ptr = m_pointer->prev();
        return ptr ? iterator_const(ptr) : iterator_const(m_offset);
      }
      iterator_const next() const {
        map_pointer_type * const ptr = m_pointer->next();
        return ptr ? iterator_const(ptr) : iterator_const(m_offset);
      }

      iterator_const & operator--() {
        m_pointer = m_pointer->prev();
        return *this;
      }
      iterator_const operator--(int) {
        iterator_const rv(*this);
        m_pointer = m_pointer->prev();
        return rv;
      }
      iterator_const & operator++() {
        m_pointer = m_pointer->next();
        return *this;
      }
      iterator_const operator++(int) {
        iterator_const rv(*this);
        m_pointer = m_pointer->next();
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
      bool operator==(const map_pointer_type &rhs) const {
        return m_pointer == rhs;
      }
      bool operator!=(const map_pointer_type &rhs) const {
        return m_pointer != rhs;
      }

      operator map_pointer_type () const {
        return m_pointer;
      }
      operator map_pointer_type & () {
        return m_pointer;
      }

    private:
      size_t m_offset;
      map_pointer_type m_pointer;
    };

    Map(value_pointer_type value, const key_type &key_ = key_type())
     : Linked_List<TYPE>(value),
     key(key_)
    {
      m_parent = nullptr;
    }

    Map(value_pointer_type value, key_type &&key_ = key_type())
     : Linked_List<TYPE>(value),
     key(std::forward<key_type>(key_))
    {
      m_parent = nullptr;
    }

    std::function<bool (const TYPE &, const TYPE &)> comparator() const {
      return [this](const TYPE &lhs, const TYPE &rhs)->bool{
        const Map<KEY, TYPE, COMPARE> * const lm = reinterpret_cast<const Map<KEY, TYPE, COMPARE> *>(reinterpret_cast<const char *>(&lhs) + this->offset());
        const Map<KEY, TYPE, COMPARE> * const rm = reinterpret_cast<const Map<KEY, TYPE, COMPARE> *>(reinterpret_cast<const char *>(&rhs) + this->offset());

        return COMPARE()(lm->key, rm->key);
      };
    }

    void insert_into(map_pointer_type &root) {
      assert(!m_parent);
      assert(!get_left());
      assert(!get_right());

      insert_case0(root, root, nullptr);

//      assert(root->verify(nullptr));
    }

    map_pointer_type insert_into_unique(map_pointer_type &root) {
      assert(!m_parent);
      assert(!get_left());
      assert(!get_right());

      const map_pointer_type rv = insert_case0_unique(root, root, nullptr);

//      assert(root->verify(nullptr));

      return rv;
    }

    void remove_from(map_pointer_type &root) {
      assert(this);
      if(!this)
        throw::std::runtime_error("Attempt to remove null map_pointer_type from AVL tree.");

      assert(!root->get_parent());

      if(get_left())
        swap_remove_rightmost(get_left(), root);
      else if(get_right())
        swap_remove_leftmost(get_right(), root);
      else {
        remove_child(root, nullptr);

//        assert(root->verify(nullptr));
      }
    }

    template <typename TYPE2, typename COMPARE2 = COMPARE>
    map_pointer_type find(const TYPE2 &key_, const COMPARE2 &compare = COMPARE2()) {
      if(this) {
        if(compare(key_, key))
          return get_left()->find(key_, compare);
        else if(compare(key, key_))
          return get_right()->find(key_, compare);
        else
          return this;
      }
      else
        return nullptr;
    }

    const key_type & get_key() const {
      return key;
    }

    void set_key(const key_type &key_) {
      if(get_left() && COMPARE()(key_, static_cast<map_pointer_type>(get_left())->key))
        throw std::runtime_error("Illegal key modification in Zeni::Map::set_key.");
      if(get_right() && COMPARE()(static_cast<map_pointer_type>(get_right())->key, key_))
        throw std::runtime_error("Illegal key modification in Zeni::Map::set_key.");
      key = key_;
    }

    void set_key(key_type &&key_) {
      if(get_left() && COMPARE()(key_, static_cast<map_pointer_type>(get_left())->key))
        throw std::runtime_error("Illegal key modification in Zeni::Map::set_key.");
      if(get_right() && COMPARE()(static_cast<map_pointer_type>(get_right())->key, key_))
        throw std::runtime_error("Illegal key modification in Zeni::Map::set_key.");
      key = std::forward(key_);
    }

    map_pointer_type prev() const {
      if(get_left())
        return get_left();
      else if(!get_parent())
        return nullptr;
      else {
        map_pointer_type ptr = const_cast<map_pointer_type>(this);
        map_pointer_type parent = ptr->get_parent();

        while(parent && parent->get_left() == ptr) {
          ptr = parent;
          parent = parent->get_parent();
        }

        return parent;
      }
    }

    map_pointer_type next() const {
      if(get_right())
        return get_right();
      else if(!get_parent())
        return nullptr;
      else {
        map_pointer_type ptr = const_cast<map_pointer_type>(this);
        map_pointer_type parent = ptr->get_parent();

        while(parent && parent->get_right() == ptr) {
          ptr = parent;
          parent = parent->get_parent();
        }

        return parent;
      }
    }

    /// return an iterator_const pointing to this list entry; only the beginning if !prev()
    iterator_const begin() const {
      return this ? iterator_const(first()) : iterator_const();
    }
    /// return an iterator pointing to this list entry; only the beginning if !prev()
    iterator begin() {
      return this ? iterator(first()) : iterator();
    }
    /// return an iterator_const pointing to an empty list entry of the appropriate size
    iterator_const end() const {
      return this ? iterator_const(offset()) : iterator_const();
    }
    /// return an iterator pointing to an empty list entry of the appropriate size
    iterator end() {
      return this ? iterator(offset()) : iterator();
    }

    map_pointer_type first() const {
      map_pointer_type ptr = const_cast<map_pointer_type>(this);
      if(ptr) {
        while(ptr->get_left())
          ptr = ptr->get_left();
      }
      return ptr;
    }

    static void destroy(const map_pointer_type &ptr_) {
      map_pointer_type ptr = ptr_;
      destroy(ptr);
    }
    static void destroy(map_pointer_type &ptr_) {
      if(ptr_) {
        assert(!ptr_->get_left() || ptr_->get_left()->get_right() != ptr_);
        assert(!ptr_->get_right() || ptr_->get_right()->get_left() != ptr_);

        destroy(ptr_->get_left());
        map_pointer_type right = ptr_->get_right();
        delete ptr_->get();
        ptr_ = nullptr;
        destroy(right);
      }
    }

#ifndef NDEBUG
    size_t debug_height() const {
      return this ? std::max(get_left()->debug_height(), get_right()->debug_height()) + 1 : 0;
    }

    size_t debug_size() const {
      return this ? get_left()->debug_size() + get_right()->debug_size() + 1 : 0;
    }

//    std::ostream & debug_print(std::ostream &os) const {
//      /// vt100 stuff
//      const char * const white_on_red = "\033[37;41m";
//      const char * const black_on_white = "\033[30;47m";
//      const char * const default_on_default = "\033[39;49m";
//
//      if(this) {
//        os << (is_red() ? white_on_red : black_on_white) << *this;
//        if(get_left() || get_right()) {
//          os << default_on_default << '(';
//          get_left()->debug_print(os);
//          os << default_on_default << ':';
//          get_right()->debug_print(os);
//          os << default_on_default << ')';
//        }
//        else
//          os << default_on_default;
//      }
//
//      return os;
//    }
#endif

  private:
    void insert_case0(map_pointer_type &node, map_pointer_type &root, map_pointer_type const parent) {
      if(node) {
        if(COMPARE()(key, node->key))
          insert_case0(node->get_left(), root, node);
        else
          insert_case0(node->get_right(), root, node);
      }
      else {
        node = this;
        m_parent = parent;
        insert_case1(root, parent);
      }

//      assert(root->verify(nullptr));
    }

    map_pointer_type insert_case0_unique(map_pointer_type &node, map_pointer_type &root, map_pointer_type const parent) {
      if(node) {
        if(COMPARE()(key, node->key))
          return insert_case0_unique(node->get_left(), root, node);
        else if(COMPARE()(node->key, key))
          return insert_case0_unique(node->get_right(), root, node);
        else {
          destroy(this);
          return node;
        }
      }
      else {
        node = this;
        m_parent = parent;
        insert_case1(root, parent);
        return this;
      }
    }

    void insert_case1(map_pointer_type &root, map_pointer_type const parent) {
      if(parent)
        insert_case2(root, parent);
      else
        set_black();

//      assert(root->verify(nullptr));
    }

    void insert_case2(map_pointer_type &root, map_pointer_type const parent) {
      if(parent->is_red())
        insert_case3(root, parent);

//      assert(root->verify(nullptr));
    }

    void insert_case3(map_pointer_type &root, map_pointer_type const parent) {
      assert(is_red());
      assert(parent->is_red());

      map_pointer_type const gp = parent->get_parent();
      map_pointer_type const un = uncle(parent, gp);

      if(un->is_red()) {
        parent->set_black();
        un->set_black();
        gp->set_red();
        gp->insert_case1(root, gp->get_parent());
      }
      else
        insert_case4(root, parent, gp);

//      assert(root->verify(nullptr));
    }

    void insert_case4(map_pointer_type &root, map_pointer_type const parent, map_pointer_type const gp) {
      if(parent->get_right() == this && parent == gp->get_left()) {
        parent->rotate_left(root);
        get_left()->insert_case5(root);
      }
      else if(parent->get_left() == this && parent == gp->get_right()) {
        parent->rotate_right(root);
        get_right()->insert_case5(root);
      }
      else
        insert_case5(root);

//      assert(root->verify(nullptr));
    }

    void insert_case5(map_pointer_type &root) {
      map_pointer_type const parent = get_parent();

      assert(is_red());
      assert(parent->is_red());

      map_pointer_type const gp = parent->get_parent();

      parent->set_black();
      gp->set_red();
      if(parent->get_left() == this)
        gp->rotate_right(root);
      else
        gp->rotate_left(root);

//      assert(root->verify(nullptr));
    }

    void swap_remove_leftmost(map_pointer_type const node, map_pointer_type &root) {
      if(node->get_left())
        swap_remove_leftmost(node->get_left(), root);
      else
        swap_remove_child(node, root, node->get_right());
    }

    void swap_remove_rightmost(map_pointer_type const node, map_pointer_type &root) {
      if(node->get_right())
        swap_remove_rightmost(node->get_right(), root);
      else
        swap_remove_child(node, root, node->get_left());
    }

    void swap_remove_child(map_pointer_type const node, map_pointer_type &root, map_pointer_type const child) {
//      assert(root->verify(nullptr));

      swap_nodes(node, root);

//      assert(root->verify(nullptr));

      remove_child(root, child);
    }

    void remove_child(map_pointer_type &root, map_pointer_type const child) {
      map_pointer_type const parent = get_parent();

      /// Rebalance
      if(child) {
        disconnect_one_child(parent);

        if(is_black()) {
          if(child->is_black())
            rebalance(root);
          else
            child->set_black();
        }
      }
      else if(root != this) {
        if(is_black())
          rebalance(root);

        disconnect_one_child(parent ? parent : root);
      }
      else {
        root = nullptr;
        return;
      }

#ifndef NDEBUG
//      root->debug_print(std::cerr) << std::endl;

//      assert(root->verify(nullptr));
#endif
    }

    void rebalance(map_pointer_type &root) {
      delete_case1(root);

//#ifndef NDEBUG
//      root->debug_print(std::cerr) << std::endl;
//#endif
    }

    void disconnect_one_child(map_pointer_type const parent) {
      if(parent) {
        if(parent->get_left() == this) {
          parent->set_left(get_left() ? get_left() : get_right());
          if(parent->get_left())
            parent->get_left()->set_parent(parent);
        }
        else {
          assert(parent->get_right() == this);
          parent->set_right(get_left() ? get_left() : get_right());
          if(parent->get_right())
            parent->get_right()->set_parent(parent);
        }
      }
    }

    void delete_case1(map_pointer_type &root) {
      map_pointer_type const parent = get_parent();

      if(parent)
        delete_case2(root, parent);
      else {
#ifndef NDEBUG
        std::cerr << "case1" << std::endl;
#endif
      }
    }

    void delete_case2(map_pointer_type &root, map_pointer_type const parent) {
      map_pointer_type si = sibling(parent);

      if(si) {
        if(parent->is_black() &&
           si->is_red())
        {
#ifndef NDEBUG
          std::cerr << "case2" << std::endl;
#endif
          parent->set_red();
          si->set_black();
          if(parent->get_left() == si)
            parent->rotate_right(root);
          else
            parent->rotate_left(root);
        }

        delete_case3(root);
      }
    }

    void delete_case3(map_pointer_type &root) {
      map_pointer_type const parent = get_parent();
      map_pointer_type const si = sibling(parent);

      if(parent->is_black() &&
         si->is_black() &&
         si->get_left()->is_black() &&
         si->get_right()->is_black())
      {
#ifndef NDEBUG
        std::cerr << "case3" << std::endl;
#endif
        si->set_red();
//#ifndef NDEBUG
//        root->debug_print(std::cerr) << std::endl;
//#endif

        parent->delete_case1(root);
      }
      else
        delete_case4(root);
    }

    void delete_case4(map_pointer_type &root) {
      map_pointer_type const parent = get_parent();
      map_pointer_type const si = sibling(parent);

      if(parent->is_red() &&
         si->is_black() &&
         si->get_left()->is_black() &&
         si->get_right()->is_black())
      {
#ifndef NDEBUG
        std::cerr << "case4" << std::endl;
#endif
        parent->set_black();
        si->set_red();
      }
      else
        delete_case5(root, parent, si);
    }

    void delete_case5(map_pointer_type &root, map_pointer_type const parent, map_pointer_type const si) {
      if(si->is_black()) {
        if(this == parent->get_left() &&
           si->get_right()->is_black() &&
           si->get_left()->is_red())
        {
#ifndef NDEBUG
          std::cerr << "case5a" << std::endl;
#endif
          si->set_red();
          si->get_left()->set_black();
          si->rotate_right(root);
        }
        else if(this == parent->get_right() &&
                si->get_left()->is_black() &&
                si->get_right()->is_red())
        {
#ifndef NDEBUG
          std::cerr << "case5b" << std::endl;
#endif
          si->set_red();
          si->get_right()->set_black();
          si->rotate_left(root);
        }
      }

      delete_case6(root);
    }

    void delete_case6(map_pointer_type &root) {
#ifndef NDEBUG
      std::cerr << "case6" << std::endl;
#endif

      map_pointer_type const parent = get_parent();
      map_pointer_type const si = sibling(parent);

      si->set_color(parent->get_color());
      parent->set_black();

      if(parent->get_left() == si) {
        if(si->get_left())
          si->get_left()->set_black();
        parent->rotate_right(root);
      }
      else {
        if(si->get_right())
          si->get_right()->set_black();
        parent->rotate_left(root);
      }
    }

    void swap_nodes(map_pointer_type const node, map_pointer_type &root) {
      if(node->get_parent() != this) {
        std::swap(m_parent, node->m_parent);
        std::swap(get_left(), node->get_left());
        std::swap(get_right(), node->get_right());

        fixup_post_swap(root, node);
        node->fixup_post_swap(root, this);
      }
      else {
        map_pointer_type const parent = get_parent();
        const uintptr_t node_color = node->get_color();

        if(parent) {
          if(parent->get_left() == this)
            parent->set_left(node);
          else {
            assert(parent->get_right() == this);
            parent->set_right(node);
          }
        }
        else
          root = node;
        node->m_parent = m_parent;
        set_parent(node);
        set_color(node_color);

        map_pointer_type const child = node->get_left() ? node->get_left() : node->get_right();
        if(child)
          child->set_parent(this);

        if(get_left() == node) {
          node->set_left(this);
          node->set_right(get_right());
          if(get_right())
            get_right()->set_parent(node);
          set_left(child);
          set_right(nullptr);
        }
        else {
          assert(get_right() == node);
          node->set_left(get_left());
          node->set_right(this);
          if(get_left())
            get_right()->set_parent(node);
          set_left(nullptr);
          set_right(child);
        }
      }

//#ifndef NDEBUG
//      root->debug_print(std::cerr) << std::endl;
//#endif
    }

    void fixup_post_swap(map_pointer_type & root, map_pointer_type const prev) {
      map_pointer_type const parent = get_parent();

      if(parent) {
        if(parent->get_left() == prev)
          parent->set_left(this);
        else {
          assert(parent->get_right() == prev);
          parent->set_right(this);
        }
      }
      else
        root = this;

      if(get_left())
        get_left()->set_parent(this);

      if(get_right())
        get_right()->set_parent(this);
    }

    void rotate_left(map_pointer_type &root) {
      map_pointer_type const parent = get_parent();
      map_pointer_type const child = get_right();

      set_right(child->get_left());
      if(child->get_left())
        child->get_left()->set_parent(this);

      child->set_parent(parent);
      child->set_left(this);
      if(parent) {
        if(parent->get_left() == this)
          parent->set_left(child);
        else {
          assert(parent->get_right() == this);
          parent->set_right(child);
        }
      }
      else
        root = child;

      set_parent(child);

//#ifndef NDEBUG
//      root->debug_print(std::cerr) << std::endl;
//#endif
    }

    void rotate_right(map_pointer_type &root) {
      map_pointer_type const parent = get_parent();
      map_pointer_type const child = get_left();

      set_left(child->get_right());
      if(child->get_right())
        child->get_right()->set_parent(this);

      child->set_parent(parent);
      child->set_right(this);
      if(parent) {
        if(parent->get_left() == this)
          parent->set_left(child);
        else {
          assert(parent->get_right() == this);
          parent->set_right(child);
        }
      }
      else
        root = child;

      set_parent(child);

//#ifndef NDEBUG
//      root->debug_print(std::cerr) << std::endl;
//#endif
    }

    map_pointer_type sibling(map_pointer_type const parent) {
      return parent->get_right() == this ? parent->get_left() : parent->get_right();
    }

    map_pointer_type uncle(map_pointer_type const parent, map_pointer_type const gp) {
      return gp ? (gp->get_right() == parent ? gp->get_left() : gp->get_right()) : nullptr;
    }

    map_pointer_type get_left() const {
      return reinterpret_cast<map_pointer_type>(list_value_type::prev());
    }
    map_pointer_type & get_left() {
      return reinterpret_cast<map_pointer_type &>(list_value_type::get_prev());
    }

    void set_left(const map_pointer_type &left_) {
      return list_value_type::set_prev(left_);
    }

    map_pointer_type get_right() const {
      return reinterpret_cast<map_pointer_type>(list_value_type::next());
    }
    map_pointer_type & get_right() {
      return reinterpret_cast<map_pointer_type &>(list_value_type::get_next());
    }

    void set_right(const map_pointer_type &right_) {
      return list_value_type::set_next(right_);
    }

    const Map<KEY, TYPE, COMPARE> * get_parent() const {
      return reinterpret_cast<map_pointer_type>(uintptr_t(m_parent) & get_mask());
    }

    map_pointer_type get_parent() {
      return reinterpret_cast<map_pointer_type>(uintptr_t(m_parent) & get_mask());
    }

    void set_parent(map_pointer_type const parent) {
      m_parent = reinterpret_cast<map_pointer_type>(uintptr_t(parent) | get_color());
    }

    uintptr_t get_color() const {
      return this ? m_color_bits & 1 : 1;
    }

    bool is_black() const {
      return !this || m_color_bits & 1;
    }

    bool is_red() const {
      return this && !(m_color_bits & 1);
    }

    void set_color(const uintptr_t color) {
      m_color_bits &= get_mask();
      m_color_bits |= color;
    }

    void set_black() {
      m_color_bits |= 1;
    }

    void set_red() {
      m_color_bits &= get_mask();
    }

    constexpr static uintptr_t get_mask() {
      return uintptr_t(-2);
    }

//#ifndef NDEBUG
//    int verify(map_pointer_type const removal_node) const {
//      if(this) {
//        const auto parent = get_parent();
//
//        assert(parent != this);
//        assert(get_left() != this);
//        assert(get_right() != this);
//        assert(!parent || parent->get_left() == this || parent->get_right() == this);
//        assert(parent || is_black());
//        assert(!parent || parent->is_black() || is_black());
////        assert(!get_left() || get_left()->value <= value || get_left() == removal_node);
////        assert(!get_right() || get_right()->value >= value || get_right() == removal_node);
//
//        const int bnc_left = get_left()->verify(removal_node);
//        const int bnc_right = get_right()->verify(removal_node);
//
//        assert(bnc_left == bnc_right);
//
//        return bnc_left + (is_black() && removal_node != this ? 1 : 0);
//      }
//      else {
//        return 1;
//      }
//    }
//#endif

    key_type key;
    union {
      uintptr_t m_color_bits;
      map_pointer_type m_parent;
    };
  };

}

#endif
