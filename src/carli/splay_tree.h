#ifndef SPLAY_TREE_H
#define SPLAY_TREE_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#ifndef NDEBUG
#include <cstring>
#endif

using namespace std;

template <typename TYPE, typename COMPARE = std::less<TYPE>>
class Splay_Tree {
public:
  Splay_Tree(const TYPE &value_)
    : value(value_)
  {
  }

  const TYPE & get_value() const {
    return value;
  }

  void insert_into(Splay_Tree * &root) {
    insert_recursively(root, root, nullptr, COMPARE());

//    assert(root->verify());
  }

  void remove_from(Splay_Tree * &root) {
    assert(this);
    if(!this)
      throw::std::runtime_error("Attempt to remove null Splay_Tree * from AVL tree.");

    assert(!root->parent);

    if(left)
      swap_remove_rightmost(left, root);
    else if(right)
      swap_remove_leftmost(right, root);
    else {
      if(parent) {
        if(parent->left == this)
          parent->left = nullptr;
        else {
          assert(parent->right == this);
          parent->right = nullptr;
        }
      }
      else
        root = nullptr;
    }

#ifndef NDEBUG
    memset(this, 0xDEADBEEF, sizeof(Splay_Tree));
#endif
    delete this;

//    assert(root->verify());
  }

  static Splay_Tree * find_in(Splay_Tree * &root, const TYPE &value_) {
    return root->find_recursively(value_, root, nullptr, COMPARE());
  }

#ifndef NDEBUG
  size_t debug_height() const {
    return this ? std::max(left->debug_height(), right->debug_height()) + 1 : 0;
  }

  size_t debug_size() const {
    return this ? left->debug_size() + right->debug_size() + 1 : 0;
  }
#endif

private:
  Splay_Tree * find_recursively(const TYPE &value_, Splay_Tree * &root, Splay_Tree * const parent_, const COMPARE &compare) {
    if(this) {
      if(compare(value_, value))
        return left->find_recursively(value_, root, this, compare);
      else if(compare(value, value_))
        return right->find_recursively(value_, root, this, compare);
      else {
        splay(root);
        return this;
      }
    }
    else {
      parent_->splay(root);
      return nullptr;
    }
  }

  void insert_recursively(Splay_Tree * &node, Splay_Tree * &root, Splay_Tree * const parent_, const COMPARE &compare) {
    if(node) {
      if(compare(value, node->value))
        insert_recursively(node->left, root, node, compare);
      else
        insert_recursively(node->right, root, node, compare);
    }
    else {
      node = this;
      parent = parent_;
      splay(root);
    }
  }

  void swap_remove_leftmost(Splay_Tree * &node, Splay_Tree * &root) {
    if(node->left)
      swap_remove_leftmost(node->left, root);
    else {
      Splay_Tree * const node_ = node;
      Splay_Tree * const node_parent = node->parent != this ? node->parent : node;

      /// Disconnect node
      if(node->right)
        node->right->parent = node->parent;
      node = node->right;

      replacement_node(node_, root);
      node_parent->splay(root);
    }
  }

  void swap_remove_rightmost(Splay_Tree * &node, Splay_Tree * &root) {
    if(node->right)
      swap_remove_rightmost(node->right, root);
    else {
      Splay_Tree * const node_ = node;
      Splay_Tree * const node_parent = node->parent != this ? node->parent : node;

      /// Disconnect node
      if(node->left)
        node->left->parent = node->parent;
      node = node->left;

      replacement_node(node_, root);
      node_parent->splay(root);
    }
  }

  void replacement_node(Splay_Tree * const &node, Splay_Tree * &root) {
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
    if(left)
      left->parent = node;
    node->left = left;
    if(right)
      right->parent = node;
    node->right = right;
  }

  void splay(Splay_Tree * &root) {
    if(this && parent) {
      if(parent->parent) {
        if(parent->parent->left == parent) {
          if(parent->left == this) {
            parent->parent->rotate_right(root);
            parent->rotate_right(root);
          }
          else {
            assert(parent->right == this);
            parent->rotate_left(root);
            parent->rotate_right(root);
          }
        }
        else {
          assert(parent->parent->right == parent);
          if(parent->right == this) {
            parent->parent->rotate_left(root);
            parent->rotate_left(root);
          }
          else {
            assert(parent->left == this);
            parent->rotate_right(root);
            parent->rotate_left(root);
          }
        }

        parent->splay(root);
      }
      else {
        if(parent->left == this)
          parent->rotate_right(root);
        else {
          assert(parent->right == this);
          parent->rotate_left(root);
        }
      }
    }
  }

  void rotate_left(Splay_Tree * &root) {
    Splay_Tree * const child = right;

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
  }

  void rotate_right(Splay_Tree * &root) {
    Splay_Tree * const child = left;

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
  }

#ifndef NDEBUG
  bool verify() const {
    if(this) {
      assert(parent != this);
      if(!(parent != this))
        return false;

      if(left) {
        assert(left != this);
        if(!(left != this))
          return false;

        assert(left->parent == this);
        if(!(left->parent == this))
          return false;

        if(right) {
          assert(right != this);
          if(!(right != this))
            return false;

          assert(right->parent == this);
          if(!(right->parent == this))
            return false;

          assert(right->verify());
          if(!(right->verify()))
            return false;
        }
        else

        assert(left->verify());
        if(!(left->verify()))
          return false;
      }
      else if(right) {
        assert(right != this);
        if(!(right != this))
          return false;

        assert(right->parent == this);
        if(!(right->parent == this))
          return false;

        assert(right->verify());
        if(!(right->verify()))
          return false;
      }
    }

    return true;
  }
#endif

  TYPE value;
  Splay_Tree *parent = nullptr;
  Splay_Tree *left = nullptr;
  Splay_Tree *right = nullptr;
};

#endif
