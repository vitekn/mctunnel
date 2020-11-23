#ifndef MC2TCPD_OBJECTPOOL_H
#define MC2TCPD_OBJECTPOOL_H
#include <memory>
#include <mutex>
#include <type_traits>

template<class T>
class ObjectPool
{
    using ObjectStorage = std::aligned_storage<sizeof(T), alignof(T)>::type ;

public:
    ObjectPool (size_t num): _storage(std::unique_ptr<ObjectStorage[]>(new ObjectStorage [num]))
    {
        _freeElements.reserve(num);
        for (size_t i=0; i<num; ++i){
            _freeElements.emplace_back(_storage.get() + i));
        }
    }

    T* get()
    {
        T* ptr = nullptr;
        {
            const std::lock_guard<std::mutex> lock(_mutex);
            if (_freeElements.empty()) return nullptr;
            ptr = reinterpret_cast<T*>(_freeElements.back());
            _freeElements.pop_back();
        }
        return ptr;

    }

    void release(T* ptr)
    {
        const std::lock_guard<std::mutex> lock(_mutex);
        _freeElements.push_back(reinterpret_cast<ObjectStorage*>(ptr));
    }

private:
    std::unique_ptr<ObjectStorage[]> _storage;
    vector<ObjectStorage*> _freeElements;
    std::mutex _mutex;
};

#endif //MC2TCPD_OBJECTPOOL_H
