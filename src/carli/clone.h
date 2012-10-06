#ifndef ZENI_CLONEABLE_H
#define ZENI_CLONEABLE_H

namespace Zeni {

  template <typename TYPE>
  class Cloneable {
  public:
    virtual TYPE * clone() const = 0;
  };

}

#endif
