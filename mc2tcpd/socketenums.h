#ifndef MC2TCPD_SOCKETENUMS_H
#define MC2TCPD_SOCKETENUMS_H

namespace Networking {

enum class SocketError {
    SUCCESS = 0,
    ERROR = 1,
    TRY_AGAIN = 2,
};
enum class SocketType {
    TCP_SOCKET = 1,
    UDP_SOCKET = 2
};

}
#endif //MC2TCPD_SOCKETENUMS_H
