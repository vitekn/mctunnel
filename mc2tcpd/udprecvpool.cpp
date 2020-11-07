#include "udprecvpool.h"
#include "libzet/global.h"

using namespace Configuration;

UDPRecvPool::UDPRecvPool(UDPTCPStreamer* st): ThreadPool<UDPRecv>(), _logger(ZLogger::Instance()), conf_(LibConf::Instance()),
									_max_sockets(conf_ -> socketsCount())
{
	int grp_cnt = conf_ -> groupCount(), threads_count = grp_cnt / _max_sockets;
	if(grp_cnt % _max_sockets)
		++threads_count;
	if(!init(threads_count))
	{
		_logger -> log(ZLOG_ERR, 1, "UDPRecvPool::UDPRecvPool(st = %p):    threads_count must not be 0 (threads_count = %d)\n", st, threads_count);
		return;
	}
	_logger -> log(ZLOG_DEBUG, 5, "UDPRecvPool::UDPRecvPool()\n");
	_logger -> log(ZLOG_NOTICE, 5, "    threads_count %d, group_count %i\n", threads_count, grp_cnt);

	UDPRecv *ur;
	u_int64_t t = getTime();
	for(int i=0; (ur = getThread(i))!=NULL; ++i)
	{
		ur -> setStreamer(st);
		ur -> setTimer(t);
	}

	addGroup();

	Int32Array ip_array(conf_ -> getArrayIPUDP());
	IntArray port_array(conf_ -> getArrayPortUDP());
	_logger -> log(ZLOG_DEBUG, 5, "    ip_array size %i, port_array size %i\n", ip_array.size(), port_array.size());
	int ip_size = ip_array.size(), port_size = port_array.size();
    _logger -> log(ZLOG_NOTICE, 5, "    ip_size %d, port_size %i\n", ip_size, port_size);
	for(int i=0; i<ip_size; ++i)
	{
		int n = (port_size < ip_size && i >= port_size) ? port_size - 1 : i;
        _logger -> log(ZLOG_NOTICE, 3, "    %i) %s:%i\n", i + 1, ipaddrToStr(ip_array.at(i)).c_str(), port_array.at(n));
	}
}

UDPRecvPool::~UDPRecvPool()
{}

bool UDPRecvPool::addGroup(in_addr_t ip, short port)
{
	UDPRecv *ur;
	bool ok = false;

	//_logger -> log(ZLOG_DEBUG, 5, "    max sockets = %i\n", _max_sockets);
	for(int i=0; !ok && (ur = getThread(i)) != NULL; ++i)
	{
		if(ur -> getSocketsCount() < _max_sockets)
		{
			ur -> addSocket(ip, port);
			ok = true;
		}
	}

	return ok;
}

/// Добавление IP UDP-nomokoв, cчumaнныe uз фaйлa koнфuгypaцuu
bool UDPRecvPool::addGroup()
{
	bool r = false;
	_logger -> log(ZLOG_DEBUG, 1, "UDPRecvPool::addGroup()\n");
	Int32Array ip_array(conf_ -> getArrayIPUDP());
	IntArray port_array(conf_ -> getArrayPortUDP());
	int ip_size = ip_array.size(), port_size = port_array.size();
	_logger -> log(ZLOG_DEBUG, 5, "    ip_size = %i, port_size = %i\n", ip_size, port_size);
	for(int i=0; i<ip_size; ++i)
	{
		in_addr_t ip = ip_array.at(i);
		int n = (port_size < ip_size && i >= port_size) ? port_size - 1 : i;
		short port = port_array.at(n);
		r = addGroup(ip, port);
	}

	return r;
}
