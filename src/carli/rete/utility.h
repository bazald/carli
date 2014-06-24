#ifndef RETE_UTILITY_H
#define RETE_UTILITY_H

#include <array>
#include <functional>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../linkage.h"

class RETE_LINKAGE compare_deref_eq {
public:
  template <typename Ptr1, typename Ptr2>
  bool operator()(const Ptr1 &lhs, const Ptr2 &rhs) const {
    return *lhs == *rhs;
  }
};

class RETE_LINKAGE compare_deref_lt {
public:
  template <typename Ptr1, typename Ptr2>
  bool operator()(const Ptr1 &lhs, const Ptr2 &rhs) const {
    return *lhs < *rhs;
  }
};

template <typename Type1, typename Type2, int64_t (Type1::*MemFun)(const Type2 &) const>
class compare_deref_memfun_lt {
public:
  template <typename Ptr1, typename Ptr2>
  bool operator()(const Ptr1 &lhs, const Ptr2 &rhs) const {
    return std::bind(MemFun, &*lhs, std::cref(*rhs))() < 0;
  }
};

inline size_t hash_combine(const size_t &prev_h, const size_t &new_val) {
  return prev_h * 31 + new_val;
}

template <typename CONTAINER, typename HASHER>
size_t hash_container(const CONTAINER &container, HASHER &hasher) {
  size_t h = 0;
  for(const auto &entry : container)
    h = hash_combine(h, hasher(entry));
  return h;
}

template <typename T>
class hash_deref {
public:
  template <typename Ptr>
  size_t operator()(const Ptr &ptr) const {
    return std::hash<T>()(*ptr);
  }
};

namespace std {
  template <typename T, size_t N>
  struct hash<array<T, N>> {
    size_t operator()(const array<T, N> &a) const {
      hash<T> hasher;
      return hash_container(a, hasher);
    }
  };

  template <typename T1, typename T2>
  struct hash<pair<T1, T2>> {
    size_t operator()(const pair<T1, T2> &p) const {
      return hash_combine(hash<T1>()(p.first), hash<T2>()(p.second));
    }
  };

  template <typename Key, typename Hash, typename Pred, typename Alloc>
  struct hash<unordered_set<Key, Hash, Pred, Alloc>> {
    size_t operator()(const unordered_set<Key, Hash, Pred, Alloc> &us) const {
      hash<Key> hasher;
      return hash_container(us, hasher);
    }
  };

  template <typename T, typename Alloc>
  struct hash<vector<T, Alloc>> {
    size_t operator()(const vector<T, Alloc> &v) const {
      hash<T> hasher;
      return hash_container(v, hasher);
    }
  };
}

#endif
