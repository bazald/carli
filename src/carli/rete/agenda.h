#ifndef RETE_AGENDA_H
#define RETE_AGENDA_H

#include "rete_node.h"

#include <cassert>
#include <cstddef>
#include <functional>
#include <list>

namespace Rete {

  class Rete_Node;

  class Agenda {
    Agenda(Agenda &);
    Agenda & operator=(Agenda &);

  public:
    Agenda();

    void insert_front(const Rete_Node_Ptr_C &node, const std::function<void ()> &action);
    void insert_back(const Rete_Node_Ptr_C &node, const std::function<void ()> &action);

    void lock();
    void unlock();
    void run();

  private:
    std::list<std::pair<std::shared_ptr<const Rete_Node>, std::function<void ()>>, Zeni::Pool_Allocator<std::pair<std::shared_ptr<const Rete_Node>, std::function<void ()>>>> agenda;
    bool m_locked = false;
    bool m_manually_locked = false;
  };

}

#endif
