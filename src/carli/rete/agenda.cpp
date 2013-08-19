#include "agenda.h"

namespace Rete {

  Agenda::Agenda(Agenda * const &block_on_)
    : block_on(block_on_)
  {
  }

  void Agenda::insert(const Rete_Node * const &node, const std::function<void ()> &action) {
    agenda.push_back(std::make_pair(node, action));
    if(agenda_ready)
      run();
  }

  void Agenda::remove(const Rete_Node * const &node) {
    for(auto it = agenda.begin(), iend = agenda.end(); it != iend; ) {
      if(it->first == node)
        agenda.erase(it++);
      else
        ++it;
    }
  }

  size_t Agenda::run() {
    assert(agenda_ready);
    agenda_ready = false;
    size_t count = 0u;

    if(!block_on || block_on->agenda_ready) {
      for(;;) {
        if(block_on)
          count += block_on->run();

        if(agenda.empty())
          break;

        auto fcn = agenda.front().second;
        agenda.pop_front();
        fcn();
        ++count;
      }
    }

    agenda_ready = true;
    return count;
  }

}
