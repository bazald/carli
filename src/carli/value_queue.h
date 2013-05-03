#ifndef VALUE_QUEUE_H
#define VALUE_QUEUE_H

#include "linked_list.h"
#include "memory_pool.h"
#include "value.h"

class Value_Queue {
  Value_Queue(const Value_Queue &) = delete;
  Value_Queue operator=(const Value_Queue &) = delete;

  class Value_List;
  class Value_List : public Zeni::Pool_Allocator<Value_List> {
  public:
    typedef Zeni::Linked_List<Value_List> List;

    Value_List(const double &value_ = 0.0)
     : value(value_),
     list(this)
    {
    }

    Value value;
    List list;
  };

public:
  Value_Queue()
  {
  }

  ~Value_Queue() {
    m_value_list->destroy(m_value_list);
  }

  void push(const double &value) {
    Value_List::List * const entry = &(new Value_List(value))->list;

    entry->insert_after(m_value_list_tail);
    m_value_list_tail = entry;
    if(!m_value_list)
      m_value_list = entry;

    m_mean.contribute(entry->get()->value);

    ++m_size;
  }
  
  void pop() {
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
  
  size_t size() const {
    return m_size;
  }

  Mean mean() const {
    return m_mean;
  }

private:
  Value_List::List * m_value_list = nullptr;
  Value_List::List * m_value_list_tail = nullptr;
  Mean m_mean;
  size_t m_size = 0lu;
};

#endif
