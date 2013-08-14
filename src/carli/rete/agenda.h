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
    Agenda(Agenda * const &block_on_ = nullptr)
      : block_on(block_on_)
    {
    }

    void insert(const Rete_Node * const &node, const std::function<void ()> &action) {
      agenda.push_back(std::make_pair(node, action));
      if(agenda_ready)
        run();
      else
        wait();
    }

    void remove(const Rete_Node * const &node) {
      for(auto it = agenda.begin(), iend = agenda.end(); it != iend; ) {
        if(it->first == node)
          agenda.erase(it++);
        else
          ++it;
      }
    }

    size_t run() {
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

  private:
    void wait() {}

    std::list<std::pair<const Rete_Node *, std::function<void ()>>> agenda;
    bool agenda_ready = true;

    Agenda * block_on;
  };

}

#endif
