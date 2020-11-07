#include "udprecv.h"
#include "libzet/global.h"

#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <endian.h>
#include <memory.h>
#include <cmath>

UDPRecv::UDPRecv(): Thread(), streamer_(NULL)
{
	timer_ = 0;
	log_ = ZLogger::Instance();
	log_ -> log(ZLOG_DEBUG, 5, "UDPRecv::UDPRecv()\n");
	pthread_mutex_init(&_s_lock, NULL);
}

UDPRecv::~UDPRecv()
{
	log_ -> log(ZLOG_DEBUG, 5, "UDPRecv::~UDPRecv()\n");
	list<ZUdpSocket*>::iterator it;
	for(it=sockets_.begin(); it!=sockets_.end(); it++)
	{
		(*it) -> close();
		delete (*it);
	}
	log_ -> log(ZLOG_DEBUG, 5, "    finished\n");
}

bool UDPRecv::addSocket(u_int32_t ip, u_int16_t port)
{
	//log_ -> log(ZLOG_DEBUG, 5, "UDPRecv::addSocket(ip = %s, port = %d)\n", ipaddrToStr(ip).c_str(), port);
	ZUdpSocket *ns(new ZUdpSocket());
	ns -> setReuse(true);
	ns -> setBlocking(false);
	ns -> setRecieveTimeOut(100);

	if(ns -> bind(ip, port))
	{ /*log_ -> log(ZLOG_DEBUG, 5, "    binded %x, [%s]\n", ip, strerror(errno));*/ }
	else
	{ /*log_ -> log(ZLOG_WARNING, 2, "    not binded [%s]\n", strerror(errno));*/ }

	if(ns -> addMembership(ip))
	{ /*log_ -> log(ZLOG_DEBUG, 5, "    joined [%s]\n", strerror(errno));*/ }
	else
	{ /*log_ -> log(ZLOG_WARNING, 2, "    not joined [%s]\n", strerror(errno));*/ }
	pthread_mutex_lock(&_s_lock);
	sockets_.push_back(ns);
	pthread_mutex_unlock(&_s_lock);

	//log_ -> log(ZLOG_DEBUG, 5, "    finished %p, [%s]\n", ns, strerror(errno));
	return true;
}

int UDPRecv::getSocketsCount()
{
	//log_ -> log(ZLOG_DEBUG, 5, "UDPRecv::getSocketsCount()\n");
	pthread_mutex_lock(&_s_lock);
	int c = sockets_.size();
	pthread_mutex_unlock(&_s_lock);
	//log_ -> log(ZLOG_DEBUG, 5, "    finished %i\n", c);
	return c;
}

void UDPRecv::setStreamer(UDPTCPStreamer* st)
{
	log_ -> log(ZLOG_DEBUG, 5, "UDPRecv::setStreamer()\n");
	streamer_ = st;
}

void UDPRecv::run()
{
	log_ -> log(ZLOG_DEBUG, 5, "UDPRecv::run():\n");

	char *buf = NULL;			//[64*1024];
	buf = new char[MAX_UDP_SIZE + sizeof(UdpPacket)];
	log_ -> log(ZLOG_DEBUG, 5, "    after creating buf = %p, [thread = %p]\n", buf, this);
	UdpPacket* u = (UdpPacket*)buf;
	buf   +=   sizeof(UdpPacket);
	list<ZUdpSocket*>::iterator it;

	while(!is_finish)
	{
		bool idle = true;
		for(it=sockets_.begin(); it!=sockets_.end(); it++)
		{
			int res = (*it) -> readData(buf, MAX_UDP_SIZE);//,(in_addr_t*)&ip);
			//log_ -> log(ZLOG_DEBUG, 2, "UDPRecv::run() read data res = %i\n", res);
			if(res > 0)
			{
				if(res > 1500)
					log_ -> log(ZLOG_WARNING, 2, "UDPRecv::run()    greater 1500 (MTU) res %i \n", res);
				if(u)		//data
				{
					u -> ip = htonl((*it) -> getBindedIp());
					u -> port = htons((*it) -> getBindedPort());
					u -> size = htons(res);
					u -> time_stamp = htobe64((getTime() - timer_));
				}
				if(streamer_)
				{
					if(!streamer_ -> writePacket(u, res))
					{
						log_ -> log(ZLOG_WARNING, 2, "UDPRecv::run()     buffer overflow dropping packet %x, size %i [%s]\n", ntohl(u -> ip), res, strerror(errno));
						sleep(1);
					}
					else
						log_ -> log(ZLOG_DEBUG, 2, "UDPRecv::run()    read ip %x data res %i\n", (*it) -> getBindedIp(), res);
				}
				idle = false;
			}
			else if(errno != EAGAIN)
			{
				log_ -> log(ZLOG_WARNING, 5, "UDPRecv::run()    errno != EAGAIN: [%s]\n", strerror(errno));
				idle = false;
			}
		}
		if(idle) usleep(1000);
	}

	if(buf)
	{ buf -= sizeof(UdpPacket); delete[] buf; u = NULL; buf = NULL; }
	log_ -> log(ZLOG_DEBUG, 5, "    UDPRecv::run finished\n");
}
