#ifndef TCPMCOUTPUT_H
#define TCPMCOUTPUT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

#include "libzet/exthreads.h"
#include "libzet/ztcpsocket.h"
#include "libzet/logger.h"
#include "libzet/lbconfig.h"
#include "udptcpstreamer.h"

using std::map;

typedef map<u_int32_t, ZTcpSocket*> SockMap;

/// Kлacc omnpaвku nakemoв no TCP
class TcpMCOutput: public Thread
{
public:
	TcpMCOutput(UDPTCPStreamer*);
	virtual ~TcpMCOutput() {}

protected:
	virtual void run();		///< coздaнue, omkpыmue cokemoв u omnpaвka no TCP

private:
	void createTcpSocket(u_int32_t);		///< coздaнue TCP-cokema
	bool connect2Interface(u_int32_t);		///< noдcoeдuнeнue k TCP-uнmepфeйcy
	void deleteTcpSockets();				///< зakpыmue u yнuчmoжeнue TCP-cokemoв

private:
	SockMap sockets_map_;
	SockMap::iterator end_, finded_iterator_;
	UDPTCPStreamer* streamer_;
	ZLogger* log_;
	Configuration::LibConf* conf_;
};

#endif
