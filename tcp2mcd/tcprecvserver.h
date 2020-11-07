/** Moдyль cepвepa npueмa TCP-nakemoв Quality **/

#ifndef TCPRECVSERVER_H
#define TCPRECVSERVER_H

#include "libzet/xtcpserver.h"
#include "libzet/lbconfig.h"
#include "libzet/logger.h"
#include "tcprecv.h"

using Configuration::LibConf;

class Depacker;

/// Kлacc cepвepa npueмa TCP-nakemoв muna Quality
class TCPRecvServer: public XTCPServer<TCPRecv>
{
public:
	TCPRecvServer(Depacker*);
	virtual ~TCPRecvServer() {}
	void actionsWithThread(TCPRecv* t)		///< уcmанoвka ynakoвщuka nakemoв кaждoмy nakemy
	{ t -> setDepacker(depacker_); }

private:
	ZLogger* log_;							///< лorrep neчamu debug-uнфopмaцuu
	LibConf* conf_;							///< koнфигурационный файл
	Depacker* depacker_;					///< ynakoвщuk пakemoв
};

#endif // TCPRECVSERVER_H
