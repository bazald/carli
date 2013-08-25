#include "agenda.h"

namespace Rete {

  Agenda::Agenda() {
  }

  void Agenda::insert_front(const Rete_Node_Ptr_C &node, const std::function<void ()> &action) {
    agenda.push_front(std::make_pair(node, action));
    run();
  }

  void Agenda::insert_back(const Rete_Node_Ptr_C &node, const std::function<void ()> &action) {
    agenda.push_back(std::make_pair(node, action));
    run();
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
      auto front = agenda.front();
      agenda.pop_front();
      front.second();
    }

    assert(m_locked);
    m_locked = false;
  }

}
