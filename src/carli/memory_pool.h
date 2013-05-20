#ifndef ZENI_MEMORY_POOL_H
#define ZENI_MEMORY_POOL_H

#include <cstddef>
#include <cstdlib>
#include <map>

namespace Zeni {

  class Pool {
    Pool(const Pool &rhs) = delete;
    Pool & operator=(const Pool &rhs) = delete;

  public:
    Pool(const size_t &size_) throw()
      : size(std::max(sizeof(void *), size_)),
      available(nullptr)
    {
    }

    ~Pool() throw() {
      clear();
    }

    /// free any cached memory blocks; return a count of the number of blocks freed
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

    /// get a cached memory block or allocate one as needed
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

        return nullptr;
      }
    }

    /// return a memory block to be cached (and eventually freed)
    void give(void * const &ptr_) throw() {
#ifndef NDEBUG
      for(unsigned char * pp = reinterpret_cast<unsigned char *>(ptr_), * pend = pp + size_of(ptr_); pp != pend; ++pp) {
        *pp = 0xEF;
        if(++pp == pend)
          break;
        *pp = 0xBE;
        if(++pp == pend)
          break;
        *pp = 0xAD;
        if(++pp == pend)
          break;
        *pp = 0xDE;
      }
#endif
      *reinterpret_cast<void **>(ptr_) = available;
      available = reinterpret_cast<size_t *>(ptr_) - 1;
    }

    /// get the size of a memory block allocated with an instance of Pool
    static size_t size_of(const void * const &ptr_) throw() {
      return reinterpret_cast<const size_t *>(ptr_)[-1];
    }

    /// check if the size of the memory block provided by this Pool is greater than or equal to sz_
    bool size_gte(const size_t &sz_) throw() {
      return size >= sz_;
    }

    /// check if the size of the memory block provided by this Pool is greater than or equal to the size of the memory block pointed to by ptr_
    bool size_gte(const void * const &ptr_) throw() {
      return size >= size_of(ptr_);
    }

  private:
    size_t size;
    void * available;
  };

  class Pool_Map {
    Pool_Map(const Pool_Map &rhs) = delete;
    Pool_Map & operator=(const Pool_Map &rhs) = delete;

    Pool_Map() throw() {}
    ~Pool_Map() throw() {
      for(auto &pool : m_pools)
        delete pool.second;
    }

  public:
    static Pool_Map & get() {
      static Pool_Map pool_map;
      return pool_map;
    };

    /// free memory blocks from every Pool; return a count of all freed blocks
    size_t clear() throw() {
      size_t count = 0;
      for(std::map<size_t, Pool *>::iterator it = m_pools.begin(), iend = m_pools.end(); it != iend; ++it)
        count += it->second->clear();
      return count;
    }

    /// get a memory Pool that allocates memory blocks of a given size
    Pool & get_Pool(const size_t &size_) {
      Pool * &pool = m_pools[size_];
      if(!pool)
        pool = new Pool(size_);

      return *pool;
    }

    /// get a memory Pool that allocates memory blocks the size of the block pointed to by ptr_
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

    /**
     * get the default Pool if sz_ is less than or equal to the size of TYPE, otherwise a non-default pool;
     * then get a cached or freshly allocated memory block from the Pool;
     * if this fails, clear every Pool and try again;
     * if it fails again, throw std::bad_alloc;
     */
    static void * operator new(size_t sz_) {
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

    /// return a memory block to the appropriate memory Pool
    static void operator delete(void * ptr_) throw() {
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
  Pool * Pool_Allocator<TYPE>::pool = &Pool_Map::get().get_Pool(sizeof(TYPE));

  void register_new_handler(const bool &force_reregister = false);

}

#endif
