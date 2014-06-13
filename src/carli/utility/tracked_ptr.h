/** tracked_ptr.h -- Mitchell Keith Bloch -- bazald@gmail.com -- 20130712
 *
 *  tracked_ptr provides a pointer type along the lines of shared_ptr,
 *  weak_ptr, and unique_ptr. The key objective of tracked_ptr is to
 *  allow multiple pointers to the same address, but to provide error
 *  checking at deletion time.
 *
 *  The following errors are checked:
 *  1. Other tracked_ptr instances point to the address at the time of
 *     deletion.
 *  2. No other tracked_ptr instances point to the address at the time
 *     it is overwritten or zeroed, but not deleted.
 *  3. Any tracked_ptr instances exist at exit.
 **/

#ifndef TRACKED_PTR_H
#define TRACKED_PTR_H

#include <cassert>
#include <memory>

#include "linkage.h"

template <typename T>
struct null_delete {
  void operator()(const T * const &) {}
};

template <typename T, typename Deleter = std::default_delete<T>, bool Deleteable = true>
class tracked_ptr {
public:
  typedef T element_type;
  typedef Deleter deleter_type;
  typedef element_type * pointer;
  enum {deleteable = Deleteable};

  tracked_ptr(const pointer ptr_ = nullptr);

  ~tracked_ptr() {
#ifndef NDEBUG
    zero();
#endif
  }

  tracked_ptr(const tracked_ptr<T, Deleter, Deleteable> &rhs);
  tracked_ptr<T, Deleter, Deleteable> & operator=(const tracked_ptr<T, Deleter, Deleteable> &rhs);

  /** Delete the pointer and zero out the pointer. **/
  void delete_and_zero();
  /** Zero out the pointer without deleting it. **/
  void zero();

  void swap(tracked_ptr<T, Deleter, Deleteable> &rhs) {
    tracked_ptr<T, Deleter> temp(rhs);
    rhs = *this;
    *this = temp;
  }

  pointer get() const {return ptr;}
  deleter_type get_deleter() const {return deleter_type();}
  operator bool() const {return ptr != nullptr;}

  element_type & operator*() const {return *ptr;}
  element_type * operator->() const {return ptr;}

  template <typename INT>
  element_type & operator[](const INT &index) const {
    return ptr[index];
  }

  bool operator==(const tracked_ptr<T, Deleter> &rhs) const {return ptr == rhs.ptr;}
  bool operator!=(const tracked_ptr<T, Deleter> &rhs) const {return ptr != rhs.ptr;}
  bool operator<(const tracked_ptr<T, Deleter> &rhs) const {return ptr < rhs.ptr;}
  bool operator<=(const tracked_ptr<T, Deleter> &rhs) const {return ptr <= rhs.ptr;}
  bool operator>(const tracked_ptr<T, Deleter> &rhs) const {return ptr > rhs.ptr;}
  bool operator>=(const tracked_ptr<T, Deleter> &rhs) const {return ptr >= rhs.ptr;}
  bool operator==(const T * const rhs) const {return ptr == rhs;}
  bool operator!=(const T * const rhs) const {return ptr != rhs;}
  bool operator<(const T * const rhs) const {return ptr < rhs;}
  bool operator<=(const T * const rhs) const {return ptr <= rhs;}
  bool operator>(const T * const rhs) const {return ptr > rhs;}
  bool operator>=(const T * const rhs) const {return ptr >= rhs;}

private:
  pointer ptr;
};

template <typename T, typename Deleter, bool Deleteable>
inline bool operator==(const T * const lhs, const tracked_ptr<T, Deleter, Deleteable> &rhs) {return lhs == rhs.get();}
template <typename T, typename Deleter, bool Deleteable>
inline bool operator!=(const T * const lhs, const tracked_ptr<T, Deleter, Deleteable> &rhs) {return lhs != rhs.get();}
template <typename T, typename Deleter, bool Deleteable>
inline bool operator<(const T * const lhs, const tracked_ptr<T, Deleter, Deleteable> &rhs) {return lhs < rhs.get();}
template <typename T, typename Deleter, bool Deleteable>
inline bool operator<=(const T * const lhs, const tracked_ptr<T, Deleter, Deleteable> &rhs) {return lhs <= rhs.get();}
template <typename T, typename Deleter, bool Deleteable>
inline bool operator>(const T * const lhs, const tracked_ptr<T, Deleter, Deleteable> &rhs) {return lhs > rhs.get();}
template <typename T, typename Deleter, bool Deleteable>
inline bool operator>=(const T * const lhs, const tracked_ptr<T, Deleter, Deleteable> &rhs) {return lhs >= rhs.get();}

class UTILITY_LINKAGE pointer_tracker {
  template <typename T, typename Deleter, bool Deleteable>
  friend class tracked_ptr;

#ifdef NDEBUG
  inline static void set_pointer(const void *, const void *) {}
  inline static void clear_pointer(const void *, const void *, const bool & = true) {}
public:
  inline static size_t count(const void *) {return 0;}
#else
  static void set_pointer(const void * to, const void * from);
  static void clear_pointer(const void * to, const void * from, const bool &deleteable);
public:
  static size_t count(const void * to);
  static void print(const void * to);
#endif

  void break_on(const size_t &count);
};

template <typename T, typename Deleter, bool Deleteable>
tracked_ptr<T, Deleter, Deleteable>::tracked_ptr(const pointer ptr_)
  : ptr(ptr_)
{
  pointer_tracker::set_pointer(ptr, this);
}

template <typename T, typename Deleter, bool Deleteable>
tracked_ptr<T, Deleter, Deleteable>::tracked_ptr(const tracked_ptr<T, Deleter, Deleteable> &rhs)
  : ptr(rhs.ptr)
{
  pointer_tracker::set_pointer(ptr, this);
}

template <typename T, typename Deleter, bool Deleteable>
tracked_ptr<T, Deleter, Deleteable> & tracked_ptr<T, Deleter, Deleteable>::operator=(const tracked_ptr<T, Deleter, Deleteable> &rhs) {
#ifndef NDEBUG
  if(ptr != rhs.ptr)
#endif
  {
    pointer_tracker::clear_pointer(ptr, this, deleteable);
    assert(!deleteable || !ptr || pointer_tracker::count(ptr) != 0);
    ptr = rhs.ptr;
    pointer_tracker::set_pointer(ptr, this);
  }

  return *this;
}

template <typename T, typename Deleter, bool Deleteable>
void tracked_ptr<T, Deleter, Deleteable>::delete_and_zero() {
#ifndef NDEBUG
  assert(deleteable);
  if(ptr)
#endif
  {
#ifndef NDEBUG
    if(pointer_tracker::count(ptr) != 1) {
      pointer_tracker::clear_pointer(ptr, this, deleteable);
      pointer_tracker::print(ptr);
      assert(pointer_tracker::count(ptr) == 1);
    }
#endif
    deleter_type()(ptr);
    pointer_tracker::clear_pointer(ptr, this, deleteable);
    ptr = nullptr;
  }
}

template <typename T, typename Deleter, bool Deleteable>
void tracked_ptr<T, Deleter, Deleteable>::zero() {
#ifndef NDEBUG
  if(ptr)
#endif
  {
    pointer_tracker::clear_pointer(ptr, this, deleteable);
    assert(!deleteable || pointer_tracker::count(ptr) != 0);
    ptr = nullptr;
  }
}

template <typename T, typename Deleter, bool Deleteable>
std::ostream & operator<<(std::ostream &os, const tracked_ptr<T, Deleter> &ptr) {
  return os << ptr.get();
}

#endif
