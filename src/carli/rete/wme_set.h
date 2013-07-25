#ifndef WME_SET_H
#define WME_SET_H

#include "wme.h"
#include "utility.h"

namespace Rete {

  class WME_Set {
  public:
    std::unordered_set<WME> wmes;
  };

  inline std::ostream & operator<<(std::ostream &os, const WME_Set &wme_set) {
    os << '{' << std::endl;
    for(const WME &wme : wme_set.wmes)
      os << "  " << wme << std::endl;
    os << '}';
    return os;
  }

}

namespace std {
  template <> struct hash<Rete::WME_Set> {
    size_t operator()(const Rete::WME_Set &wme_set) const {
      return hash<std::unordered_set<Rete::WME>>()(wme_set.wmes);
    }
  };
}

#endif
