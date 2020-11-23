#include "udptcpstreamer.h"
#include "circbuffer.h"

namespace McTunnel{

void UDPTCPStreamer::fromNet(const UdpPacket& buf, UdpPacket& packet)
{
    packet.ip = ntohl(buf->ip);
    packet.port = ntohs(buf->port);
    packet.size = ntohs(buf->size);
    packet.time_stamp = be64toh(buf->time_stamp);
}

bool UDPTCPStreamer::writePacket(const UdpPacket& packet)
{
    if (_cbuf.freeSize() < packet.getSize() + packet.metaDataSize()) {
//        log_ -> log(ZLOG_DEBUG, 5, "    size %i, %x, %i (available %i)\n", size, ntohl(packet -> ip), ntohs(packet ->size), cbuf_.bytesAvail());
        return false;
    }
    cbuf_ << packet;
    return true;
}

int UDPTCPStreamer::readData(UdpPacket& buf)
{
    if(cbuf_.empty()) {
        return 0;
    }
    cbuf_ >> buf;

    return buf.getSize();
}

void UDPTCPStreamer::resetBuf()
{
    cbuf_.reset();
}
}