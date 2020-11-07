#include "udptcpstreamer.h"

using namespace Configuration;

void UDPTCPStreamer::fromNet(unsigned char* buf, UdpPacket& packet)
{
	UdpPacket* u((UdpPacket*)buf);
	packet.ip = ntohl(u -> ip);
	packet.port = ntohs(u -> port);
	packet.size = ntohs(u -> size);
	packet.time_stamp = be64toh(u -> time_stamp);
}

bool UDPTCPStreamer::writePacket(UdpPacket* packet, int size)
{
	if(cbuf_.writeData((unsigned char*)packet, sizeof(UdpPacket) + size, CB_WRITE_ALL))
		return true;
	else
	{
		log_ -> log(ZLOG_DEBUG, 5, "    size %i, %x, %i (available %i)\n", size, ntohl(packet -> ip), ntohs(packet ->size), cbuf_.bytesAvail());
		return false;
	}
}

int UDPTCPStreamer::readData(unsigned char* buf)
{
	if(cbuf_.readData(buf, sizeof(UdpPacket), CB_WRITE_ALL) <= 0)
		return 0;

	UdpPacket packet;
	fromNet(buf, packet);
	int res = sizeof(UdpPacket);
	//log_ -> log(ZLOG_DEBUG, 5, "UDPTCPStreamer::readData(): res = %i; %x, %i\n", sizeof(UdpPacket), packet.ip, packet.size);
	if(packet.size > 0 && packet.size < MAX_UDP_SIZE)
	{
		res += cbuf_.readData(buf + sizeof(UdpPacket), packet.size, CB_WRITE_ALL);
		if(res == ((int)sizeof(UdpPacket) + packet.size))
			log_ -> log(ZLOG_DEBUG, 5, "    res  %i, %x, %i (available %i)\n", res, packet.ip, packet.size, cbuf_.bytesAvail());
	}

	return res;
}

void UDPTCPStreamer::resetBuf()
{ cbuf_.reset(); }
