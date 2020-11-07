#include "tcprecvserver.h"

TCPRecvServer::TCPRecvServer(Depacker* dp): XTCPServer<TCPRecv>(1), log_(ZLogger::Instance()), conf_(LibConf::Instance()), depacker_(dp)
{
	max_count_sockets = conf_ -> lookupLong(string("sockets_count"), 1l);

	int port = conf_ -> getTCPSrcPort();
	bind(0, port);
}
