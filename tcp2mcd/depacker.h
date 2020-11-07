#ifndef DEPACKER_H
#define DEPACKER_H

#include <map>

#include "udppacket.h"
#include "infodata.h"
#include "libzet/logger.h"
#include "libzet/lbconfig.h"

using std::iterator;

class Depacker
{
public:
	Depacker();
	virtual ~Depacker() {}

    bool writeData(char* data, int size);
	UdpPacket* getPacket();
    int decreaseBuffNow(int size){
        return sizebuff_now -= size;
    }

private:
	UdpPacket* fromNet(char *buf);

private:
	ZLogger* log_;
	Configuration::LibConf* conf_;
	u_int64_t timer_;
	IDataMap buf_map_;
	int max_size_buf_;
    int max_size_buf_one_ip;
    int sizebuff_now;
};

#endif
