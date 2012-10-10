#ifndef ZENI_MAP_H
#define ZENI_MAP_H

#include "linked_list.h"
#include <stdexcept>

namespace Zeni {

  template<typename KEY, typename TYPE, typename COMPARE = std::less<KEY> >
  class Map : public Linked_List<TYPE> {
  public:
    typedef KEY key_type;
    typedef TYPE value_type;
    typedef value_type * value_pointer_type;
    typedef Map<KEY, TYPE, COMPARE> list_value_type;
    typedef list_value_type * list_pointer_type;

    Map(value_pointer_type value, const key_type &key_ = key_type())
     : Linked_List<TYPE>(value),
     key(key_)
    {
    }

    std::pair<list_pointer_type, bool> insert(list_pointer_type &ptr) {
      auto rv = this->insert_in_order(reinterpret_cast<Linked_List<TYPE> * &>(ptr), false, [this](const TYPE &lhs, const TYPE &rhs) {
        const Map * const lm = reinterpret_cast<const Map<KEY, TYPE, COMPARE> *>(reinterpret_cast<const char *>(&lhs) + this->offset());
        const Map * const rm = reinterpret_cast<const Map<KEY, TYPE, COMPARE> *>(reinterpret_cast<const char *>(&rhs) + this->offset());

        return COMPARE()(lm->key, rm->key);
      });

      return std::make_pair(reinterpret_cast<list_pointer_type>(rv.first), rv.second);
    }

    const key_type & get_key() const {
      return key;
    }

    void set_key(const key_type &key_) {
      if(this->prev() && COMPARE()(key_, static_cast<list_pointer_type>(this->prev())->key))
        throw std::runtime_error("Illegal key modification in Zeni::Map::set_key.");
      if(this->next() && COMPARE()(static_cast<list_pointer_type>(this->next())->key, key_))
        throw std::runtime_error("Illegal key modification in Zeni::Map::set_key.");
      key = key_;
    }

  private:
    key_type key;
  };

}

#endif
