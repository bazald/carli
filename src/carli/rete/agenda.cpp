#include "agenda.h"

#include "rete_action.h"

namespace Rete {

  void Agenda::insert_action(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token) {
    agenda.emplace_back(action, wme_token, true);
    run();
  }

  void Agenda::insert_retraction(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token) {
    Rete_Action_to_Agenda::retraction(*action)(*action, *wme_token);
  }

  void Agenda::lock() {
    assert(!m_locked);
    m_locked = true;
    m_manually_locked = true;
  }

  void Agenda::unlock() {
    assert(m_manually_locked);
    m_locked = false;
    m_manually_locked = false;
  }

  void Agenda::run() {
    if(m_locked)
      return;
    m_locked = true;

    while(!agenda.empty()) {
      const auto front_iterator = agenda.begin();
      const auto &front = *front_iterator;
      if(std::get<2>(front))
        Rete_Action_to_Agenda::action(*std::get<0>(front))(*std::get<0>(front), *std::get<1>(front));
      else
        Rete_Action_to_Agenda::retraction(*std::get<0>(front))(*std::get<0>(front), *std::get<1>(front));
      agenda.erase(front_iterator);
    }

    assert(m_locked);
    m_locked = false;
  }

}
