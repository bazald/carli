#include "agenda.h"

#include "rete_action.h"

namespace Rete {

  void Agenda::insert_action(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token) {
//#ifdef DEBUG_OUTPUT
//    std::cerr << "Inserting " << *wme_token << std::endl;
//#endif
    agenda.emplace_back(action, wme_token, true);
    run();
  }

  void Agenda::insert_retraction(const Rete_Action_Ptr_C &action, const WME_Token_Ptr_C &wme_token) {
    Rete_Action_to_Agenda::retraction(*action)(*action, *wme_token);
  }

  void Agenda::lock() {
    ++m_locked;
    ++m_manually_locked;
  }

  void Agenda::unlock() {
    assert(m_manually_locked);
    --m_locked;
    --m_manually_locked;
  }

  void Agenda::run() {
    if(m_locked)
      return;
    ++m_locked;

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
    --m_locked;
  }

}
