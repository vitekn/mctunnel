#ifndef UDPTCPSTREAMER_H
#define UDPTCPSTREAMER_H

#include <sys/types.h>

#include "libzet/circbuffer.h"
#include "libzet/logger.h"
#include "libzet/lbconfig.h"
#include "udppacket.h"
#include "circbuffer.h"
#include <mutex>
namespace McTunnel{

class UDPTCPStreamer
{
    using Buffer = CircBuffer<100>;
public:
    UDPTCPStreamer(): log_(ZLogger::Instance()), conf_(Configuration::LibConf::Instance()), cbuf_(CircBuffer(conf_ -> bufSize())) {}
    ~UDPTCPStreamer() {}
    bool writePacket(const UdpPacket&);
    int readData(unsigned char*);
    void resetBuf();

private:
    void fromNet(const UdpPacket&, UdpPacket&);

private:
    ZLogger* log_;
    Configuration::LibConf* conf_;
    Buffer _cbuf;
};
}
#endif
