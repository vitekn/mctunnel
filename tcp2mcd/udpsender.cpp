#include "udpsender.h"
#include "udppacket.h"

#include <unistd.h>
#include <errno.h>
#include <cstring>

using namespace Configuration;

static int k = 0;

UDPSender::UDPSender(Depacker* dp): depacker_(dp), log_(ZLogger::Instance()), conf_(LibConf::Instance())
{
	socket_.setTransmitTimeOut(100);
	socket_.setReuse(true);
	short src_port = conf_ -> getSourcePort();
	if(socket_.bind(INADDR_ANY, src_port))
		log_ -> log(ZLOG_NOTICE, 5, "UDPSender sock binded: src_ip = %s, src_port = %d [%s]\n", ipaddrToStr(INADDR_ANY).c_str(), src_port, strerror(errno));
	else
		log_ -> log(ZLOG_WARNING, 5, "UDPSender sock not binded: src_ip = %s, src_port = %d [%s]\n", ipaddrToStr(INADDR_ANY).c_str(), src_port, strerror(errno));
}

void UDPSender::run()
{
	int t = 0;
	while(!is_finish)
	{
		if(depacker_)
		{
            UdpPacket *up(depacker_ -> getPacket());
			if(up)
			{
				if(socket_.isWriteReady(0, 10 * 1000))
				{
					int sr = socket_.writeDataTo(up -> data, up -> size, up -> ip/* | 0x00000100*/);
                    depacker_->decreaseBuffNow(up->size);
					//if(!(t % 1000))
					log_ -> log(ZLOG_DEBUG, 5, "UDPSender::run()     packet ip %x %i [%s]\n", up -> ip, sr, strerror(errno));
					++t;
					char* buf = (char*)up;
					delete[] buf;
                    k++;
                    log_ -> log(ZLOG_DEBUG, 5, "UDPSender::run()     delete data in %x, del count %i\n",buf, k);
					buf = NULL;
				}
                else
                    log_ -> log(ZLOG_ERR, 1, "UDPSender::run():    socket (%i) isn't ready for write [%s]\n", socket_.socketDescriptor(), strerror(errno));
            }
            //else
            //    usleep(1000);
		}
		else
			log_ -> log(ZLOG_WARNING, 1, "UDPSender::run():    depacker (%p) is not initialized [%s]\n", depacker_, strerror(errno));
	}
}
