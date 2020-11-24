#ifndef ZSOCKET_H
#define ZSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <socketenums.h>
#include <ipaddress.h>
#include <tuple>
#include <chrono>

namespace Networking
{
namespace PosixImplementation
{

enum class MsgFlags: uint32_t {
    WaitAll = MSG_WAITALL,
    NoSignal = MSG_NOSIGNAL,
};
class FlagUnion
{
public:
    FlagUnion():_value(0){}

    FlagUnion(std::initializer_list<MsgFlags> flags)
            :_value(0)
    {
        for(auto f: flags){
            _value |= static_cast<uint32_t>(f);
        }
    }
    [[nodiscard]] uint32_t value() const { return _value;}
private:
    uint32_t _value;
};


class PosixSocket
{
public:

    PosixSocket(int existing, const IpEndpoint& ipEndpoint) noexcept;
    PosixSocket(PosixSocket&& other) noexcept;
    PosixSocket(const PosixSocket&) = delete;
    virtual ~PosixSocket() { close(); }

    PosixSocket& operator=(const PosixSocket&) = delete;


    SocketError bind(const IpEndpoint& ipEndpoint) noexcept;

    void close() noexcept;

    SocketError setReceiveTimeOut(std::chrono::milliseconds milliseconds) noexcept;
    SocketError setTransmitTimeOut(std::chrono::milliseconds milliseconds) noexcept;
    SocketError setReuse(bool reuse) noexcept;
    SocketError setBlocking(bool block) noexcept;
    SocketError setReceiveMinSize(size_t size) noexcept;
    SocketError setSndBufSize(size_t size) noexcept;
    SocketError setRcvBufSize(size_t size) noexcept;
    SocketError setNoCheck(int checkSum) noexcept;

    [[nodiscard]] bool isValid() const { return (_socket >= 0); }
    [[nodiscard]] bool isReadReady(int millisec) const noexcept;
    [[nodiscard]] bool isWriteReady(int millisec) const noexcept;

    std::tuple<SocketError, size_t> readData(char* buf, size_t size, FlagUnion msgFlags = FlagUnion{MsgFlags::WaitAll, MsgFlags::NoSignal}) noexcept;
    std::tuple<SocketError, size_t> writeData(char* buf, size_t size, FlagUnion msgFlags = FlagUnion{MsgFlags::NoSignal}) noexcept;

    [[nodiscard]] const IpEndpoint& ipEndpoint() const { return _ipEndpoint; }
    [[nodiscard]] int nativeSocketDescriptor() const  { return _socket; }



private:
    IpEndpoint _ipEndpoint;
    int _socket;
};
}
}
#endif
