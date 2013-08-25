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

    void insert_action(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token) {
      agenda.emplace_back(action, wme_token, true);
      run();
    }

    void insert_retraction(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token) {
      agenda.emplace_front(action, wme_token, false);
      run();
    }

    void lock();
    void unlock();
    void run();

  private:
    std::list<std::tuple<Rete_Action_Ptr_C, WME_Token_Ptr_C, bool>, Zeni::Pool_Allocator<std::tuple<Rete_Action_Ptr_C, WME_Token_Ptr_C, bool>>> agenda;
    bool m_locked = false;
    bool m_manually_locked = false;
  };

}

#endif
