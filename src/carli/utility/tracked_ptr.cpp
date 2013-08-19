/** tracked_ptr.cpp -- Mitchell Keith Bloch -- bazald@gmail.com -- 20130712 **/

#include "tracked_ptr.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>

#ifndef NDEBUG
static size_t g_tracked_ptr_break = 0;
static size_t g_tracked_ptr_count = 0;

class pointer_tracker_impl {
  pointer_tracker_impl(const pointer_tracker_impl &);
  pointer_tracker_impl & operator=(const pointer_tracker_impl &);

  friend class pointer_tracker;

  pointer_tracker_impl() {}

  ~pointer_tracker_impl() {
    assert(address_to_pointer.empty());
  }

//public:
  static pointer_tracker_impl & get() {
    static pointer_tracker_impl pt;
    return pt;
  }

  void set_pointer(const void * to, const void * from) {
    if(to) {
      if(++g_tracked_ptr_count == g_tracked_ptr_break)
        assert(g_tracked_ptr_count != g_tracked_ptr_break);
      address_to_pointer.insert(std::make_pair(to, std::make_pair(from, g_tracked_ptr_count)));
    }
  }

  void clear_pointer(const void * to, const void * from) {
    if(to) {
      if(++g_tracked_ptr_count == g_tracked_ptr_break)
        assert(g_tracked_ptr_count != g_tracked_ptr_break);
      auto range = address_to_pointer.equal_range(to);
      auto atp = std::find_if(range.first, range.second, [from](const std::pair<const void *, std::pair<const void *, size_t>> &atp){return atp.second.first == from;});
      assert(atp != address_to_pointer.end());
      address_to_pointer.erase(atp);
    }
  }

  size_t count(const void * to) {
    auto range = address_to_pointer.equal_range(to);
    size_t count = 0;
    std::for_each(range.first, range.second, [&count](const std::pair<const void *, std::pair<const void *, size_t>> &){++count;});
    return count;
  }

  void print(const void * to) {
    auto range = address_to_pointer.equal_range(to);
    std::for_each(range.first, range.second, [](const std::pair<const void *, std::pair<const void *, size_t>> &to_from) {
      std::cerr << "tracked_ptr: " << to_from.first << " from " << to_from.second.first << " at " << to_from.second.second << std::endl;
    });
  }

//private:
  std::unordered_multimap<const void *, std::pair<const void *, size_t>> address_to_pointer;
};

void pointer_tracker::set_pointer(const void * to, const void * from) {
  return pointer_tracker_impl::get().set_pointer(to, from);
}

void pointer_tracker::clear_pointer(const void * to, const void * from) {
  return pointer_tracker_impl::get().clear_pointer(to, from);
}

size_t pointer_tracker::count(const void * to) {
  return pointer_tracker_impl::get().count(to);
}

void pointer_tracker::print(const void * to) {
  return pointer_tracker_impl::get().print(to);

}

#ifndef NDEBUG
void pointer_tracker::break_on(const size_t &count) {
  g_tracked_ptr_break = count;
}
#else
void pointer_tracker::break_on(const size_t &) {}
#endif
#endif
