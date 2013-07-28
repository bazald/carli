#ifndef WME_VECTOR_H
#define WME_VECTOR_H

#include "wme.h"
#include "utility.h"
#include "../memory_pool.h"

namespace Rete {

  class WME_Vector;
  class WME_Vector : public Zeni::Pool_Allocator<WME_Vector> {
  public:
    std::vector<WME> wmes;

    bool operator==(const WME_Vector &rhs) const {
      return wmes == rhs.wmes;
    }
  };

  typedef std::shared_ptr<const WME_Vector> WME_Vector_Ptr_C;
  typedef std::shared_ptr<WME_Vector> WME_Vector_Ptr;
  typedef std::pair<size_t, uint8_t> WME_Vector_Index;
  typedef std::pair<WME_Vector_Index, WME_Vector_Index> WME_Binding;
  typedef std::unordered_set<WME_Binding> WME_Bindings;

  inline std::ostream & operator<<(std::ostream &os, const WME_Vector &wme_vector) {
    os << '{' << std::endl;
    for(const WME &wme : wme_vector.wmes)
      os << "  " << wme << std::endl;
    os << '}';
    return os;
  }

}

namespace std {
  template <> struct hash<Rete::WME_Vector> {
    size_t operator()(const Rete::WME_Vector &wme_vector) const {
      return hash<std::vector<Rete::WME>>()(wme_vector.wmes);
    }
  };
}

#endif
