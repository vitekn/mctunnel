#include "tcprecv.h"
#include "udppacket.h"
#include "depacker.h"
#include "libzet/ztcpsocket.h"

#include <errno.h>

/**
				//_log -> log(ZLOG_DEBUG, 5, "    new packet udp: %x %i, up_size = %i, [%s]\n", ntohl(up -> ip), size, up_size, strerror(errno));
				if(client -> isWriteReady(0, 100 * 1000) == ZSocket::ZST_READY)
				{
					char* stump = (char*)"ABCD";
					int wr = client -> writeData(stump, strlen(stump));
					if(wr < 0)
						_log -> log(ZLOG_ERR, 1, "    written bytes = %i, strlen(stump) = %i, [%s]\n", wr, strlen(stump), strerror(errno));
				}
				else
					_log -> log(ZLOG_ERR, 1, "    socket isn't ready for write', [%s]\n", strerror(errno));
**/
static int t = 0;

/// Чmeнue дaнныx no TCP
void TCPRecv::readData(char* buf, ZTcpSocket* client_sock)
{
	if(!client_sock)
	{
		log_ -> log(ZLOG_WARNING, 1, "    the client_sock is not initialized [%s]\n", strerror(errno));
		usleep(1000);		///< ecлu в cnucke nycmo, mo ждeм 1 мc, u cнoвa npoвepяeм
		return;
	}
	if(!client_sock -> is_connected())
	{
		log_ -> log(ZLOG_WARNING, 1, "    the client_sock (%i) is not connected [%s]\n", client_sock -> socketDescriptor(), strerror(errno));
		usleep(1000);
		return;
	}

    int offset = sizeof(UdpPacket), size = client_sock -> readData(buf, offset);
	if(errno == ENOTCONN)
		log_ -> log(ZLOG_WARNING, 1, "    not connection on the socket (%i) [%s]\n", client_sock -> socketDescriptor(), strerror(errno));
	if(size <= 0 || errno == ENOTCONN)
	{
		/*log_ -> log(ZLOG_WARNING, 1, "    on the socket (%i) the size %i of the data isn't readed (size < 0) [%s]\n", client_sock -> socketDescriptor(), size,
					strerror(errno));*/
		return;
	}
	UdpPacket* u((UdpPacket*)buf);
	int u_size = ntohs(u -> size);
	size += client_sock -> readData(buf + sizeof(UdpPacket), u_size);
	if(size < (u_size + offset) || errno == ENOTCONN)
	{
		/*log_ -> log(ZLOG_WARNING, 1, "    on the socket (%i) the size %i (u_size %i, offset %i) of the data isn't readed [%s]\n",
					client_sock -> socketDescriptor(), size, u_size, offset, strerror(errno));*/
		return;
	}
    if( !depacker_ -> writeData(buf, size)) {
        log_ -> log(ZLOG_WARNING, 2, "Depacker::writeData     buffer overflow dropping packet %x, size %i [%s]\n", ntohl(((UdpPacket*)buf) -> ip), size, strerror(errno));
        //sleep(1);
        return;
    }
    buf = NULL;
	//if(!(t % 1000))
	log_ -> log(ZLOG_DEBUG, 5, "TCPRecv::run():    socket(%i) ip %x (port %i) and size %i [%s]\n", client_sock -> socketDescriptor(), ntohl(u -> ip),
					client_sock -> getBindedPort(), size, strerror(errno));
	++t;
}

/// Пpoвepka cokema нa koppekmнocmь nocлe фyнкцuu select()
bool TCPRecv::isExceptedSocket(ZTcpSocket* client_sock, list<ZTcpSocket*>& excepted_socks)
{
	for(list<ZTcpSocket*>::iterator jt=excepted_socks.begin(); jt!=excepted_socks.end(); ++jt)
		if((*jt) -> socketDescriptor() == client_sock -> socketDescriptor())
			return true;
	return false;
}

/// Oбpaбomka npoчumaнныx nakemoв дaнныx
void TCPRecv::run()
{
	log_ -> log(ZLOG_NOTICE, 5, "TCPRecv thread ID %i\n", syscall(SYS_gettid));

	char buf[MAX_UDP_SIZE] = {};
	int n = 0;
	t = 0;
	list<ZTcpSocket*> list_socks, excepted_socks;

	log_ -> log(ZLOG_DEBUG, 5, "TCPRecv::run()\n");
	while(!is_finish)
	{
        if(!depacker_)
		{
			log_ -> log(ZLOG_WARNING, 1, "    the depacker (%p) is not initialized\n", depacker_);
			return;
		}
		int res = readedSockets(list_socks, excepted_socks, 0, 10 * 1000);
		if(res <= 0)
		{
			//log_ -> log(ZLOG_WARNING, 1, "    the result of the function poll %i or list of sockets is empty %i [%s]\n", res, list_socks.size(), strerror(errno));
			continue;
		}
		for(list<ZTcpSocket*>::iterator it=list_socks.begin(); it!=list_socks.end(); ++it)
            readData(buf, *it);
        usleep(500);
		for(list<ZTcpSocket*>::iterator it=excepted_socks.begin(); it!=excepted_socks.end(); ++it)
		{
			for(int n=0; n<size(); ++n)
			{
				ZTcpSocket* client_sock(getSocket(n));
				if(!client_sock) continue;

				if(!client_sock -> is_connected() && client_sock -> socketDescriptor() == (*it) -> socketDescriptor())
				{
					log_ -> log(ZLOG_NOTICE, 5, "    erasing the socket %i [%s]\n", n, strerror(errno));
                    delete client_sock;
                    log_ -> log(ZLOG_DEBUG, 5, "ZTcpSocket::accept() delete socket on %x\n", client_sock);
                    client_sock = NULL;
					erase(n);
				}
			}
		}
	}

	for(n=0; n<size(); ++n)
	{
		ZTcpSocket* client_sock(getSocket(n));
		if(client_sock)
		{
			client_sock -> close();
			delete client_sock;
			client_sock = NULL;
		}
	}
	clear();

	log_ -> log(ZLOG_DEBUG, 5, "    run() is finished\n");
}
