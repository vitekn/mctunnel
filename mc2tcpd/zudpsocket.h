#ifndef ZUDPSOCKET_H
#define ZUDPSOCKET_H
#include <cstddef>
#include "zsocket.h"
#include <tuple>
namespace Networking {
namespace PosixImplementation {

class PosixUdpSocket: public PosixSocket
{
public:
    PosixUdpSocket() noexcept;
    PosixUdpSocket(PosixUdpSocket&& other) noexcept;
    ~PosixUdpSocket() override = default;

    std::tuple<SocketError, size_t, IpAddress> readDataFrom(char* buf, size_t size, FlagUnion flags = FlagUnion{MsgFlags::WaitAll}) noexcept;
    std::tuple<SocketError, size_t> writeDataTo(const char* buf, size_t size, const IpAddress& ip, FlagUnion flags = FlagUnion()) noexcept;
    SocketError addMembership(const IpAddress& group) noexcept;
    SocketError setMulticastTtl(uint32_t ttl) noexcept;

};
}
}

#endif
