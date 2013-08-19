#include "value_queue.h"

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

    Value_List::List * const next = m_value_list->next();

    m_value_list->erase();
    m_value_list->destroy(m_value_list);

    m_value_list = next;
    if(!m_value_list)
      m_value_list_tail = nullptr;
  }
}
