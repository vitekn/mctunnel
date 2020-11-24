#ifndef MC2TCPD_POOLALLOCATOR_H
#define MC2TCPD_POOLALLOCATOR_H

#include "objectpool.h"
#include <memory>

template <class T>
class PoolAllocator
{
public:
    using value_type = T;
    explicit PoolAllocator (std::shared_ptr<ObjectPool<T>> pool): _pool(pool){}

    T* allocate(size_t n) {
        if (n != 1) {
            throw std::bad_alloc();
        }
        T* ptr = _pool->get();
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate( T* p, std::size_t n)
    {
        _pool->release(p);
    }

private:
    std::shared_ptr<ObjectPool<T>> _pool;
};

#endif //MC2TCPD_POOLALLOCATOR_H
