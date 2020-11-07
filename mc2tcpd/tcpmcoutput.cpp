#include "tcpmcoutput.h"
#include "udppacket.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

using std::pair;
using namespace Configuration;

static unsigned char snd_buf[MAX_UDP_SIZE + sizeof(UdpPacket)] = {};

TcpMCOutput::TcpMCOutput(UDPTCPStreamer* st): Thread(), streamer_(st), log_(ZLogger::Instance()), conf_(LibConf::Instance())
{ log_ -> log(ZLOG_DEBUG, 5, "TcpMCOutput::TcpMCOutput() ok\n"); }

/// Coздaнue TCP cokema
void TcpMCOutput::createTcpSocket(u_int32_t ip)
{
	bool is_create = false;
	if(finded_iterator_ == end_)
	{
        sockets_map_.insert(pair<u_int32_t, ZTcpSocket*>(ip, new ZTcpSocket()));
        is_create = true;
        finded_iterator_ = sockets_map_.find(ip);
	}
    else if(!finded_iterator_ -> second -> isValid())
    {
        finded_iterator_ -> second -> createSocket(ZSocket::TCP_SOCKET);
		is_create = true;
    }

	if(!is_create) return;

    finded_iterator_ -> second -> setTransmitTimeOut(2000);
    finded_iterator_ -> second -> setRecieveTimeOut(2000);
	finded_iterator_ -> second -> setSndBufSize(6 * 1024 * 1024);
	finded_iterator_ -> second -> setRcvBufSize(6 * 1024 * 1024);
	//log_ -> log(ZLOG_NOTICE, 5, "    creating the client socket (%i) %x; [%s]\n", finded_iterator_ -> second -> socketDescriptor(), ip, strerror(errno));
}

/// Пoдcoeдuнeнue k TCP-uнmepфeйcy
bool TcpMCOutput::connect2Interface(u_int32_t ip)
{
	if(finded_iterator_ -> second -> is_connected())
		return true;

    //usleep(50 * 1000);
	u_int32_t tcp_ip = conf_ -> getTCPIP();
	u_int16_t tcp_port = conf_ -> getTCPPort();
	if(finded_iterator_ -> second -> connect(tcp_ip, tcp_port))
	{
		log_ -> log(ZLOG_NOTICE, 5, "    connecting the client (%i) %x with server is successful on the port %i and tcp ip %x [%s]\n",
					finded_iterator_ -> second -> socketDescriptor(), ip, tcp_port, tcp_ip, strerror(errno));
		return true;
	}
	else
	{
		log_ -> log(ZLOG_WARNING, 3, "    connecting the client (%d) %x with server is unsuccessful on the port %i and tcp ip %x [%s]\n",
									finded_iterator_ -> second -> socketDescriptor(), ip, tcp_port, tcp_ip, strerror(errno));
        finded_iterator_ -> second ->close();
        log_ -> log(ZLOG_NOTICE, 5, "     the socket with ip %x closed, need for reconnecting\n", ip);
        sleep(2);
		return false;
	}
}

/// Зakpыmue u yнuчmoжeнue TCP-cokemoв
void TcpMCOutput::deleteTcpSockets()
{
	for(SockMap::iterator curr=sockets_map_.begin(); curr!=end_; ++curr)
	{
		log_ -> log(ZLOG_DEBUG, 5, "    the socket with ip %x closed\n", curr -> first);
		curr -> second -> close();
		delete curr -> second;
		curr -> second = NULL;
	}
	sockets_map_.clear();
}

/// Omkpыmue cokemoв u omnpaвka no TCP
void TcpMCOutput::run()
{
	int len = -3, t = 0;
	end_ = sockets_map_.end();
	finded_iterator_ = end_;
	log_ -> log(ZLOG_DEBUG, 5, "TcpMCOutput::run()\n");
	u_int64_t curr_time = 0;
	const int HALF_HOUR = 30 * 60 * 1000;

	while(!is_finish)
	{
		len = streamer_ -> readData(snd_buf);
		if(len <= 0)
		{
			log_ -> log(ZLOG_ERR, 1, "TcpMCOutput::run():    len = %i [%s]\n", len, strerror(errno));
			continue;
		}

		UdpPacket* u((UdpPacket*)snd_buf);
		u_int32_t ip = ntohl(u -> ip);
		finded_iterator_ = sockets_map_.find(ip);
		createTcpSocket(ip);
		if(!connect2Interface(ip))
		{
			log_ -> log(ZLOG_ERR, 1, "TcpMCOutput::run():    run(): ip = %x, the socket (%i) isn't connected [%s]\n", ip, finded_iterator_ -> second -> socketDescriptor(),
						strerror(errno));
			continue;
		}

		if(finded_iterator_ -> second -> isWriteReady(0, 100))
		{
			int written_bytes = finded_iterator_ -> second -> writeData((char*)snd_buf, len);
			if(written_bytes > 0)
				log_ -> log(ZLOG_DEBUG, 5, "TcpMCOutput::run():    wr data %i; | ip = %x, socket (%i), [%s]\n", len, ip, finded_iterator_ -> second -> socketDescriptor(),
							strerror(errno));
			else if(errno != EAGAIN && errno != EWOULDBLOCK)
			{
				log_ -> log(ZLOG_ERR, 1, "TcpMCOutput::run():    len = %i; | ip = %x, socket (%i), [%s]\n", len, ip, finded_iterator_ -> second -> socketDescriptor(),
							strerror(errno));
				if(finded_iterator_ -> second -> socketDescriptor() > 0 && finded_iterator_ -> second -> socketDescriptor() < 1001)
					finded_iterator_ -> second -> close();
				log_ -> log(ZLOG_NOTICE, 5, "TcpMCOutput::run():    the socket with ip %x closed, need for reconnecting\n", ip);
			}
		}
		else if(t < 5)
		{
            log_ -> log(ZLOG_ERR, 1, "TcpMCOutput::run():    ip = %x, the write to the socket (%i) is not ready %i [%s]\n", ip, finded_iterator_ -> second -> socketDescriptor(), t, strerror(errno));
			++t;
			curr_time = getTime();
		}

		if((getTime() - curr_time) > HALF_HOUR)
			t = 0;
	}

	deleteTcpSockets();

	log_ -> log(ZLOG_DEBUG, 5, "    TcpMCOutput::run finished\n");
}
