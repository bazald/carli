#ifndef ZENI_SET_H
#define ZENI_SET_H

#include "linked_list.h"

namespace Zeni {

  template<typename TYPE, typename COMPARE = std::less<TYPE> >
  class Set : public Linked_List<TYPE> {
  public:
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef Set<TYPE, COMPARE> list_value_type;
    typedef list_value_type * list_pointer_type;

    Set(value_pointer_type value)
     : Linked_List<TYPE>(value)
    {
    }

    std::pair<list_pointer_type, bool> insert(list_pointer_type &ptr) {
      auto rv = this->insert_in_order(ptr, false, COMPARE());

      return std::make_pair(reinterpret_cast<list_pointer_type>(rv.first), rv.second);
    }
  };

}

#endif
