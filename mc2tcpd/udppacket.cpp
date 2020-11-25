#include "udppacket.h"

namespace McTunnel
{

Networking::SocketError readFromSocket(Networking::Socket &socket, UdpPacket &packet)
{
    Networking::SocketError err;
    size_t size;
    std::tie(err, size) = socket.readData(packet.getUdpPayloadPtr(), UdpPacket::MAX_DATA_SIZE);
    if (err != Networking::SocketError::SUCCESS) {
        return err;
    }
    auto tm = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch());
    packet.setHeader(UdpHeader(socket.ipEndpoint(), size, tm.count()));

    if (size > 1500) {
//        log_->log(ZLOG_WARNING, 2, "UDPRecv::run()    greater 1500 (MTU) res %i \n", res);
    }

    return Networking::SocketError::SUCCESS;

}

}