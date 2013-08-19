#ifndef VALUE_QUEUE_H
#define VALUE_QUEUE_H

#include "utility/linked_list.h"
#include "utility/memory_pool.h"

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

  ~Value_Queue();

  void push(const double &value);

  void pop();

  size_t size() const {
    return m_size;
  }

  const Mean & mean() const {
    return m_mean;
  }

private:
  Value_List::List * m_value_list = nullptr;
  Value_List::List * m_value_list_tail = nullptr;
  Mean m_mean;
  size_t m_size = 0lu;
};

#endif
