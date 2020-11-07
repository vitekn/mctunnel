#ifndef UDPSENDER_H
#define UDPSENDER_H

#include "libzet/exthreads.h"
#include "libzet/zudpsocket.h"
#include "libzet/logger.h"
#include "libzet/lbconfig.h"

#include "depacker.h"

class UDPSender: public Thread
{
public:
	UDPSender(Depacker*);

protected:
	virtual void run();

private:
	ZUdpSocket socket_;
	Depacker* depacker_;
	ZLogger* log_;
	Configuration::LibConf* conf_;
};

#endif
