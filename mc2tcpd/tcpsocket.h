#ifndef MC2TCPD_TCPSOCKET_H
#define MC2TCPD_TCPSOCKET_H

#include <ztcpsocket.h>

namespace Networking{
    using TcpSocket = PosixImplementation::PosixTcpSocket;
}

#endif //MC2TCPD_TCPSOCKET_H
