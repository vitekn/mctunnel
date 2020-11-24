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
    PosixTcpSocket() noexcept;
    PosixTcpSocket(PosixTcpSocket&& other) noexcept;
    PosixTcpSocket(int existing, const IpEndpoint& ipEndpoint) noexcept;
    ~PosixTcpSocket() override = default;

    SocketError listen() noexcept;
    std::tuple<SocketError, std::unique_ptr<PosixTcpSocket> > accept() noexcept;
    SocketError connect(const IpEndpoint& ipEndpoint) noexcept;

    SocketError setNoDelay(bool value);

    [[nodiscard]] bool isConnected() const { return _connectedEp.ipAddress().family() != IpAddress::Family::UNKNOWN; }

    [[nodiscard]] const IpEndpoint& connectedEndpoint() const {return _connectedEp;}

private:
    IpEndpoint _connectedEp;
};

}
}
#endif
