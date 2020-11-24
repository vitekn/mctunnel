#ifndef MC2TCPD_CHUNCKEDSTREAM_H
#define MC2TCPD_CHUNCKEDSTREAM_H
#include <array>
#include <memory>
#include <deque>
#include <mutex>
#include <vector>
#include <cstring>

//TODO deq mutex
class AsArray {
public:
    AsArray(char *ptr, size_t size):_ptr(ptr),_size(size){}

    char *_ptr;
    size_t _size;
};

template <size_t SIZE>
struct Chunk
{
    constexpr static const int MAX_SIZE = SIZE;
    Chunk():size(0), readPos(0){}

    size_t size;
    size_t readPos;
    std::array<char, SIZE> buf;
};

struct NoMutex{
    void lock(){}
    void unlock(){}
    bool try_lock() {return true;}

    NoMutex(const NoMutex&) = delete;
    NoMutex(NoMutex&&) = delete;

    NoMutex& operator=(const NoMutex&) = delete;
    NoMutex& operator=(NoMutex&&) = delete;
};

template <class CHUNK, class Allocator = std::allocator<CHUNK>, class MUTEX = NoMutex>
class ChunkedStream {
    struct Deleter{
        Deleter():_alloc(nullptr){}
        explicit Deleter(Allocator* alloc) noexcept
                :_alloc(alloc)
        {}
        void operator()(CHUNK *ptr)
        {
            std::allocator_traits<Allocator>::destroy(*_alloc, ptr);
            std::allocator_traits<Allocator>::deallocate(*_alloc, ptr, 1);
        }
        Allocator* _alloc;
    };

public:

    using ChunkPtr = std::unique_ptr<CHUNK, Deleter>;


    explicit ChunkedStream(const Allocator& allocator=Allocator())
    : _queue()
    , _allocator(allocator)
    , _currentChunk(createChunk())
    , _size(0)
    {}

/*    template<class T,
            class std::enable_if_t<std::is_integral<std::remove_reference<T>>,int > = 0 >
    ChunkedStream& operator>>(T& value) {
        read(reinterpret_cast<char*>(&value), sizeof(T));
        return *this;
    }*/

    ChunkedStream& operator>>(ChunkPtr& value) {
        std::lock_guard<MUTEX> lock(_mutex);
        if (_queue.empty()) {
            value = ChunkPtr();
        } else {
            value = std::move(_queue.front());
            _queue.pop_front();
        }
        return *this;
    }

    void revertChunk(ChunkPtr&& ptr){
        std::lock_guard<MUTEX> lock(_mutex);
        _queue.emplace_front(std::move(ptr));
    }

    [[nodiscard]] size_t size() const { return _size; }
    [[nodiscard]] bool empty() const { return _size == 0;}

    void flush()
    {
        if (_currentChunk != nullptr && _currentChunk->size != 0) {
            std::lock_guard<MUTEX> lock(_mutex);
            _queue.emplace_back(std::move(_currentChunk));
        }
    }

    bool write(const char* buf, size_t size) noexcept
    {
        const size_t chunkSize = _currentChunk->size;

        _size += size;
        std::vector<ChunkPtr> chunksToAdd;

        while (size != 0) {
            size_t sz = std::min(size, CHUNK::MAX_SIZE - _currentChunk->size);
            std::memcpy(_currentChunk->buf.data()+_currentChunk->size, buf, sz);
            size -= sz;
            buf += sz;
            _currentChunk->size += sz;
            if (_currentChunk->size == CHUNK::MAX_SIZE) {
                try {
                    ChunkPtr newChunk = createChunk();
                    chunksToAdd.emplace_back(std::move(_currentChunk));
                    _currentChunk = std::move(newChunk);

                } catch (const std::bad_alloc&) {
                    if (!chunksToAdd.empty()) {
                        _currentChunk = std::move(chunksToAdd.front());
                    }
                    _currentChunk->size = chunkSize;
                    return false;
                }
            }
        }
        {
            std::lock_guard <MUTEX> lock(_mutex);
            for (auto &ch: chunksToAdd) {
                _queue.emplace_back(std::move(ch));
            }
        }
        return true;
    }

    size_t read(char* buf, size_t size)
    {
        size_t readSz = 0;
        CHUNK *chunkToRead;
        {
            std::lock_guard <MUTEX> lock(_mutex);
            chunkToRead = _queue.empty() ? 0 : _queue.front().get();
        }
        while (chunkToRead && readSz != size) {
            int sz = min(size, chunkToRead->size);
            memcpy(buf, chunkToRead->buf.data() + chunkToRead->readPos, sz);
            readSz += sz;
            chunkToRead->readPos += sz;
            chunkToRead->size -= sz;
            if (chunkToRead->size == 0) {
                std::lock_guard <MUTEX> lock(_mutex);
                _queue.pop_front();
                chunkToRead = _queue.empty() ? 0 : _queue.front().get();
            }
        }
        _size -= readSz;
        return readSz;
    }

private:

    ChunkPtr createChunk(){
        return ChunkPtr(constructChunk(), Deleter(&_allocator));
    }

    CHUNK* constructChunk()
    {
        CHUNK* ch = std::allocator_traits<Allocator>::allocate(_allocator, 1U);
        std::allocator_traits<Allocator>::construct(_allocator, ch);
        return ch;
    }

    std::deque<ChunkPtr> _queue;
    ChunkPtr _currentChunk;
    Allocator _allocator;
    size_t _size;
    MUTEX _mutex;
};

#endif //MC2TCPD_CHUNCKEDSTREAM_H
