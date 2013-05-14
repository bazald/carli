#ifndef RB_TREE_H
#define RB_TREE_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

#ifndef NDEBUG
#include <cstring>
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

    assert(root->verify());
  }

  void remove_from(RB_Tree * &root) {
    assert(this);
    if(!this)
      throw::std::runtime_error("Attempt to remove null RB_Tree * from AVL tree.");

    assert(!root->parent);

    RB_Tree * const root2 = root != this ? root : root->right ? root->right : root->left;

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

    if(root2) {
      root = root2;
      while(root->parent)
        root = root->parent;
    }

#ifndef NDEBUG
    memset(this, 0xDEADBEEF, sizeof(RB_Tree));
#endif
    delete this;

    assert(root->verify());
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

    assert(root->verify());
  }

  void insert_case1(RB_Tree * &root) {
    if(parent)
      insert_case2(root);
    else
      color = Color::BLACK;

    assert(root->verify());
  }

  void insert_case2(RB_Tree * &root) {
    if(parent->color == Color::RED)
      insert_case3(root);

    assert(root->verify());
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

    assert(root->verify());
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

    assert(root->verify());
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

    assert(root->verify());
  }

  void swap_remove_leftmost(RB_Tree * &node, RB_Tree * &root) {
    if(node->left)
      swap_remove_leftmost(node->left, root);
    else {
      RB_Tree * const node_ = node;
      RB_Tree * const node_parent = node->parent;

      /// Disconnect node
      if(node->right)
        node->right->parent = node->parent;
      node = node->right;

      replacement_node(node_, root);
      node_parent->delete_one_child(node_parent->right);
    }
  }

  void swap_remove_rightmost(RB_Tree * &node, RB_Tree * &root) {
    if(node->right)
      swap_remove_rightmost(node->right, root);
    else {
      RB_Tree * const node_ = node;
      RB_Tree * const node_parent = node->parent;

      /// Disconnect node
      if(node->left)
        node->left->parent = node->parent;
      node = node->left;

      replacement_node(node_, root);
      node_parent->delete_one_child(node_parent->right);
    }
  }

  void delete_one_child(RB_Tree * const &child) {
    if(color == Color::BLACK) {
      if(child->get_color() == Color::BLACK)
        child->delete_case1(this);
      else
        child->color = Color::BLACK;
    }
  }

  void delete_case1(RB_Tree * const &parent_) {
    if(parent_)
      delete_case2(parent_);
  }

  void delete_case2(RB_Tree * const &parent_) {
    RB_Tree * const si = sibling(parent_);

    if(si->get_color() == Color::RED) {
      parent_->color = Color::RED;
      si->color = Color::BLACK;
      if(parent_->left == this)
        parent_->rotate_left();
      else {
        assert(parent_->right == this);
        parent_->rotate_right();
      }
    }

    delete_case3(parent);
  }

  void delete_case3(RB_Tree * const &parent_) {
    RB_Tree * const si = sibling(parent_);

    if(parent_->color == Color::BLACK &&
       si->get_color() == Color::BLACK &&
       si->left->color == Color::BLACK &&
       si->right->color == Color::BLACK)
    {
      si->color = Color::RED;
      parent->delete_case1(parent->parent);
    }
    else
      ;//delete_case4(n);
  }

  void replacement_node(RB_Tree * const &node, RB_Tree * &root) {
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
    node->color = color;

    assert(root->verify());
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
  }

  void rotate_left() {
    assert(verify());

    RB_Tree * const grandparent = parent;
    RB_Tree * const child = right;
    RB_Tree * const grandchild = child->left;

    parent = child;
    right = grandchild;
    if(grandchild)
      grandchild->parent = this;

    child->parent = grandparent;
    child->left = this;
    if(grandparent) {
      if(grandparent->left == this)
        grandparent->left = child;
      else {
        assert(grandparent->right == this);
        grandparent->right = child;
      }
    }

    assert(child->verify());
    assert(grandparent->verify());
  }

  void rotate_right() {
    assert(verify());

    RB_Tree * const grandparent = parent;
    RB_Tree * const child = left;
    RB_Tree * const grandchild = child->right;

    parent = child;
    left = grandchild;
    if(grandchild)
      grandchild->parent = this;

    child->parent = grandparent;
    child->right = this;
    if(grandparent) {
      if(grandparent->left == this)
        grandparent->left = child;
      else {
        assert(grandparent->right == this);
        grandparent->right = child;
      }
    }

    assert(child->verify());
    assert(grandparent->verify());
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
  bool verify() const {
    if(this) {
      assert(parent != this);
      if(!(parent != this))
        return false;

      assert(parent || color == Color::BLACK);
      if(!(parent || color == Color::BLACK))
        return false;

      assert(!parent || parent->color == Color::BLACK || color == Color::BLACK);
      if(!(!parent || parent->color == Color::BLACK || color == Color::BLACK))
        return false;

      if(left) {
        assert(left != this);
        if(!(left != this))
          return false;

        assert(left->parent == this);
        if(!(left->parent == this))
          return false;

        assert(color == Color::BLACK || left->color == Color::BLACK);
        if(!(color == Color::BLACK || left->color == Color::BLACK))
          return false;

        if(right) {
          assert(right != this);
          if(!(right != this))
            return false;

          assert(right->parent == this);
          if(!(right->parent == this))
            return false;

          assert(color == Color::BLACK || right->color == Color::BLACK);
          if(!(color == Color::BLACK || right->color == Color::BLACK))
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

        assert(color == Color::BLACK || right->color == Color::BLACK);
        if(!(color == Color::BLACK || right->color == Color::BLACK))
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
  RB_Tree *parent = nullptr;
  RB_Tree *left = nullptr;
  RB_Tree *right = nullptr;
  Color color = Color::RED;
};

#endif
