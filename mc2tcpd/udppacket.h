#ifndef UDPPACKET_H
#define UDPPACKET_H

#include <sys/types.h>

const int MAX_UDP_SIZE = 64 * 1024;

struct UdpPacket
{
	u_int32_t ip;
	u_int16_t port;
	u_int16_t size;
	u_int64_t time_stamp;
	char data[];
};

#endif
