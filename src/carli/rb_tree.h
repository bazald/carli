#ifndef RB_TREE_H
#define RB_TREE_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>

#ifndef NDEBUG
#include <cstring>
#include <iostream>
#endif

using namespace std;

template <typename TYPE, typename COMPARE = std::less<TYPE>>
class RB_Tree {
  RB_Tree(const RB_Tree &) = delete;
  RB_Tree operator=(const RB_Tree &) = delete;

public:
  RB_Tree(const TYPE &value_)
    : value(value_)
  {
  }

  const TYPE & get_value() const {
    return value;
  }

  void insert_into(RB_Tree * &root) {
    insert_case0(root, root, nullptr);

    assert(root->verify(nullptr));
  }

  void remove_from(RB_Tree * &root) {
    assert(this);
    if(!this)
      throw::std::runtime_error("Attempt to remove null RB_Tree * from AVL tree.");

    assert(!root->get_parent());

    if(left)
      swap_remove_rightmost(left, root);
    else if(right)
      swap_remove_leftmost(right, root);
    else {
      remove_child(root, nullptr);

      assert(root->verify(nullptr));
    }

#ifndef NDEBUG
    memset(this, 0xDEADBEEF, sizeof(RB_Tree));
#endif
    delete this;
  }

  RB_Tree * find(const TYPE &value_) {
    if(this) {
      if(COMPARE()(value_, value))
        return left->find(value_);
      else if(COMPARE()(value, value_))
        return right->find(value_);
      else
        return this;
    }
    else
      return nullptr;
  }

#ifndef NDEBUG
  size_t debug_height() const {
    return this ? std::max(left->debug_height(), right->debug_height()) + 1 : 0;
  }

  size_t debug_size() const {
    return this ? left->debug_size() + right->debug_size() + 1 : 0;
  }

  std::ostream & debug_print(std::ostream &os) const {
    /// vt100 stuff
    const char * const white_on_red = "\033[37;41m";
    const char * const black_on_white = "\033[30;47m";
    const char * const default_on_default = "\033[39;49m";

    if(this) {
      os << (is_red() ? white_on_red : black_on_white) << value;
      if(left || right) {
        os << default_on_default << '(';
        left->debug_print(os);
        os << default_on_default << ':';
        right->debug_print(os);
        os << default_on_default << ')';
      }
      else
        os << default_on_default;
    }

    return os;
  }
#endif

private:
  void insert_case0(RB_Tree * &node, RB_Tree * &root, RB_Tree * const parent) {
    if(node) {
      if(COMPARE()(value, node->value))
        insert_case0(node->left, root, node);
      else
        insert_case0(node->right, root, node);
    }
    else {
      node = this;
      m_parent = parent;
      insert_case1(root, parent);
    }

    assert(root->verify(nullptr));
  }

  void insert_case1(RB_Tree * &root, RB_Tree * const parent) {
    if(parent)
      insert_case2(root, parent);
    else
      set_black();

    assert(root->verify(nullptr));
  }

  void insert_case2(RB_Tree * &root, RB_Tree * const parent) {
    if(parent->is_red())
      insert_case3(root, parent);

    assert(root->verify(nullptr));
  }

  void insert_case3(RB_Tree * &root, RB_Tree * const parent) {
    assert(is_red());
    assert(parent->is_red());

    RB_Tree * const gp = parent->get_parent();
    RB_Tree * const un = uncle(parent, gp);

    if(un->is_red()) {
      parent->set_black();
      un->set_black();
      gp->set_red();
      gp->insert_case1(root, gp->get_parent());
    }
    else
      insert_case4(root, parent, gp);

    assert(root->verify(nullptr));
  }

  void insert_case4(RB_Tree * &root, RB_Tree * const parent, RB_Tree * const gp) {
    if(parent->right == this && parent == gp->left) {
      parent->rotate_left(root);
      left->insert_case5(root);
    }
    else if(parent->left == this && parent == gp->right) {
      parent->rotate_right(root);
      right->insert_case5(root);
    }
    else
      insert_case5(root);

    assert(root->verify(nullptr));
  }

  void insert_case5(RB_Tree * &root) {
    RB_Tree * const parent = get_parent();

    assert(is_red());
    assert(parent->is_red());

    RB_Tree * const gp = parent->get_parent();

    parent->set_black();
    gp->set_red();
    if(parent->left == this)
      gp->rotate_right(root);
    else
      gp->rotate_left(root);

    assert(root->verify(nullptr));
  }

  void swap_remove_leftmost(RB_Tree * const node, RB_Tree * &root) {
    if(node->left)
      swap_remove_leftmost(node->left, root);
    else
      swap_remove_child(node, root, node->right);
  }

  void swap_remove_rightmost(RB_Tree * const node, RB_Tree * &root) {
    if(node->right)
      swap_remove_rightmost(node->right, root);
    else
      swap_remove_child(node, root, node->left);
  }

  void swap_remove_child(RB_Tree * const node, RB_Tree * &root, RB_Tree * const child) {
    assert(root->verify(nullptr));

    swap_nodes(node, root);

    assert(root->verify(nullptr));

    remove_child(root, child);
  }

  void remove_child(RB_Tree * &root, RB_Tree * const child) {
    RB_Tree * const parent = get_parent();

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
    root->debug_print(std::cerr) << std::endl;

    assert(root->verify(nullptr));
#endif
  }

  void rebalance(RB_Tree * &root) {
    delete_case1(root);

#ifndef NDEBUG
    root->debug_print(std::cerr) << std::endl;
#endif
  }

  void disconnect_one_child(RB_Tree * const parent) {
    if(parent) {
      if(parent->left == this) {
        parent->left = left ? left : right;
        if(parent->left)
          parent->left->set_parent(parent);
      }
      else {
        assert(parent->right == this);
        parent->right = left ? left : right;
        if(parent->right)
          parent->right->set_parent(parent);
      }
    }
  }

  void delete_case1(RB_Tree * &root) {
    RB_Tree * const parent = get_parent();

    if(parent)
      delete_case2(root, parent);
    else {
#ifndef NDEBUG
      std::cerr << "case1" << std::endl;
#endif
    }
  }

  void delete_case2(RB_Tree * &root, RB_Tree * const parent) {
    RB_Tree * si = sibling(parent);

    if(si) {
      if(parent->is_black() &&
         si->is_red())
      {
#ifndef NDEBUG
        std::cerr << "case2" << std::endl;
#endif
        parent->set_red();
        si->set_black();
        if(parent->left == si)
          parent->rotate_right(root);
        else
          parent->rotate_left(root);
      }

      delete_case3(root);
    }
  }

  void delete_case3(RB_Tree * &root) {
    RB_Tree * const parent = get_parent();
    RB_Tree * const si = sibling(parent);

    if(parent->is_black() &&
       si->is_black() &&
       si->left->is_black() &&
       si->right->is_black())
    {
#ifndef NDEBUG
      std::cerr << "case3" << std::endl;
#endif
      si->set_red();
#ifndef NDEBUG
      root->debug_print(std::cerr) << std::endl;
#endif

      parent->delete_case1(root);
    }
    else
      delete_case4(root);
  }

  void delete_case4(RB_Tree * &root) {
    RB_Tree * const parent = get_parent();
    RB_Tree * const si = sibling(parent);

    if(parent->is_red() &&
       si->is_black() &&
       si->left->is_black() &&
       si->right->is_black())
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

  void delete_case5(RB_Tree * &root, RB_Tree * const parent, RB_Tree * const si) {
    if(si->is_black()) {
      if(this == parent->left &&
         si->right->is_black() &&
         si->left->is_red())
      {
#ifndef NDEBUG
        std::cerr << "case5a" << std::endl;
#endif
        si->set_red();
        si->left->set_black();
        si->rotate_right(root);
      }
      else if(this == parent->right &&
              si->left->is_black() &&
              si->right->is_red())
      {
#ifndef NDEBUG
        std::cerr << "case5b" << std::endl;
#endif
        si->set_red();
        si->right->set_black();
        si->rotate_left(root);
      }
    }

    delete_case6(root);
  }

  void delete_case6(RB_Tree * &root) {
#ifndef NDEBUG
    std::cerr << "case6" << std::endl;
#endif

    RB_Tree * const parent = get_parent();
    RB_Tree * const si = sibling(parent);

    si->set_color(parent->get_color());
    parent->set_black();

    if(parent->left == si) {
      if(si->left)
        si->left->set_black();
      parent->rotate_right(root);
    }
    else {
      if(si->right)
        si->right->set_black();
      parent->rotate_left(root);
    }
  }

  void swap_nodes(RB_Tree * const node, RB_Tree * &root) {
    if(node->get_parent() != this) {
      std::swap(m_parent, node->m_parent);
      std::swap(left, node->left);
      std::swap(right, node->right);

      fixup_post_swap(root, node);
      node->fixup_post_swap(root, this);
    }
    else {
      RB_Tree * const parent = get_parent();
      const uintptr_t node_color = node->get_color();

      if(parent) {
        if(parent->left == this)
          parent->left = node;
        else {
          assert(parent->right == this);
          parent->right = node;
        }
      }
      else
        root = node;
      node->m_parent = m_parent;
      set_parent(node);
      set_color(node_color);

      RB_Tree * const child = node->left ? node->left : node->right;
      if(child)
        child->set_parent(this);

      if(left == node) {
        node->left = this;
        node->right = right;
        if(right)
          right->set_parent(node);
        left = child;
        right = nullptr;
      }
      else {
        assert(right == node);
        node->left = left;
        node->right = this;
        if(left)
          right->set_parent(node);
        left = nullptr;
        right = child;
      }
    }

#ifndef NDEBUG
    value = node->value;

    root->debug_print(std::cerr) << std::endl;
#endif
  }

  void fixup_post_swap(RB_Tree * & root, RB_Tree * const prev) {
    RB_Tree * const parent = get_parent();

    if(parent) {
      if(parent->left == prev)
        parent->left = this;
      else {
        assert(parent->right == prev);
        parent->right = this;
      }
    }
    else
      root = this;

    if(left)
      left->set_parent(this);

    if(right)
      right->set_parent(this);
  }

  void rotate_left(RB_Tree * &root) {
    RB_Tree * const parent = get_parent();
    RB_Tree * const child = right;

    right = child->left;
    if(child->left)
      child->left->set_parent(this);

    child->set_parent(parent);
    child->left = this;
    if(parent) {
      if(parent->left == this)
        parent->left = child;
      else {
        assert(parent->right == this);
        parent->right = child;
      }
    }
    else
      root = child;

    set_parent(child);

#ifndef NDEBUG
    root->debug_print(std::cerr) << std::endl;
#endif
  }

  void rotate_right(RB_Tree * &root) {
    RB_Tree * const parent = get_parent();
    RB_Tree * const child = left;

    left = child->right;
    if(child->right)
      child->right->set_parent(this);

    child->set_parent(parent);
    child->right = this;
    if(parent) {
      if(parent->left == this)
        parent->left = child;
      else {
        assert(parent->right == this);
        parent->right = child;
      }
    }
    else
      root = child;

    set_parent(child);

#ifndef NDEBUG
    root->debug_print(std::cerr) << std::endl;
#endif
  }

  RB_Tree * sibling(RB_Tree * const parent) {
    return parent->right == this ? parent->left : parent->right;
  }

  RB_Tree * uncle(RB_Tree * const parent, RB_Tree * const gp) {
    return gp ? (gp->right == parent ? gp->left : gp->right) : nullptr;
  }

  const RB_Tree * get_parent() const {
    return reinterpret_cast<RB_Tree *>(uintptr_t(m_parent) & get_mask());
  }

  RB_Tree * get_parent() {
    return reinterpret_cast<RB_Tree *>(uintptr_t(m_parent) & get_mask());
  }

  void set_parent(RB_Tree * const parent) {
    m_parent = reinterpret_cast<RB_Tree *>(uintptr_t(parent) | get_color());
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

#ifndef NDEBUG
  int verify(RB_Tree * const removal_node) const {
    if(this) {
      const RB_Tree * const parent = get_parent();

      assert(parent != this);
      assert(left != this);
      assert(right != this);
      assert(!parent || parent->left == this || parent->right == this);
      assert(parent || is_black());
      assert(!parent || parent->is_black() || is_black());
      assert(!left || left->value <= value || left == removal_node);
      assert(!right || right->value >= value || right == removal_node);

      const int bnc_left = left->verify(removal_node);
      const int bnc_right = right->verify(removal_node);

      assert(bnc_left == bnc_right);

      return bnc_left + (is_black() && removal_node != this ? 1 : 0);
    }
    else {
      return 1;
    }
  }
#endif

  TYPE value;
  union {
    uintptr_t m_color_bits;
    RB_Tree *m_parent;
  };
  RB_Tree *left = nullptr;
  RB_Tree *right = nullptr;
};

#endif
