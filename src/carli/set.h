#ifndef ZENI_SET_H
#define ZENI_SET_H

#include "linked_list.h"

namespace Zeni {

  template<typename TYPE, typename COMPARE = std::less<TYPE> >
  class Set : public Linked_List<TYPE> {
  public:
    typedef TYPE value_type;
    typedef Linked_List<value_type> list_type;
    typedef typename list_type::value_pointer_type value_pointer_type;
    typedef typename list_type::list_pointer_type list_pointer_type;

    Set(value_pointer_type value)
     : list_type(value)
    {
    }

    std::pair<list_pointer_type, bool> insert(list_pointer_type &ptr) {
      return list_type::insert_in_order(ptr, false, compare);
    }

  private:
    COMPARE compare;
  };

}

#endif
