#ifndef ZENI_MEMORY_POOL_H
#define ZENI_MEMORY_POOL_H

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <inttypes.h>
#include <limits>
#include <unordered_map>

#include "linkage.h"

namespace Zeni {

  class UTILITY_LINKAGE Pool {
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
#ifndef NDEBUG
        fill(reinterpret_cast<size_t *>(ptr) + 1, 0xFA57F00D);
#endif
        return reinterpret_cast<size_t *>(ptr) + 1;
      }
      else {
        void * ptr = malloc(sizeof(size_t) + size);

        if(ptr) {
          *reinterpret_cast<size_t *>(ptr) = size;
#ifndef NDEBUG
          fill(reinterpret_cast<size_t *>(ptr) + 1, 0xED1B13BF);
#endif
          return reinterpret_cast<size_t *>(ptr) + 1;
        }

        return nullptr;
      }
    }

    /// return a memory block to be cached (and eventually freed)
    void give(void * const &ptr_) throw() {
#ifndef NDEBUG
      fill(ptr_, 0xDEADBEEF);
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
#ifndef NDEBUG
    void fill(void * const dest, const uint32_t pattern) {
      unsigned char * dd = reinterpret_cast<unsigned char *>(dest);
      const unsigned char * const dend = dd + size_of(dest);

      while(dend - dd > 3) {
        *reinterpret_cast<uint32_t *>(dd) = pattern;
        dd += 4;
      }
      if(dd == dend)
        return;
      *dd = reinterpret_cast<const unsigned char *>(pattern)[3];
      if(++dd == dend)
        return;
      *dd = reinterpret_cast<const unsigned char *>(pattern)[2];
      if(++dd == dend)
        return;
      *dd = reinterpret_cast<const unsigned char *>(pattern)[1];
    }
#endif

    size_t size;
    void * available;
  };

  class UTILITY_LINKAGE Pool_Map {
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
      for(std::unordered_map<size_t, Pool *>::iterator it = m_pools.begin(), iend = m_pools.end(); it != iend; ++it)
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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
    std::unordered_map<size_t, Pool *> m_pools;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  };

#ifndef DISABLE_POOL_ALLOCATOR
  template <typename TYPE>
  class Pool_Allocator {
  public:
    typedef TYPE value_type;
    typedef TYPE * pointer;
    typedef const TYPE * const_pointer;
    typedef TYPE & reference;
    typedef const TYPE & const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;
    template <typename NEWTYPE> struct rebind {typedef Pool_Allocator<NEWTYPE> other;};

    Pool_Allocator() {}

    template <typename OTHERTYPE>
    Pool_Allocator(const Pool_Allocator<OTHERTYPE> &) {}

    /**
     * get the default Pool if sz_ is less than or equal to the size of TYPE, otherwise a non-default pool;
     * then get a cached or freshly allocated memory block from the Pool;
     * if this fails, clear every Pool and try again;
     * if it fails again, throw std::bad_alloc;
     */
    static void * operator new(size_t sz_) {
      return allocate(sz_);
    }

    /// return a memory block to the appropriate memory Pool
    static void operator delete(void * ptr_) throw() {
      deallocate(reinterpret_cast<pointer>(ptr_), sizeof(TYPE));
    }

    static pointer address(reference x) {
      return reinterpret_cast<pointer>(&reinterpret_cast<char &>(x));
    }

    static const_pointer address(const_reference x) {
      return reinterpret_cast<const_pointer>(&reinterpret_cast<const char &>(x));
    }

    static pointer allocate(size_type sz_, std::allocator<void>::const_pointer /*hint*/ = 0) {
      Pool &p = pool->size_gte(sz_) ? *pool : pool_map->get_Pool(sz_);

      void * ptr = p.get();
      if(ptr)
        return pointer(ptr);

      if(pool_map->clear()) {
        ptr = p.get();
        if(ptr)
          return pointer(ptr);
      }

      throw std::bad_alloc();
    }

    static void deallocate(pointer ptr_, size_type /*n*/) throw() {
      Pool &p = pool->size_gte(ptr_) ? *pool : pool_map->get_Pool(ptr_);

      p.give(ptr_);
    }

    static size_type max_size() {
      return std::numeric_limits<size_type>::max();
    }

    template<class NEWTYPE, class... ARGS>
    static void construct(NEWTYPE *p, ARGS&&... args) {
      ::new((void *)p) NEWTYPE(std::forward<ARGS>(args)...);
    }

    template<class NEWTYPE>
    static void destroy(NEWTYPE *p) {
      p->~NEWTYPE();
    }

    template <typename RHSTYPE>
    bool operator==(const Pool_Allocator<RHSTYPE> &rhs) const {
      return pool == rhs.pool;
    }

    template <typename RHSTYPE>
    bool operator!=(const Pool_Allocator<RHSTYPE> &rhs) const {
      return pool != rhs.pool;
    }

  private:
    static Pool_Map * pool_map;
    static Pool * pool;
  };

  template <typename TYPE>
  Pool_Map * Pool_Allocator<TYPE>::pool_map = &Pool_Map::get();
  template <typename TYPE>
  Pool * Pool_Allocator<TYPE>::pool = &Pool_Map::get().get_Pool(sizeof(TYPE));
#else
  template <typename TYPE>
  using Pool_Allocator = std::allocator<TYPE>;
#endif

  UTILITY_LINKAGE void register_new_handler(const bool &force_reregister = false);

}

#endif
