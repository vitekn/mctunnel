#include "ztcpsocket.h"

#include <strings.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <cstddef>
#include <cstring>

namespace Networking {
namespace PosixImplementation {

PosixTcpSocket::PosixTcpSocket() noexcept
: PosixSocket(::socket(AF_INET, SOCK_STREAM, 0), IpEndpoint())
, _connectedEp(IpAddress(), std::optional<uint16_t>())
{}

PosixTcpSocket::PosixTcpSocket(int existing, const IpEndpoint& ipEndpoint) noexcept
: PosixSocket(existing, ipEndpoint)
, _connectedEp(IpAddress(), std::optional<uint16_t>())
{}

PosixTcpSocket::PosixTcpSocket(PosixTcpSocket &&other) noexcept
: PosixSocket(std::move(other))
, _connectedEp(other._connectedEp)
{}

std::tuple<SocketError, std::unique_ptr<PosixTcpSocket> > PosixTcpSocket::accept() noexcept
{
    IpAddress::Family family = PosixSocket::ipEndpoint().ipAddress().family();
    if (family == IpAddress::Family::UNKNOWN) return std::make_tuple(SocketError::ERROR, nullptr);

    if (family == IpAddress::Family::IPV4) {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t addr_len = sizeof(addr);

        int new_sock = ::accept(PosixSocket::nativeSocketDescriptor(), reinterpret_cast<sockaddr*>(&addr), &addr_len);
        if (new_sock < 0) {
            return std::make_tuple(SocketError::ERROR, nullptr);
        } else {
            IpEndpoint ipEndpoint(IpAddress(IpAddress::IpV4{ntohl(addr.sin_addr.s_addr)}), ntohs(addr.sin_port));
            return std::make_tuple(SocketError::SUCCESS,
                                   std::make_unique<PosixTcpSocket>(new_sock, ipEndpoint));
        }
    }

    sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_len = sizeof(addr);

    int new_sock = ::accept(PosixSocket::nativeSocketDescriptor(), reinterpret_cast<sockaddr*>(&addr), &addr_len);
    if (new_sock < 0) {
        return std::make_tuple(SocketError::ERROR, nullptr);
    } else {
        IpEndpoint ipEndpoint(IpAddress(IpAddress::IpV6{ntohl(addr.sin6_addr.s6_addr32[0]),
                                        ntohl(addr.sin6_addr.s6_addr32[1]),
                                        ntohl(addr.sin6_addr.s6_addr32[2]),
                                        ntohl(addr.sin6_addr.s6_addr32[3])}),
                              ntohs(addr.sin6_port));
        return std::make_tuple(SocketError::SUCCESS,
                               std::make_unique<PosixTcpSocket>(new_sock, ipEndpoint));
    }

}

SocketError PosixTcpSocket::listen() noexcept
{
    return ::listen(PosixSocket::nativeSocketDescriptor(), SOMAXCONN) == 0 ? SocketError::SUCCESS : SocketError::ERROR;
}

SocketError PosixTcpSocket::connect(const IpEndpoint& ipEndpoint) noexcept
{
    if(_connectedEp.ipAddress().family() != IpAddress::Family::UNKNOWN) {
        return SocketError::ERROR;
    }

    IpAddress::Family family = PosixSocket::ipEndpoint().ipAddress().family();
    if (family == IpAddress::Family::UNKNOWN || family != ipEndpoint.ipAddress().family()) {
        return SocketError::ERROR;
    }

    if (family == IpAddress::Family::IPV4) {
        sockaddr_in server;
        const size_t length = sizeof(server);
        memset(&server, 0, length);

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(ipEndpoint.ipAddress().ipv4().value);
        server.sin_port = htons(ipEndpoint.port().value_or(0));

        if (::connect(PosixSocket::nativeSocketDescriptor(), reinterpret_cast<sockaddr*>(&server), length) >= 0) {
            _connectedEp = ipEndpoint;
            return SocketError::SUCCESS;
        }
    }

    sockaddr_in6 server;
    const size_t length = sizeof(server);
    memset(&server, 0, length);

    server.sin6_family = AF_INET6;
    server.sin6_addr.s6_addr32[0] = htonl(ipEndpoint.ipAddress().ipv6().value4);
    server.sin6_addr.s6_addr32[1] = htonl(ipEndpoint.ipAddress().ipv6().value3);
    server.sin6_addr.s6_addr32[2] = htonl(ipEndpoint.ipAddress().ipv6().value2);
    server.sin6_addr.s6_addr32[3] = htonl(ipEndpoint.ipAddress().ipv6().value1);
    server.sin6_port = htons(ipEndpoint.port().value_or(0));

    if (::connect(PosixSocket::nativeSocketDescriptor(), reinterpret_cast<sockaddr*>(&server), length) >= 0) {
        _connectedEp = ipEndpoint;
        return SocketError::SUCCESS;
    }
    return SocketError::ERROR;

}

SocketError PosixTcpSocket::setNoDelay(bool no_delay)
{
    const int on = ((no_delay) ? 1 : 0);

    return (setsockopt(PosixSocket::nativeSocketDescriptor(), IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) >= 0)
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

}
}