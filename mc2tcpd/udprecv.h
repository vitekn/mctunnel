#ifndef UDPRECV_H
#define UDPRECV_H

#include <list>

#include "libzet/exthreads.h"
#include "libzet/zudpsocket.h"
#include "libzet/logger.h"
#include "udptcpstreamer.h"

using namespace std;

class UDPRecv: public Thread
{
public:
	UDPRecv();
	virtual ~UDPRecv();
	int getSocketsCount();
	bool addSocket(u_int32_t, u_int16_t);
	void setStreamer(UDPTCPStreamer*);
	void setTimer(u_int64_t t)		///< ycmaнoвka значения cчemчuka
	{ timer_ = t; }

protected:
	virtual void run();

private:
	list<ZUdpSocket*> sockets_;
	UDPTCPStreamer* streamer_;
	pthread_mutex_t _s_lock;
	ZLogger* log_;
	u_int64_t timer_;
};

#endif
