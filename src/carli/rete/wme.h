#ifndef WME_H
#define WME_H

#include "symbol.h"
#include "utility.h"

namespace Rete {

  class WME {
  public:
    typedef std::array<Symbol_Ptr_C, 3> WME_Symbols;

    WME() {}
    WME(const Symbol_Ptr_C &first, const Symbol_Ptr_C &second, const Symbol_Ptr_C &third) {
      symbols[0] = first;
      symbols[1] = second;
      symbols[2] = third;
    }

    bool operator==(const WME &rhs) const {
      return *symbols[0] == *rhs.symbols[0] && *symbols[1] == *rhs.symbols[1] && *symbols[2] == *rhs.symbols[2];
    }

    WME_Symbols symbols;
  };

  typedef std::shared_ptr<const WME> WME_Ptr_C;
  typedef std::shared_ptr<WME> WME_Ptr;

  inline std::ostream & operator<<(std::ostream &os, const WME &wme) {
    return os << '(' << *wme.symbols[0] << " ^" << *wme.symbols[1] << ' ' << *wme.symbols[2] << ')';
  }

}

namespace std {
  template <> struct hash<Rete::WME> {
    size_t operator()(const Rete::WME &wme) const {
      return hash_combine(hash_combine(wme.symbols[0]->hash(), wme.symbols[1]->hash()), wme.symbols[2]->hash());
    }
  };
}

#endif
