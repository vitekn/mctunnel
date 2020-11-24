#include "zudpsocket.h"
#include <string.h>

namespace Networking {

namespace PosixImplementation {

PosixUdpSocket::PosixUdpSocket()  noexcept
: PosixSocket(::socket(AF_INET, SOCK_DGRAM, 0), IpEndpoint())
{}

PosixUdpSocket::PosixUdpSocket(PosixUdpSocket &&other) noexcept
: PosixSocket(std::move(other))
{}

std::tuple<SocketError, size_t, IpAddress>
PosixUdpSocket::readDataFrom(char* buf, size_t size, FlagUnion flags) noexcept
{
    IpAddress::Family family = PosixSocket::ipEndpoint().ipAddress().family();
    IpAddress ip;
    int ret = -1;
    size_t readed = 0;

    if (family == IpAddress::Family::IPV4) {

        sockaddr_in from;
        socklen_t fromlen = sizeof(sockaddr_in);

        ret = ::recvfrom(PosixSocket::nativeSocketDescriptor(),
                             buf,
                             size,
                             flags.value(),
                             reinterpret_cast<sockaddr*>(&from), &fromlen);
        if (ret >= 0) {
            ip = IpAddress(IpAddress::IpV4{ntohl(from.sin_addr.s_addr)});
        }
    } else if (family == IpAddress::Family::IPV6) {
        sockaddr_in6 from;
        socklen_t fromlen = sizeof(sockaddr_in6);

        ret = ::recvfrom(PosixSocket::nativeSocketDescriptor(),
                             buf,
                             size,
                             flags.value(),
                             reinterpret_cast<sockaddr*>(&from), &fromlen);
        if (ret >= 0) {
            ip = IpAddress(IpAddress::IpV6{ntohl(from.sin6_addr.s6_addr32[0]),
                                           ntohl(from.sin6_addr.s6_addr32[1]),
                                           ntohl(from.sin6_addr.s6_addr32[2]),
                                           ntohl(from.sin6_addr.s6_addr32[3])});
        }
    }

    SocketError err;
    if (ret >=0 ){
        err = SocketError::SUCCESS;
        readed = ret;
    } else {
        if (ret == EAGAIN) {
            err = SocketError::TRY_AGAIN;
        } else {
            err = SocketError::ERROR;
        }
    }

    return std::make_tuple(err, readed, ip);
}

std::tuple<SocketError, size_t> PosixUdpSocket::writeDataTo(const char* buf, size_t size, const IpAddress& ip, FlagUnion flags) noexcept
{
    SocketError err = SocketError::ERROR;
    size_t written = 0;
    IpAddress::Family family = PosixSocket::ipEndpoint().ipAddress().family();
    if (family == IpAddress::Family::UNKNOWN || family != ip.family()) return std::make_tuple(err, written);

    if (family == IpAddress::Family::IPV4) {
        sockaddr_in from;
        socklen_t fromlen = sizeof(sockaddr_in);

        memset(&from, 0, sizeof(from));
        from.sin_addr.s_addr = htonl(ip.ipv4().value);
        from.sin_port = htons(PosixSocket::ipEndpoint().port().value());
        from.sin_family = AF_INET;

        int ret = ::sendto(PosixSocket::nativeSocketDescriptor(), buf, size, flags.value(), (sockaddr * ) & from, fromlen);
        if (ret>=0){
            written = ret;
            err = SocketError::SUCCESS;
        }

    } else
    if (family == IpAddress::Family::IPV6) {
        sockaddr_in6 from;
        socklen_t fromlen = sizeof(sockaddr_in6);

        memset(&from, 0, sizeof(from));
        from.sin6_addr.s6_addr32[0] = htonl(ip.ipv6().value4);
        from.sin6_addr.s6_addr32[1] = htonl(ip.ipv6().value3);
        from.sin6_addr.s6_addr32[2] = htonl(ip.ipv6().value2);
        from.sin6_addr.s6_addr32[3] = htonl(ip.ipv6().value1);
        from.sin6_port = htons(PosixSocket::ipEndpoint().port().value());
        from.sin6_family = AF_INET;

        int ret = ::sendto(PosixSocket::nativeSocketDescriptor(), buf, size, flags.value(), (sockaddr * ) & from, fromlen);
        if (ret>=0){
            written = ret;
            err = SocketError::SUCCESS;
        }
    }

    return std::make_tuple(err, written);
}

SocketError PosixUdpSocket::addMembership(const IpAddress& group) noexcept
{
    IpAddress::Family family = PosixSocket::ipEndpoint().ipAddress().family();
    if (family == IpAddress::Family::UNKNOWN || family != group.family()) return SocketError::ERROR;

    if (family == IpAddress::Family::IPV4) {
        ip_mreq gr;
        gr.imr_multiaddr.s_addr = htonl(group.ipv4().value);
        gr.imr_interface.s_addr = htonl(INADDR_ANY);
        return (setsockopt(PosixSocket::nativeSocketDescriptor(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &gr, sizeof(gr)) < 0) ?
               SocketError::ERROR :
               SocketError::SUCCESS;
    }
    ipv6_mreq gr6;
    gr6.ipv6mr_multiaddr.s6_addr32[0] = htonl(group.ipv6().value4);
    gr6.ipv6mr_multiaddr.s6_addr32[1] = htonl(group.ipv6().value3);
    gr6.ipv6mr_multiaddr.s6_addr32[2] = htonl(group.ipv6().value2);
    gr6.ipv6mr_multiaddr.s6_addr32[3] = htonl(group.ipv6().value1);

    gr6.ipv6mr_interface = 0;
    return (setsockopt(PosixSocket::nativeSocketDescriptor(), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &gr6, sizeof(gr6)) < 0) ?
           SocketError::ERROR :
           SocketError::SUCCESS;
}

SocketError PosixUdpSocket::setMulticastTtl(uint32_t ttl) noexcept
{

    IpAddress::Family family = PosixSocket::ipEndpoint().ipAddress().family();
    if (family == IpAddress::Family::UNKNOWN) return SocketError::ERROR;

    if (family == IpAddress::Family::IPV4) {
        return (setsockopt(PosixSocket::nativeSocketDescriptor(), IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) >= 0)
               ? SocketError::SUCCESS
               : SocketError::ERROR;
    }

    return (setsockopt(PosixSocket::nativeSocketDescriptor(), IPPROTO_IPV6, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) >= 0)
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

}
}
