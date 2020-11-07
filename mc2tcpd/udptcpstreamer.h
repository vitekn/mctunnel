#ifndef UDPTCPSTREAMER_H
#define UDPTCPSTREAMER_H

#include <sys/types.h>

#include "libzet/circbuffer.h"
#include "libzet/logger.h"
#include "libzet/lbconfig.h"
#include "udppacket.h"

class UDPTCPStreamer
{
public:
	UDPTCPStreamer(): log_(ZLogger::Instance()), conf_(Configuration::LibConf::Instance()), cbuf_(CircBuffer(conf_ -> bufSize())) {}
	~UDPTCPStreamer() {}
	bool writePacket(UdpPacket*, int);
	int readData(unsigned char*);
	void resetBuf();

private:
	void fromNet(unsigned char*, UdpPacket&);

private:
	ZLogger* log_;
	Configuration::LibConf* conf_;
	CircBuffer cbuf_;
};

#endif
