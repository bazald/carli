#include "value_queue.h"

namespace Carli {

  Value_Queue::~Value_Queue() {
    m_value_list->destroy(m_value_list);
  }

  void Value_Queue::push(const double &value) {
    Value_List::List * const entry = &(new Value_List(value))->list;

    entry->insert_after(m_value_list_tail);
    m_value_list_tail = entry;
    if(!m_value_list)
      m_value_list = entry;

    m_mean.contribute(entry->get()->value);

    ++m_size;
  }

  void Value_Queue::pop() {
    if(m_value_list) {
      --m_size;

      m_mean.uncontribute(m_value_list->get()->value);
      
      tracked_ptr<Value_List::List> head = m_value_list;

      head->erase_from(m_value_list);
      head->destroy(head);

      if(!m_value_list)
        m_value_list_tail = nullptr;
    }
  }

}
