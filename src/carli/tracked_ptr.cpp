/** tracked_ptr.cpp -- Mitchell Keith Bloch -- bazald@gmail.com -- 20130712 **/

#include "tracked_ptr.h"

#include <algorithm>
#include <unordered_map>

#ifndef NDEBUG
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
    if(to)
      address_to_pointer.insert(std::make_pair(to, from));
  }

  void clear_pointer(const void * to, const void * from) {
    if(to) {
      auto range = address_to_pointer.equal_range(to);
      auto atp = std::find_if(range.first, range.second, [from](const std::pair<const void *, const void *> &atp){return atp.second == from;});
      assert(atp != address_to_pointer.end());
      address_to_pointer.erase(atp);
    }
  }

  size_t count(const void * to) {
    auto range = address_to_pointer.equal_range(to);
    size_t count = 0;
    std::for_each(range.first, range.second, [&count](const std::pair<const void *, const void *> &){++count;});
    return count;
  }

//private:
  std::unordered_multimap<const void *, const void *> address_to_pointer;
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
#endif
