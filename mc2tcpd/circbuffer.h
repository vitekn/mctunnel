#ifndef MC2TCPD_CIRCBUFFER_H
#define MC2TCPD_CIRCBUFFER_H
#include <utility>
#include <array>
#include <cstring>

template <size_t N>
class CircBuffer {
    using BufferType = std::array<char, N+1>;
public:
    class AsArray {
        friend class CircBuffer;
    public:
        AsArray(char *ptr, size_t size):_ptr(ptr),_size(size){}

    private:
        char *_ptr;
        size_t _size;
    };

    template<class T,
             class std::enable_if_t<std::is_integral<std::remove_reference<T>>,int > = 0 >
    CircBuffer& operator<<(T value) {
        write(reinterpret_cast<char*>(&value), sizeof(T));
    }

    CircBuffer& operator<<(AsArray&& value) {
        write(value._ptr, value._size);
    }

    template<class T,
            class std::enable_if_t<std::is_integral<std::remove_reference<T>>,int > = 0 >
    CircBuffer& operator>>(T& value) {
        read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    CircBuffer& operator>>(AsArray&& value) {
        read(value._ptr, value._size);
    }

    void clear();

    size_t usedSize() const;
    size_t freeSize() const;

    bool isEmpty() const;
    bool isGood() const;

    constexpr size_t maxSize() const;

private:
    void write(char* ptr, size_t size);
    void read(char* ptr, size_t size);

    template<class F>
    void copy(BufferType::iterator& it, char *src, size_t size, F copyFn);

    BufferType _buffer;
    BufferType::iterator _dataBegin;
    BufferType::iterator _dataEnd;
    bool _isGood;

};

template <size_t N>
void CircBuffer<N>::write(char* ptr, size_t size)
{
    static auto cf = [](char* iterPtr, char* ptr, size_t size){
        std::memcpy(iterPtr, ptr, sz);
    };
    copy<decltype(cf)>(_dataEnd, ptr, size, cf);
}

template <size_t N>
void CircBuffer<N>::read(char* ptr, size_t size) {
    static auto cf = [](char* iterPtr, char* ptr, size_t size){
        std::memcpy(ptr, iterPtr, sz);
    };
    copy<decltype(cf)>(_dataBegin, ptr, size, cf);
}

template <size_t N>
template<class F>
void CircBuffer<N>::copy<F>(BufferType::iterator& it, char *dst, char *src, size_t size, F copyFn) {
    const int sz = std::min(std::distance(it, _buffer.end()), size);
    copyFn(&(*it), src, sz);
    size -= sz;
    it += sz;
    if (it == _buffer.end()){
        it = _buffer.begin();
    }
    if(size != 0) {
        copyFn(&(*it), ptr + sz, size);
        it+=size;
    }
}

template <size_t N>
size_t CircBuffer<N>::usedSize() const
{
    if (_dataBegin <= _dataEnd) {
        return std::distance(_dataBegin, _dataEnd);
    }
    return _buffer.max_size() - std::distance(_dataEnd,_dataBegin);
}

template <size_t N>
size_t CircBuffer<N>::freeSize() const
{
    return N - usedSize();
}

template <size_t N>
bool CircBuffer<N>::isEmpty() const
{
    return _dataBegin == _dataEnd;
}

template <size_t N>
bool CircBuffer<N>::isGood() const
{
    return _isGood;
}

template <size_t N>
void CircBuffer<N>::clear() const
{
    _is_good = true;
    _dataBegin = _buffer.begin();
    _dataEnd = _dataBegin;
}

template <size_t N>
constexpr size_t CircBuffer<N>::maxSize() const
{
    return N;
}



#endif //MC2TCPD_CIRCBUFFER_H
