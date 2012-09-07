#ifndef ZENI_MEMORY_POOL_H
#define ZENI_MEMORY_POOL_H

#include <cstddef>
#include <cstdlib>
#include <map>

namespace Zeni {

  class Pool {
    Pool(const Pool &rhs);
    Pool & operator=(const Pool &rhs);

  public:
    Pool(const size_t &size_) throw()
      : size(std::max(sizeof(void *), size_)),
      available(0)
    {
    }

    ~Pool() throw() {
      clear();
    }

    size_t clear() throw() {
      size_t count = 0;
      while(available) {
        void * const ptr = available;
        available = *reinterpret_cast<void **>(reinterpret_cast<size_t *>(available) + 1);
        free(ptr);
        ++count;
      }
      return count;
    }

    void * get() throw() {
      if(available) {
        void * const ptr = available;
        available = *reinterpret_cast<void **>(reinterpret_cast<size_t *>(available) + 1);
        return reinterpret_cast<size_t *>(ptr) + 1;
      }
      else {
        void * ptr = malloc(sizeof(size_t) + size);

        if(ptr) {
          *reinterpret_cast<size_t *>(ptr) = size;
          return reinterpret_cast<size_t *>(ptr) + 1;
        }

        return 0;
      }
    }

    void give(void * const &ptr_) throw() {
      *reinterpret_cast<void **>(ptr_) = available;
      available = reinterpret_cast<size_t *>(ptr_) - 1;
    }

    static size_t size_of(const void * const &ptr_) throw() {
      return reinterpret_cast<const size_t *>(ptr_)[-1];
    }

    bool size_gte(const size_t &sz_) throw() {
      return size >= sz_;
    }

    bool size_gte(const void * const &ptr_) throw() {
      return size >= size_of(ptr_);
    }

  private:
    size_t size;
    void * available;
  };

  class Pool_Map {
    Pool_Map(const Pool_Map &rhs);
    Pool_Map & operator=(const Pool_Map &rhs);

    Pool_Map() throw() {}
    ~Pool_Map() throw() {
      for(std::map<size_t, Pool *>::iterator it = m_pools.begin(), iend = m_pools.end(); it != iend; ++it)
        delete it->second;
    }

  public:
    static Pool_Map & get() {
      static Pool_Map pool_map;
      return pool_map;
    };

    size_t clear() throw() {
      size_t count = 0;
      for(std::map<size_t, Pool *>::iterator it = m_pools.begin(), iend = m_pools.end(); it != iend; ++it)
        count += it->second->clear();
      return count;
    }

    Pool & get_Pool(const size_t &size_) {
      Pool * &pool = m_pools[size_];
      if(!pool)
        pool = new Pool(size_);

      return *pool;
    }

    Pool & get_Pool(void * const &ptr_) {
      return get_Pool(Pool::size_of(ptr_));
    }

  private:
    std::map<size_t, Pool *> m_pools;
  };

  template <typename TYPE>
  class Pool_Allocator {
  public:
    virtual ~Pool_Allocator() {}

    void * operator new(size_t sz_) {
      Pool &p = pool->size_gte(sz_) ? *pool : pool_map->get_Pool(sz_);

      void * ptr = p.get();
      if(ptr)
        return ptr;

      if(pool_map->clear()) {
        ptr = p.get();
        if(ptr)
          return ptr;
      }

      throw std::bad_alloc();
    }

    void operator delete(void * ptr_) throw() {
      Pool &p = pool->size_gte(ptr_) ? *pool : pool_map->get_Pool(ptr_);

      p.give(ptr_);
    }

  private:
    static Pool_Map * pool_map;
    static Pool * pool;
  };

  template <typename TYPE>
  Pool_Map * Pool_Allocator<TYPE>::pool_map = &Pool_Map::get();
  template <typename TYPE>
  Pool * Pool_Allocator<TYPE>::pool = &pool_map->get_Pool(sizeof(TYPE));

  void register_new_handler(const bool &force_reregister = false);

}

#endif
