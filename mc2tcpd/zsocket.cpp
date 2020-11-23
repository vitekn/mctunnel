#include "zsocket.h"

#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <sys/select.h>

#include <cstring>
namespace Networking
{
namespace PosixImplementation
{

PosixSocket::PosixSocket(int existing, IpEndpoint&& ipEndpoint)
: _socket(existing)
, _ipEndpoint(ipEndpoint)
{}

PosixSocket::PosixSocket(SocketType type)
: _socket(createSocket(type))
{}

PosixSocket::PosixSocket(PosixSocket&& other) noexcept
: _socket(other._socket)
, _ipEndpoint(std::move(other._ipEndpoint));
{}

int PosixSocket::createSocket(SocketType type)
{
    switch(type)
    {
        case SocketType::TCP_SOCKET:
            _socket = ::socket(AF_INET, SOCK_STREAM, 0);
        break;
        case SocketType::UDP_SOCKET:
            _socket = ::socket(AF_INET, SOCK_DGRAM, 0);
        break;
    }
}

bool bindIpV4(IpAddress::IpV4 ip, uint16_t port)
{
    sockaddr_in server;
    size_t length = sizeof(server);
    memset(&server, 0, length);

    server.sin_family = AF_INET;
    if (ip == IpAddress::IpV4::ANY) {
        server.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        server.sin_addr.s_addr = htonl(ip);
    }
    server.sin_port = htons(port);
    return ::bind(_socket, (sockaddr * ) & server, length) >= 0;
}

bool bindIpV6(const IpAddress::IpV6& ip, uint16_t port)
{
    sockaddr_in6 server;
    size_t length = sizeof(server);
    memset(&server, 0, length);

    server.sin6_family = AF_INET;
    if (ip == IpAddress::IpV6::ANY) {
        server.sin6_addr.s6_addr = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    }
    else {
        const IpAddress::Ipv6 &ip6 = ipAddress.ipv6();
        server.sin6_addr.s6_addr32[0] = htonl(ip6.value4);
        server.sin6_addr.s6_addr32[1] = htonl(ip6.value3);
        server.sin6_addr.s6_addr32[2] = htonl(ip6.value2);
        server.sin6_addr.s6_addr32[3] = htonl(ip6.value1);
    }
    server.sin6_port = htons(port);

    return ::bind(_socket, (sockaddr * ) & server, length) >= 0;
}

SocketError PosixSocket::bind(const IpEndpoint& ipEndpoint)
{
    const IpAddress::Family family = ipEndpoint.ipAddress().family();
    if(_socket < 0 || family == IpAddress::Family::UNKNOWN || !ipEndpoint.port().has_value()) {
        return false;
    }

    const IpAddress& address = ipEndpoint.ipAddress();
    bool res = false;
    if (family == IpAddress::Family::IPV4) {
        res = bindIpV4(address.ipv4(), ipEndpoint.port().value());
    } else
    if (family == IpAddress::Family::IPV6) {
        res = bindIpV6(address.ipv6(), ipEndpoint.port().value());
    }

    if (res) {
        _ipEndpoint = ipEndpoint;
    }
    return res ? SocketError::SUCCESS : SocketError::ERROR;
}

void PosixSocket::close()
{
    if(_socket < 0) {
        return;
    }

    ::close(_socket);
    _socket = -1;
}

SocketError PosixSocket::setRecieveTimeOut(int millisecs)
{
    timeval to;
    to.tv_sec = millisecs / 1000;
    to.tv_usec = (millisecs % 1000) * 1000;

    return ::setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(timeval)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

SocketError PosixSocket::setTransmitTimeOut(int millisecs)
{
    timeval to;
    to.tv_sec = millisecs / 1000;
    to.tv_usec =(millisecs % 1000) * 1000;
    return ::setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(timeval)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setReuse(bool reuse)
{
    const int on = ((reuse) ? 1 : 0);
    return ::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setBlocking(bool block)
{
    int x = fcntl(_socket, F_GETFL, 0);
    if(block) {
        x &= (~O_NONBLOCK);
    } else {
        x |= O_NONBLOCK;
    }

    return ::fcntl(_socket, F_SETFL, x) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setRecieveMinSize(size_t size)
{
    return ::setsockopt(_socket, SOL_SOCKET, SO_RCVLOWAT, (char*)&size, sizeof(size)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

bool PosixSocket::isReadReady(int millisec) const
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_socket, &rfds);

    timeval t;
    t.tv_sec = millisec / 1000;
    t.tv_usec = (millisec % 1000) * 1000;

    if(::select(_socket + 1, &rfds, NULL, NULL, &t) > 0)
    {
        return FD_ISSET(_socket, &rfds) != 0;
    }

    return false;
}

bool PosixSocket::isWriteReady(int millisec) const
{
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(_socket, &wfds);

    timeval t;
    t.tv_sec = millisec / 1000;
    t.tv_usec = (millisec % 1000) * 1000;

    if(::select(_socket + 1, NULL, &wfds, NULL, &t) > 0)
    {
        return FD_ISSET(_socket, &wfds) != 0;
    }
    return false;
}

std::tuple<SocketError, size_t> PosixSocket::readData(char* buf, size_t ize, uint32_t flags)
{
    int res = ::recv(_socket, buf, size, flags);
    if (res >= 0) {
        return std::make_tuple(SocketError::SUCCESS, res);
    } else {
        if (res == EAGAIN) {
            return std::make_tuple(SocketError::TRY_AGAIN, 0);
        }
        return std::make_tuple(SocketError::ERROR, 0);
    }

}

std::tuple<SocketError, size_t> PosixSocket::writeData(char* buf, size_t size, uint32_t flags)
{
    int res = ::send(_socket, buf, size, flags);
    if (res>=0) {
        return std::make_tuple(SocketError::SUCCESS, res);
    } else {
        return std::make_tuple(SocketError::ERROR, 0);
    }
}

SocketError PosixSocket::setSndBufSize(size_t size)
{
    return setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setRcvBufSize(size_t size)
{
    return setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

SocketError PosixSocket::setNoCheck(int checkSum)
{
    return setsockopt(_socket, SOL_SOCKET, SO_NO_CHECK, (char*)&checkSum, sizeof(checkSum)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

}
}
}
