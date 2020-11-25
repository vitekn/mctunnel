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
    constexpr static size_t headerSize() {return 16U;}
    u_int32_t ip;
    u_int16_t port;
    u_int16_t size;
    u_int64_t time_stamp;
};

class UdpPacket
{
public:
    static const int MAX_DATA_SIZE = 64 * 1024;
    using BufferType = std::array<char, MAX_DATA_SIZE + UdpHeader::headerSize()>;

    UdpPacket() noexcept
    : size(0)
    , buf()
    {}

    void setHeader(const UdpHeader& header){
        size = ((size_t)header.size) + UdpHeader::headerSize();
        *(reinterpret_cast<u_int32_t*>(&(buf[0]))) = htonl(header.ip);
        *(reinterpret_cast<u_int16_t*>(&(buf[4]))) = htons(header.port);
        *(reinterpret_cast<u_int16_t*>(&(buf[6]))) = htons(header.size);
        *(reinterpret_cast<u_int64_t*>(&(buf[8]))) = htobe64(header.time_stamp);
    };

    [[nodiscard]] const BufferType& getData() const { return buf; }

    [[nodiscard]] char* getUdpPayloadPtr() { return buf.data()+UdpHeader::headerSize(); }
    [[nodiscard]] const char* getUdpPayloadPtr() const { return buf.data()+UdpHeader::headerSize(); }

    [[nodiscard]] size_t getSize() const {return size;}

private:
    size_t size;
    std::array<char, MAX_DATA_SIZE + UdpHeader::headerSize()> buf;
};

Networking::SocketError readFromSocket(Networking::Socket& socket, UdpPacket& packet);
}
#endif
