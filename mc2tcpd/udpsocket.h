#ifndef MC2TCPD_UDPSOCKET_H
#define MC2TCPD_UDPSOCKET_H

#include <zudpsocket.h>

namespace Networking{
    using UdpSocket = PosixImplementation::PosixUdpSocket;
}

#endif //MC2TCPD_UDPSOCKET_H
