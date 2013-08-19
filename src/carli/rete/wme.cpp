#include "wme.h"

namespace Rete {

  WME::WME(const Symbol_Ptr_C &first, const Symbol_Ptr_C &second, const Symbol_Ptr_C &third) {
    symbols[0] = first;
    symbols[1] = second;
    symbols[2] = third;
  }

  bool WME::operator==(const WME &rhs) const {
    return *symbols[0] == *rhs.symbols[0] && *symbols[1] == *rhs.symbols[1] && *symbols[2] == *rhs.symbols[2];
  }

}

std::ostream & operator<<(std::ostream &os, const Rete::WME &wme) {
  return os << '(' << *wme.symbols[0] << " ^" << *wme.symbols[1] << ' ' << *wme.symbols[2] << ')';
}
