#ifndef RETE_AGENDA_H
#define RETE_AGENDA_H

#include "rete_node.h"

#include <cassert>
#include <cstddef>
#include <functional>
#include <list>

namespace Rete {

  class Rete_Node;

  class RETE_LINKAGE Agenda {
    Agenda(Agenda &);
    Agenda & operator=(Agenda &);

  public:
    class Locker {
      Locker(const Locker &rhs);
      Locker & operator=(const Locker &rhs);

    public:
      Locker(Agenda &agenda)
       : m_agenda(agenda)
      {
        m_agenda.lock();
      }

      ~Locker() {
        m_agenda.unlock();
        m_agenda.run();
      }

    private:
      Agenda &m_agenda;
    };

    Agenda() {}

    void insert_action(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token);
    void insert_retraction(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token);

    void lock();
    void unlock();
    void run();

  private:
    std::unordered_set<std::pair<Rete_Action_Ptr_C, WME_Token_Ptr_C>> agenda;
    int64_t m_locked = 0;
    int64_t m_manually_locked = 0;
  };

}

#endif
