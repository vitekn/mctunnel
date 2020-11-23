#ifndef UDPPACKET_H
#define UDPPACKET_H

#include <sys/types.h>
#include <array>

#include <onscopeexit.h>
#include <circbuffer.h>
#include <socket.h>
//class CircBuffer;
namespace McTunnel
{
struct UdpHeader{

    UdpHeader(const Networking::IpEndpoint& ep, size_t sz, uint64_t time)
    : ip(ep.ipAddress().ipv4().value)
    , port(ep.port().value())
    , size(sz)
    , time_stamp(time)
    {}

    // TODO: IPV6
    constexpr size_t headerSize() const {return 16U;}
    u_int32_t ip;
    u_int16_t port;
    u_int16_t size;
    u_int64_t time_stamp;
};

class UdpPacket
{
public:
    static const int MAX_DATA_SIZE = 64 * 1024;
    using BufferType = std::array<uint8_t, MAX_DATA_SIZE + UdpHeader::headerSize()>;

    UdpPacket():size(0){}

    void setHeader(const UdpHeader& header){
        size = ((size_t)header.size) + UdpHeader::headerSize();
        *(reinterpret_cast<u_int32_t*>(&(buf[0]))) = htonl(socket->getBindedIp());
        *(reinterpret_cast<u_int16_t*>(&(buf[4]))) = htons(socket->getBindedPort());
        *(reinterpret_cast<u_int16_t*>(&(buf[6]))) = htons(res);
        *(reinterpret_cast<u_int64_t*>(&(buf[8]))) = htobe64((getTime() - timer_));
    };

    const BufferType& getData() const { return buf; }

    u_int8_t *getUdpPayloadPtr() { return buf.data()+UdpHeader::headerSize(); }
    const u_int8_t *getUdpPayloadPtr() const { return buf.data()+UdpHeader::headerSize(); }

    size_t getSize() const {return size;}

private:
    size_t size;
    std::array<uint8_t, MAX_DATA_SIZE + UdpHeader::headerSize()> buf;
};

Networking::SocketError readFromSocket(Networking::Socket& socket, UdpPacket& packet)
{
    Networking::SocketError err;
    size_t size;
    std::tie(err, size) = socket.readData(packet.getUdpPayloadPtr(), UdpPacket::MAX_DATA_SIZE);
    if(err != Networking::SocketError::SUCCESS) {
        return err;
    }

    packet.setHeader(UdpHeader(socket.ipEndpoint(), size, getTime()));

    if(size > 1500) {
//        log_->log(ZLOG_WARNING, 2, "UDPRecv::run()    greater 1500 (MTU) res %i \n", res);
    }

    return Networking::SocketError::SUCCESS;

}

}
#endif
