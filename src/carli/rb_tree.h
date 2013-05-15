#ifndef RB_TREE_H
#define RB_TREE_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#ifndef NDEBUG
#include <cstring>
#include <iostream>
#endif

using namespace std;

template <typename TYPE, typename COMPARE = std::less<TYPE>>
class RB_Tree {
public:
  enum class Color : char {RED, BLACK};

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

    assert(!root->parent);

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
      os << (color == Color::RED ? white_on_red : black_on_white) << value;
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
  void insert_case0(RB_Tree * &node, RB_Tree * &root, RB_Tree * const parent_) {
    if(node) {
      if(COMPARE()(value, node->value))
        insert_case0(node->left, root, node);
      else
        insert_case0(node->right, root, node);
    }
    else {
      node = this;
      parent = parent_;
      insert_case1(root);
    }

    assert(root->verify(nullptr));
  }

  void insert_case1(RB_Tree * &root) {
    if(parent)
      insert_case2(root);
    else
      color = Color::BLACK;

    assert(root->verify(nullptr));
  }

  void insert_case2(RB_Tree * &root) {
    if(parent->color == Color::RED)
      insert_case3(root);

    assert(root->verify(nullptr));
  }

  void insert_case3(RB_Tree * &root) {
    assert(color == Color::RED);
    assert(parent->color == Color::RED);

    RB_Tree * const gp = grandparent();
    RB_Tree * const un = uncle(gp);

    if(un && un->color == Color::RED) {
      parent->color = Color::BLACK;
      un->color = Color::BLACK;
      gp->color = Color::RED;
      gp->insert_case1(root);
    }
    else
      insert_case4(root, gp);

    assert(root->verify(nullptr));
  }

  void insert_case4(RB_Tree * &root, RB_Tree * const &gp) {
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
    assert(color == Color::RED);
    assert(parent->color == Color::RED);

    RB_Tree * const gp = grandparent();

    parent->color = Color::BLACK;
    gp->color = Color::RED;
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
    /// Rebalance
    if(child) {
      disconnect_one_child(parent);

      if(color == Color::BLACK) {
        if(child->color == Color::BLACK)
          rebalance(root);
        else
          child->color = Color::BLACK;
      }
    }
    else if(root != this) {
      if(color == Color::BLACK)
        rebalance(root);

      disconnect_one_child(parent ? parent : root);
    }
    else {
      root = nullptr;
      return;
    }

#ifndef NDEBUG
    root->debug_print(std::cerr) << "\033[39;49m" << std::endl;

    assert(root->verify(nullptr));
#endif
  }

  void rebalance(RB_Tree * &root) {
    for(RB_Tree * node = this;; node = node->parent) {
      /// Case 1
      if(!node->parent) {
#ifndef NDEBUG
        std::cerr << "case1" << std::endl;
#endif
        break;
      }

      RB_Tree * si = node->sibling(node->parent);

      if(si) {
        /// Case 2
        if(node->parent->color == Color::BLACK &&
           si->color == Color::RED)
        {
#ifndef NDEBUG
          std::cerr << "case2" << std::endl;
#endif
          node->parent->color = Color::RED;
          si->color = Color::BLACK;
          if(node->parent->left == si)
            node->parent->rotate_right(root);
          else
            node->parent->rotate_left(root);
        }

        si = node->sibling(node->parent);

        /// Case 3
        if(node->parent->color == Color::BLACK &&
           si->color == Color::BLACK &&
           (!si->left || si->left->color == Color::BLACK) &&
           (!si->right || si->right->color == Color::BLACK))
        {
#ifndef NDEBUG
          std::cerr << "case3" << std::endl;
#endif
          si->color = Color::RED;
#ifndef NDEBUG
          root->debug_print(std::cerr) << "\033[39;49m" << std::endl;
#endif
          continue;
        }

        si = node->sibling(node->parent);

        /// Case 4
        if(si->parent->color == Color::RED &&
           si->color == Color::BLACK &&
           (!si->left || si->left->color == Color::BLACK) &&
           (!si->right || si->right->color == Color::BLACK))
        {
#ifndef NDEBUG
          std::cerr << "case4" << std::endl;
#endif
          si->parent->color = Color::BLACK;
          si->color = Color::RED;
          break;
        }

        /// Case 5
        if(si->color == Color::BLACK) {
          if(node == node->parent->left &&
             (!si->right || si->right->color == Color::BLACK) &&
             (si->left && si->left->color == Color::RED))
          {
#ifndef NDEBUG
            std::cerr << "case5a" << std::endl;
#endif
            si->color = Color::RED;
            si->left->color = Color::BLACK;
            si->rotate_right(root);
          }
          else if(node == node->parent->right &&
                  (!si->left || si->left->color == Color::BLACK) &&
                  (si->right && si->right->color == Color::RED))
          {
#ifndef NDEBUG
            std::cerr << "case5b" << std::endl;
#endif
            si->color = Color::RED;
            si->right->color = Color::BLACK;
            si->rotate_left(root);
          }
        }

        si = node->sibling(node->parent);

        /// Case 6
#ifndef NDEBUG
        std::cerr << "case6" << std::endl;
#endif
        si->color = node->parent->color;
        node->parent->color = Color::BLACK;

        if(node->parent->left == si) {
          if(si->left)
            si->left->color = Color::BLACK;
          node->parent->rotate_right(root);
        }
        else {
          if(si->right)
            si->right->color = Color::BLACK;
          node->parent->rotate_left(root);
        }
      }

      break;
    }

#ifndef NDEBUG
    root->debug_print(std::cerr) << "\033[39;49m" << std::endl;
#endif
  }

  void disconnect_one_child(RB_Tree * &parent_) {
    if(parent_) {
      if(parent_->left == this) {
        parent_->left = left ? left : right;
        if(parent_->left)
          parent_->left->parent = parent_;
      }
      else {
        assert(parent_->right == this);
        parent_->right = left ? left : right;
        if(parent_->right)
          parent_->right->parent = parent_;
      }
    }
  }

//  void delete_one_child(RB_Tree * &root, RB_Tree * const child, RB_Tree * const removal_node) {
//    if(color == Color::BLACK) {
//      assert(child);
//#ifndef NDEBUG
//      std::cerr << "delete_one_child" << std::endl;
//#endif
//      if(child->color == Color::BLACK)
//        child->delete_case1(this, root, this);
//      else
//        child->color = Color::BLACK;
//    }
//
//    assert(root->verify(removal_node));
//  }

  void delete_case1(RB_Tree * const parent_, RB_Tree * &root, RB_Tree * const removal_node) {
    if(parent_) {
#ifndef NDEBUG
      std::cerr << "delete_case1" << std::endl;
#endif
      delete_case2(parent_, root, removal_node);
    }

    assert(root->verify(removal_node));
  }

//  void delete_case2(RB_Tree * const parent_, RB_Tree * &root, RB_Tree * const removal_node) {
//    RB_Tree * const si = sibling(parent_);
//
//    if(si->color == Color::RED) {
//#ifndef NDEBUG
//      std::cerr << "delete_case2" << std::endl;
//#endif
//      parent_->color = Color::RED;
//      si->color = Color::BLACK;
//      if(parent_->left == this)
//        parent_->rotate_left(root);
//      else {
//        assert(parent_->right == this);
//        parent_->rotate_right(root);
//      }
//    }
//
//    delete_case3(this ? parent : parent_, root, removal_node);
//
//    assert(root->verify(removal_node));
//  }

  void delete_case3(RB_Tree * const parent_, RB_Tree * &root, RB_Tree * const removal_node) {
    RB_Tree * const si = sibling(parent_);

    if(parent_->color == Color::BLACK &&
       si->color == Color::BLACK &&
       si->left->get_color() == Color::BLACK &&
       si->right->get_color() == Color::BLACK)
    {
#ifndef NDEBUG
      std::cerr << "delete_case3" << std::endl;
#endif
      si->color = Color::RED;
      parent_->delete_case1(parent_->parent, root, removal_node);
    }
    else
      delete_case4(parent_, root, removal_node, si);

    assert(root->verify(removal_node));
  }

  void delete_case4(RB_Tree * const parent_, RB_Tree * &root, RB_Tree * const removal_node, RB_Tree * const &si) {
    if(parent_->color == Color::RED &&
       si->color == Color::BLACK &&
       si->left->get_color() == Color::BLACK &&
       si->right->get_color() == Color::BLACK)
    {
#ifndef NDEBUG
      std::cerr << "delete_case4" << std::endl;
#endif
      si->color = Color::RED;
      parent_->color = Color::BLACK;
    }
    else
      delete_case5(parent_, root, removal_node, si);

    assert(root->verify(removal_node));
  }

  void delete_case5(RB_Tree * const parent_, RB_Tree * &root, RB_Tree * const removal_node, RB_Tree * const &si) {
    if(si->color == Color::BLACK) {
      if(parent_->left == this &&
         si->right->color == Color::BLACK &&
         si->left->color == Color::RED)
      {
#ifndef NDEBUG
        std::cerr << "delete_case5" << std::endl;
#endif
        si->color = Color::RED;
        si->left->color = Color::BLACK;
        si->rotate_right(root);
      }
      else if(parent_->right == this &&
              si->left->color == Color::BLACK &&
              si->right->color == Color::RED)
      {
#ifndef NDEBUG
        std::cerr << "delete_case5" << std::endl;
#endif
        si->color = Color::RED;
        si->right->color = Color::BLACK;
        si->rotate_left(root);
      }
    }

    delete_case6(parent_, root, removal_node);
  }

  void delete_case6(RB_Tree * const parent_, RB_Tree * &root, RB_Tree * const removal_node) {
    RB_Tree * const si = sibling(parent_);

    si->color = parent_->color;
    parent_->color = Color::BLACK;

#ifndef NDEBUG
    std::cerr << "delete_case6" << std::endl;
#endif

    if(parent_->left == this) {
      si->right->color = Color::BLACK;
      parent->rotate_left(root);
    }
    else {
      assert(parent_->right == this);
      si->left->color = Color::BLACK;
      parent->rotate_right(root);
    }

    assert(root->verify(removal_node));
  }

  void swap_nodes(RB_Tree * const node, RB_Tree * &root) {
    if(node->parent != this) {
      std::swap(parent, node->parent);
      std::swap(left, node->left);
      std::swap(right, node->right);

      fixup_post_swap(root, node);
      node->fixup_post_swap(root, this);
    }
    else {
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
      node->parent = parent;
      parent = node;

      RB_Tree * const child = node->left ? node->left : node->right;
      if(child)
        child->parent = this;

      if(left == node) {
        node->left = this;
        node->right = right;
        if(right)
          right->parent = node;
        left = child;
        right = nullptr;
      }
      else {
        assert(right == node);
        node->left = left;
        node->right = this;
        if(left)
          right->parent = node;
        left = nullptr;
        right = child;
      }
    }

    std::swap(color, node->color);

#ifndef NDEBUG
    value = node->value;

    root->debug_print(std::cerr) << "\033[39;49m" << std::endl;
#endif
  }

  void fixup_post_swap(RB_Tree * & root, RB_Tree * const &prev) {
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
      left->parent = this;

    if(right)
      right->parent = this;
  }

  void rotate_left(RB_Tree * &root) {
    RB_Tree * const child = right;

    right = child->left;
    if(child->left)
      child->left->parent = this;

    child->parent = parent;
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

    parent = child;

#ifndef NDEBUG
    root->debug_print(std::cerr) << "\033[39;49m" << std::endl;
#endif
  }

  void rotate_right(RB_Tree * &root) {
    RB_Tree * const child = left;

    left = child->right;
    if(child->right)
      child->right->parent = this;

    child->parent = parent;
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

    parent = child;

#ifndef NDEBUG
    root->debug_print(std::cerr) << "\033[39;49m" << std::endl;
#endif
  }

  RB_Tree * sibling(const RB_Tree * const &parent_) {
    return parent_->right == this ? parent_->left : parent_->right;
  }

  RB_Tree * uncle(const RB_Tree * const &gp) {
    return gp ? (gp->right == parent ? gp->left : gp->right) : nullptr;
  }

  RB_Tree * grandparent() {
    return parent ? parent->parent : nullptr;
  }

  Color get_color() const {
    return this ? color : Color::BLACK;
  }

#ifndef NDEBUG
  int verify(RB_Tree * const removal_node) const {
    if(this) {
      assert(parent != this);
      assert(left != this);
      assert(right != this);
      assert(!parent || parent->left == this || parent->right == this);
      assert(parent || color == Color::BLACK);
      assert(!parent || parent->color == Color::BLACK || color == Color::BLACK);
      assert(!left || left->value <= value || left == removal_node);
      assert(!right || right->value >= value || right == removal_node);

//      std::cerr << '(' << value << ',' << (removal_node != this ? (color == Color::BLACK ? 1 : 0) : -1) << ',';

      const int bnc_left = left->verify(removal_node);
//      std::cerr << ',';

      const int bnc_right = right->verify(removal_node);
//      std::cerr << ')';

//      if(!parent)
//        std::cerr << std::endl;

      assert(bnc_left == bnc_right);

      return bnc_left + (color == Color::BLACK && removal_node != this ? 1 : 0);
    }
    else {
//      std::cerr << 1;
      return 1;
    }
  }
#endif

  TYPE value;
  RB_Tree *parent = nullptr;
  RB_Tree *left = nullptr;
  RB_Tree *right = nullptr;
  Color color = Color::RED;
};

#endif
