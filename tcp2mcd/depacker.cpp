#include "depacker.h"
#include "libzet/global.h"
#include "libzet/securequeue.h"
#include "libzet/securemap.h"

#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <endian.h>
#include <errno.h>

using namespace Configuration;
using namespace std;

static u_int64_t prev_p = 0;
static InfoMap null_element;
static int t = 0;

Depacker::Depacker(): log_(ZLogger::Instance()), conf_(LibConf::Instance()), timer_(0), max_size_buf_(conf_ -> bufSize()), sizebuff_now(0)
{
	null_element.written_data = 0;
	null_element.data_start = true;
    max_size_buf_one_ip = conf_ -> bufSize()/40;
}

UdpPacket* Depacker::getPacket()
{
	UdpPacket *res(NULL);

	if(buf_map_.isValidIterator())
	{
		for(; buf_map_.isValidIterator(); buf_map_.next())
		{
			InfoMap* finded_item(buf_map_.find(buf_map_.key()));
			if(!finded_item)
				continue;
			if(finded_item -> data_start || !finded_item -> info_queue.size())
			{
				finded_item -> unlock();
				continue;
			}

			InfoQueue data_item(finded_item -> info_queue.element());
			UdpPacket* u(fromNet(data_item.data));
			if(!timer_)
				timer_ = u -> time_stamp;

            //log_ -> log(ZLOG_DEBUG, 1, "Depacker::getPacket() pack ip %x, time_stamp = %llu, prev_p = %llu\n", u -> ip, u -> time_stamp, prev_p);
            log_ -> log(ZLOG_DEBUG, 1, "Depacker::getPacket() pack ip %x, time_stamp = %llu, timer_ = %llu, getTime()=%llu, getTime()-timer_=%llu\n", u -> ip, u -> time_stamp, timer_
                        ,getTime(), (getTime()-timer_) );

			if(u -> time_stamp > getTime() - timer_)
			{
				finded_item -> unlock();
				log_ -> log(ZLOG_DEBUG, 1, "Depacker::getPacket (time_stamp > getTime() - timer) pack ip %x, time_stamp = %llu, prev_p = %llu\n", u -> ip, u -> time_stamp,
							prev_p);
				continue;
			}
			else
			{
                log_ -> log(ZLOG_DEBUG, 5, "Depacker::getPacket() pack ip %x, time_stamp = %llu, pack_delta = %llu, systimer = %llu\n", u -> ip,
                            u -> time_stamp, u -> time_stamp - prev_p, getTime() - timer_);

				prev_p = u -> time_stamp;
				data_item = finded_item -> info_queue.pop();
				res = (UdpPacket*)data_item.data;

				finded_item -> unlock();
				/*if(!(t % 1000))
					log_ -> log(ZLOG_DEBUG, 5, "Depacker::getPacket() ret ip %x %p", u -> ip, res);
				++t;*/
				return res;
			}
		}
	}
	else
	{
        //log_ -> log(ZLOG_WARNING, 5, "Depacker::getPacket() buf_map_ has not valid iterators, buf_map_ size = %i\n", buf_map_.size());
        buf_map_.resetIterator();
        //usleep(1000);
		return res;
	}

	return res;
}

UdpPacket* Depacker::fromNet(char* buf)
{
	UdpPacket* u((UdpPacket*)buf);
	u -> ip = ntohl(u -> ip);
	u -> port = ntohs(u -> port);
	u -> size = ntohs(u -> size);
	u -> time_stamp = be64toh(u -> time_stamp);
	return u;
}

bool Depacker::writeData(char* data, int size)
{
    //sizebuff_now += size;
    //if(! (sizebuff_now%100))
        //log_ -> log(ZLOG_NOTICE, 5, "Depacker::writeData()   size=%i\n", sizebuff_now);
    //return false;
    if(sizebuff_now > max_size_buf_) {
       return false;
    }
    u_int32_t ip = ntohl(((UdpPacket*)data) -> ip);
	InfoMap* finded_item(buf_map_.find(ip));
	if(!finded_item)
	{
		buf_map_.insert(ip, null_element);
		log_ -> log(ZLOG_DEBUG, 5, "Depacker::writeData()    insert in map ip %x\n", ip);
		finded_item = buf_map_.find(ip);
	}

	InfoQueue item;
	item.size = size;
    item.data = new char[size];
    sizebuff_now += size;
    t++;
    log_ -> log(ZLOG_DEBUG, 5, "Depacker::writeData()    allocate data to %x in %i bytes for ip %x all count %i all size %i \n",item.data, size, ip, t, sizebuff_now);
	memcpy(item.data, data, size);
	finded_item -> info_queue.push(item);

    if(finded_item -> data_start)
	{
		finded_item -> written_data += size;
        log_ -> log(ZLOG_DEBUG, 5, "Depacker::writeData()    ip %x written_data = %d, max_size_buf_one_ip  = %i\n", ip, finded_item -> written_data,
                    max_size_buf_one_ip);
        if(finded_item -> written_data > max_size_buf_one_ip)
			finded_item -> data_start = false;
	}

	finded_item -> unlock();
	return true;
}
