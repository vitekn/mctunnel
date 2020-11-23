#include "udprecvpool.h"
#include "libzet/global.h"

namespace McTunnel
{

UDPRecvPool::UDPRecvPool(std::shared_ptr<Configuration> conf)
: _conf(conf)
, _recvThreads(_conf->receiveThreads())
, _bufferPool(std::make_shared<ObjectPool<UDPRecv::ChunkType>>(_conf->bufferSize()/UDPRecv::ChunkType::MAX_SIZE))
{
//    _logger -> log(ZLOG_DEBUG, 5, "UDPRecvPool::UDPRecvPool()\n");
  //  _logger -> log(ZLOG_NOTICE, 5, "    threads_count %d, group_count %i\n", threads_count, grp_cnt);

    for (auto& ptr: _recvThreads) {
        ptr = std::make_unique<UDPRecv>(_bufferPool);
    }
}

bool UDPRecvPool::addGroups(const OnAddGroupCb& addCb)
{
    for (size_t i=0; i<_conf->mcGroups().size(); ++i) {
        size_t thNum = i % _conf->receiveThreads();
        const Networking::IpEndpoint& ep = _conf->mcGroups()[i].first;
        std::shared_ptr<UDPRecv::DataStream> ds = _recvThreads[thNum]->addMulticast(ep);
        if (!ds) {
            return false;
        }
        addCb(ds, _conf->mcGroups());
    }
    return true;
}

}