#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <cassert>

using namespace std;

template <typename TYPE, typename COMPARE = std::less<TYPE>>
class Node {
public:
  Node(const TYPE &value_)
    : value(value_),
    parent(nullptr),
    left(nullptr),
    right(nullptr),
    height(1)
  {
  }

  const TYPE & get_value() const {
    return value;
  }

  void insert_into(Node * &root, Node * const parent_ = nullptr) {
    if(root) {
      if(COMPARE()(value, root->value))
        insert_into(root->left, root);
      else
        insert_into(root->right, root);

      balance(root);
      root->recalculate_height();
    }
    else {
      root = this;
      parent = parent_;
    }
  }

  void remove_from(Node * &root) {
    assert(this);
    if(!this)
      throw::std::runtime_error("Attempt to remove null Node * from AVL tree.");

    assert(!root->parent);

    Node * const left_ = left;
    Node * const right_ = right;
    Node * const parent_ = parent;

    if(left) {
      swap_remove_rightmost(left, root);

      balance_up(left_->parent ? (left_->parent->left == left_ ? left_->parent->left : left_->parent->right) : root, root);
    }
    else if(right) {
      swap_remove_leftmost(right, root);

      balance_up(right_->parent ? (right_->parent->left == left ? right_->parent->left : right_->parent->right) : root, root);
    }
    else {
      if(parent) {
        if(parent->left == this)
          parent->left = nullptr;
        else {
          assert(parent->right == this);
          parent->right = nullptr;
        }

        balance_up(parent_->parent ? (parent_->parent->left == parent_ ? parent_->parent->left : parent_->parent->right) : root, root);
      }
      else
        root = nullptr;
    }

#ifndef NDEBUG
    memset(this, 0xDEADBEEF, sizeof(Node));
#endif
    delete this;
  }

  Node * find(const TYPE &value_) {
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
  void swap_remove_leftmost(Node * &node, Node * &root) {
    if(node->left) {
      swap_remove_leftmost(node->left, root);
      node->recalculate_height();
    }
    else {
      Node * const node_ = node;

      /// Disconnect node
      if(node->right)
        node->right->parent = node->parent;
      node = node->right;

      /// std::swap(value, node_->value);
      if(parent) {
        if(parent->left == this)
          parent->left = node_;
        else {
          assert(parent->right == this);
          parent->right = node_;
        }
      }
      else
        root = node_;
      node_->parent = parent;
      if(left)
        left->parent = node_;
      node_->left = left;
      if(right)
        right->parent = node_;
      node_->right = right;
    }

    balance(node);
  }

  void swap_remove_rightmost(Node * &node, Node * &root) {
    if(node->right) {
      swap_remove_rightmost(node->right, root);
      node->recalculate_height();
    }
    else {
      Node * const node_ = node;

      /// Disconnect node
      if(node->left)
        node->left->parent = node->parent;
      node = node->left;

      /// std::swap(value, node_->value);
      if(parent) {
        if(parent->left == this)
          parent->left = node_;
        else {
          assert(parent->right == this);
          parent->right = node_;
        }
      }
      else
        root = node_;
      node_->parent = parent;
      if(left)
        left->parent = node_;
      node_->left = left;
      if(right)
        right->parent = node_;
      node_->right = right;
    }

    balance(node);
  }

  void balance_up(Node * &node, Node * &root) {
    if(node) {
      balance(node);

      if(node->parent) {
        if(node->parent->parent) {
          if(node->parent->parent->left == node->parent)
            balance_up(node->parent->parent->left, root);
          else {
            assert(node->parent->parent->right == node->parent);
            balance_up(node->parent->parent->right, root);
          }
        }
        else {
          assert(node->parent == root);
          balance(root);
        }
      }
    }
  }

  void balance(Node * &node) {
    if(node) {
      switch(balance_factor(node)) {
      case -2:
        if(balance_factor(node->right) == 1)
          rotate_right(node->right);
        rotate_left(node);
        break;

      case 2:
        if(balance_factor(node->left) == -1)
          rotate_left(node->left);
        rotate_right(node);
        break;

      default:
        node->recalculate_height();
        break;
      }
    }
  }

  void rotate_left(Node * &parent) const {
    Node * const grandparent = parent->parent;
    Node * const child = parent->right;
    Node * const grandchild = child->left;

    parent->parent = child;
    parent->right = grandchild;
    if(grandchild)
      grandchild->parent = parent;
    parent->recalculate_height();

    child->parent = grandparent;
    child->left = parent;
    if(grandparent) {
      if(grandparent->left == parent)
        grandparent->left = child;
      else
        grandparent->right = child;
    }
    parent = child;
    child->recalculate_height();
  }

  void rotate_right(Node * &parent) const {
    Node * const grandparent = parent->parent;
    Node * const child = parent->left;
    Node * const grandchild = child->right;

    parent->parent = child;
    parent->left = grandchild;
    if(grandchild)
      grandchild->parent = parent;
    parent->recalculate_height();

    child->parent = grandparent;
    child->right = parent;
    if(grandparent) {
      if(grandparent->left == parent)
        grandparent->left = child;
      else
        grandparent->right = child;
    }
    parent = child;
    child->recalculate_height();
  }

  int balance_factor(const Node * const &node) const {
    return node->left->get_height() - node->right->get_height();
  }

  void recalculate_height_up() {
    recalculate_height();
    if(parent)
      parent->recalculate_height_up();
  }

  void recalculate_height() {
    height = std::max(left->get_height(), right->get_height()) + 1;
  }

  int get_height() const {
    return this ? height : 0;
  }

#ifndef NDEBUG
  bool verify(const Node * const &node, const bool &ignore_height) const {
    if(node) {
      assert(node->parent != node);
      if(!(node->parent != node))
        return false;

      if(node->left) {
        assert(node->left != node);
        if(!(node->left != node))
          return false;

        assert(node->left->parent == node);
        if(!(node->left->parent == node))
          return false;

        if(node->right) {
          assert(node->right != node);
          if(!(node->right != node))
            return false;

          assert(node->right->parent == node);
          if(!(node->right->parent == node))
            return false;

          if(!ignore_height) {
            if(node->left->height > node->right->height) {
              assert(node->left->height - node->right->height < 2 && node->left->height + 1 == node->height);
              if(!(node->left->height - node->right->height < 2 && node->left->height + 1 == node->height))
                return false;
            }
            else {
              assert(node->right->height - node->left->height < 2 && node->right->height + 1 == node->height);
              if(!(node->right->height - node->left->height < 2 && node->right->height + 1 == node->height))
                return false;
            }
          }

          assert(verify(node->right, ignore_height));
          if(!(verify(node->right, ignore_height)))
            return false;
        }
        else {
          if(!ignore_height) {
            assert(node->left->height + 1 == node->height);
            if(!(node->left->height + 1 == node->height))
              return false;

            assert(node->left->height < 3);
            if(!(node->left->height < 3))
              return false;
          }
        }

        assert(verify(node->left, ignore_height));
        if(!(verify(node->left, ignore_height)))
          return false;
      }
      else if(node->right) {
        assert(node->right != node);
        if(!(node->right != node))
          return false;

        assert(node->right->parent == node);
        if(!(node->right->parent == node))
          return false;

        if(!ignore_height) {
          assert(node->right->height + 1 == node->height);
          if(!(node->right->height + 1 == node->height))
            return false;

          assert(node->right->height < 3);
          if(!(node->right->height < 3))
            return false;
        }

        assert(verify(node->right, ignore_height));
        if(!(verify(node->right, ignore_height)))
          return false;
      }
    }

    return true;
  }
#endif

  TYPE value;
  Node *parent;
  Node *left;
  Node *right;
  int height;
};

#endif
