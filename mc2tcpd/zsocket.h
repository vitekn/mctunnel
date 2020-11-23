#ifndef ZSOCKET_H
#define ZSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <socketenums.h>
#include <ipaddress.h>
#include <tuple>

namespace Networking
{
namespace PosixImplementation
{

enum class MsgFlags: uint32_t {
    WaitAll = MSG_WAITALL,
    NoSignal = MSG_NOSIGNAL,
};

class PosixSocket
{
public:

    explicit PosixSocket(int existing, IpEndpoint&& ipEndpoint) noexcept;
    explicit PosixSocket(SocketType type) noexcept;
    PosixSocket(PosixSocket&& other) noexcept;

    virtual ~PosixSocket() { close(); }

    SocketError bind(const IpEndpoint& ipEndpoint) noexcept;

    void close() noexcept;

    SocketError setReceiveTimeOut(int millisec) noexcept;
    SocketError setTransmitTimeOut(int millisec) noexcept;
    SocketError setReuse(bool reuse) noexcept;
    SocketError setBlocking(bool block) noexcept;
    SocketError setReceiveMinSize(size_t size) noexcept;
    SocketError setSndBufSize(size_t size) noexcept;
    SocketError setRcvBufSize(size_t size) noexcept;
    SocketError setNoCheck(int checkSum) noexcept;

    bool isValid() const { return (_socket >= 0); }
    bool isReadReady(int millisec) const noexcept;
    bool isWriteReady(int millisec) const noexcept;

    std::tuple<SocketError, size_t> readData(char* buf, size_t size, uint32_t msgFlags = MsgFlags::WaitAll | MsgFlags::NoSignal) noexcept;
    std::tuple<SocketError, size_t> writeData(char* buf, size_t size, uint32_t msgFlags = MSG_NOSIGNAL) noexcept;

    const IpEndpoint& ipEndpoint() const {return _ipEndpoint;}
    int nativeSocketDescriptor() const { return _socket; }

private:
    PosixSocket(const PosixSocket&) = delete;
    PosixSocket& operator=(const PosixSocket&) = delete;

    static int createSocket(SocketType type) noexcept;

private:
    IpEndpoint _ipEndpoint;
    int _socket;
};
}
}
#endif
