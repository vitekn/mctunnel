#ifndef MC2TCPD_SOCKET_H
#define MC2TCPD_SOCKET_H

#include <zsocket.h>

namespace Networking{
    using MsgFlags = PosixImplementation::MsgFlags;
    using Socket = PosixImplementation::PosixSocket;
}

#endif //MC2TCPD_SOCKET_H
