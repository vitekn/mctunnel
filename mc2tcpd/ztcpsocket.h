#ifndef ZTCPSOCKET_H
#define ZTCPSOCKET_H

#include "zsocket.h"
#include <memory>
#include <tuple>

namespace Networking {
namespace PosixImplementation {

class PosixTcpSocket: public PosixSocket
{
public:
    PosixTcpSocket();
    PosixTcpSocket(PosixTcpSocket&& other);
    PosixTcpSocket(int existing, IpEndpoint&& ipEndpoint);
    virtual ~PosixTcpSocket() {}

    SocketError listen();
    std::tuple<SocketError, std::unique_ptr<PosixTcpSocket> > accept();
    SocketError connect(const IpEndpoint& ipEndpoint);

    SocketError setNoDelay(bool value);

    bool isConnected() const { return _connectedEp.ipAddress().family != IpAddress::Family::UNKNOWN; }

    const IpEndpoint& connectedEndpoint() const {return _connectedEp;}

private:
    IpEndpoint _connectedEp;
};

}
}
#endif
