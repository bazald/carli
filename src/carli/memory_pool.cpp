#include "memory_pool.h"

namespace Zeni {

  static bool g_unregistered = true;
  static std::new_handler g_old_new_handler = 0;

  static void new_handler() {
    if(!Pool_Map::get().clear())
      if(g_old_new_handler)
        g_old_new_handler();
      else
        throw std::bad_alloc();
  }

  void register_new_handler(const bool &force_reregister) {
    if(g_unregistered || force_reregister) {
      g_old_new_handler = std::set_new_handler(new_handler);
      g_unregistered = false;
    }
  }

}
