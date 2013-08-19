#ifndef RETE_AGENDA_H
#define RETE_AGENDA_H

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
    Agenda(Agenda * const &block_on_ = nullptr);

    void insert(const Rete_Node * const &node, const std::function<void ()> &action);

    void remove(const Rete_Node * const &node);

    size_t run();

  private:
    std::list<std::pair<const Rete_Node *, std::function<void ()>>> agenda;
    bool agenda_ready = true;

    Agenda * block_on;
  };

}

#endif
