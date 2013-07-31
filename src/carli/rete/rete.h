#ifndef RETE_H
#define RETE_H

#include "rete_action.h"
#include "rete_existential.h"
#include "rete_filter.h"
#include "rete_join.h"
#include "rete_negation.h"
#include "rete_predicate.h"

namespace Rete {
  inline void __rete_node_size_check() {
    typedef Rete_Join Pool;
    static_assert(sizeof(Pool) >= sizeof(Rete_Action), "Pool size suboptimal.");
    static_assert(sizeof(Pool) >= sizeof(Rete_Existential), "Pool size suboptimal.");
    static_assert(sizeof(Pool) >= sizeof(Rete_Filter), "Pool size suboptimal.");
    static_assert(sizeof(Pool) >= sizeof(Rete_Join), "Pool size suboptimal.");
    static_assert(sizeof(Pool) >= sizeof(Rete_Negation), "Pool size suboptimal.");
    static_assert(sizeof(Pool) >= sizeof(Rete_Predicate), "Pool size suboptimal.");
  }
}

#endif
