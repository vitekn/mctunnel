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
    PosixUdpSocket();
    PosixUdpSocket(PosixUdpSocket&& other);
    virtual ~PosixUdpSocket(){}

    std::tuple<SocketError, size_t, IpAddress> readDataFrom(char* buf, size_t size, uint32_t flags = MsgFlags::MSG_WAITALL);
    std::tuple<SocketError, size_t> writeDataTo(char* buf, size_t size, const IpAddress& ip, uint32_t flags = 0);
    SocketError addMembership(const IpAddress& group);
    SocketError setMulticastTtl(uint32_t ttl);

};
}
}

#endif
