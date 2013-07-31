#ifndef ZENI_SET_H
#define ZENI_SET_H

#include "linked_list.h"

namespace Zeni {

  template <typename TYPE, typename COMPARE = std::less<TYPE>>
  class Set : public Linked_List<TYPE> {
  public:
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef Set<TYPE, COMPARE> set_value_type;
    typedef set_value_type * set_pointer_type;

    Set(value_pointer_type value)
     : Linked_List<TYPE>(value)
    {
    }

    set_pointer_type insert(set_pointer_type &ptr) {
      return static_cast<set_pointer_type>(this->insert_in_order(ptr, false, COMPARE()));
    }
  };

}

#endif
