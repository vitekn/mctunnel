#ifndef UDPRECVPOOL_H
#define UDPRECVPOOL_H

#include "udprecv.h"
#include "udptcpstreamer.h"
#include "configuration.h"
#include "objectpool.h"
#include "ipaddress.h"

#include <vector>
#include <memory>
namespace McTunnel{

    class UDPRecvPool
{
public:
    using OnAddGroupCb = std::function<void(std::shared_ptr<UDPRecv::DataStream>, const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>&)>;

    UDPRecvPool(std::shared_ptr<Configuration> conf);

    bool addGroups(const OnAddGroupCb& addCb);

private:
//    ZLogger *_logger;
    std::shared_ptr<Configuration> _conf;
    std::vector<std::unique_ptr<UDPRecv>> _recvThreads;
    std::shared_ptr<ObjectPool<UDPRecv::ChunkType>> _bufferPool;

};
}
#endif
