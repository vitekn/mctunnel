#include "zsocket.h"

#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <sys/select.h>

#include <cstring>
namespace Networking::PosixImplementation
{

PosixSocket::PosixSocket(int existing, const IpEndpoint& ipEndpoint) noexcept
: _socket(existing)
, _ipEndpoint(ipEndpoint)
{}

PosixSocket::PosixSocket(PosixSocket&& other) noexcept
: _socket(std::exchange(other._socket, -1))
, _ipEndpoint(std::exchange(other._ipEndpoint, IpEndpoint()))
{}

bool bindIpV4(IpAddress::IpV4 ip, uint16_t port, int socket)
{
    sockaddr_in server;
    const size_t length = sizeof(server);
    memset(&server, 0, length);

    server.sin_family = AF_INET;
    if (ip == IpAddress::ANYv4) {
        server.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        server.sin_addr.s_addr = htonl(ip.value);
    }
    server.sin_port = htons(port);

    return bind(socket, (sockaddr*) &server, length) >= 0;
}

bool bindIpV6(const IpAddress::IpV6& ip, uint16_t port, int socket)
{
    sockaddr_in6 server;
    size_t length = sizeof(server);
    memset(&server, 0, length);

    server.sin6_family = AF_INET;
    if (ip == IpAddress::ANYv6) {
        memset(server.sin6_addr.s6_addr,0,sizeof(server.sin6_addr.s6_addr));
    }
    else {
        server.sin6_addr.s6_addr32[0] = htonl(ip.value4);
        server.sin6_addr.s6_addr32[1] = htonl(ip.value3);
        server.sin6_addr.s6_addr32[2] = htonl(ip.value2);
        server.sin6_addr.s6_addr32[3] = htonl(ip.value1);
    }
    server.sin6_port = htons(port);

    return ::bind(socket, (sockaddr*) &server, length) >= 0;
}

SocketError PosixSocket::bind(const IpEndpoint& ipEndpoint) noexcept
{
    const IpAddress::Family family = ipEndpoint.ipAddress().family();
    if(_socket < 0 || family == IpAddress::Family::UNKNOWN || !ipEndpoint.port().has_value()) {
        return SocketError::ERROR;
    }

    const IpAddress& address = ipEndpoint.ipAddress();
    bool res = false;
    if (family == IpAddress::Family::IPV4) {
        res = bindIpV4(address.ipv4(), ipEndpoint.port().value(), _socket);
    } else
    if (family == IpAddress::Family::IPV6) {
        res = bindIpV6(address.ipv6(), ipEndpoint.port().value(), _socket);
    }

    if(res) {
        _ipEndpoint = ipEndpoint;
    }
    return res ? SocketError::SUCCESS : SocketError::ERROR;
}

void PosixSocket::close() noexcept
{
    if(_socket < 0) {
        return;
    }

    ::close(_socket);
    _socket = -1;
}

SocketError PosixSocket::setReceiveTimeOut(std::chrono::milliseconds milliseconds) noexcept
{
    timeval to;
    std::chrono::seconds secs = std::chrono::floor<std::chrono::seconds>(milliseconds);
    to.tv_sec = secs.count();
    to.tv_usec = (milliseconds.count() - secs.count()*1000) * 1000;

    return ::setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(timeval)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

SocketError PosixSocket::setTransmitTimeOut(std::chrono::milliseconds milliseconds) noexcept
{
    timeval to;
    std::chrono::seconds secs = std::chrono::floor<std::chrono::seconds>(milliseconds);
    to.tv_sec = secs.count();
    to.tv_usec = (milliseconds.count() - secs.count()*1000) * 1000;
    return ::setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(timeval)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setReuse(bool reuse) noexcept
{
    const int on = ((reuse) ? 1 : 0);
    return ::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setBlocking(bool block) noexcept
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

SocketError PosixSocket::setReceiveMinSize(size_t size) noexcept
{
    return ::setsockopt(_socket, SOL_SOCKET, SO_RCVLOWAT, (char*)&size, sizeof(size)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

bool PosixSocket::isReadReady(int millisec) const noexcept
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

bool PosixSocket::isWriteReady(int millisec) const noexcept
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

std::tuple<SocketError, size_t> PosixSocket::readData(char* buf, size_t size, FlagUnion flags) noexcept
{
    int res = ::recv(_socket, buf, size, flags.value());
    if (res >= 0) {
        return std::make_tuple(SocketError::SUCCESS, res);
    } else {
        if (res == EAGAIN) {
            return std::make_tuple(SocketError::TRY_AGAIN, 0);
        }
        return std::make_tuple(SocketError::ERROR, 0);
    }

}

std::tuple<SocketError, size_t> PosixSocket::writeData(char* buf, size_t size, FlagUnion flags) noexcept
{
    int res = ::send(_socket, buf, size, flags.value());
    if (res>=0) {
        return std::make_tuple(SocketError::SUCCESS, res);
    } else {
        return std::make_tuple(SocketError::ERROR, 0);
    }
}

SocketError PosixSocket::setSndBufSize(size_t size) noexcept
{
    return setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;

}

SocketError PosixSocket::setRcvBufSize(size_t size) noexcept
{
    return setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

SocketError PosixSocket::setNoCheck(int checkSum) noexcept
{
    return setsockopt(_socket, SOL_SOCKET, SO_NO_CHECK, (char*)&checkSum, sizeof(checkSum)) >= 0
           ? SocketError::SUCCESS
           : SocketError::ERROR;
}

}

