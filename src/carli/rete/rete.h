#ifndef RETE_H
#define RETE_H

#include "rete_action.h"
#include "rete_existential.h"
#include "rete_existential_join.h"
#include "rete_filter.h"
#include "rete_join.h"
#include "rete_negation.h"
#include "rete_negation_join.h"
#include "rete_predicate.h"

namespace Rete {
  inline void __rete_node_size_check() {
    typedef typename Rete_Node::value_type pool_allocator_type;
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Action), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Existential), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Existential_Join), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Filter), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Join), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Negation), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Negation_Join), "Pool size suboptimal.");
    static_assert(sizeof(pool_allocator_type) >= sizeof(Rete_Predicate), "Pool size suboptimal.");
  }
}

#endif
