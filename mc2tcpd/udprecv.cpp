#include <udprecv.h>

#include <unistd.h>
#include <memory.h>

namespace McTunnel
{
using namespace std::chrono_literals;

UDPRecv::UDPRecv(std::shared_ptr<ObjectPool<ChunkType>> pool) noexcept
: _pool(pool)
, _sockets()
, _jobQueue()
, _socketCount(0)
, _thread([this](){this->run();})
{
}

void UDPRecv::addToSockets(std::shared_ptr<SocketStreamBundle> bndl)
{
    _sockets.emplace_back(std::move(*bndl));
    _socketCount.store(_sockets.size());

}

std::shared_ptr<UDPRecv::DataStream> UDPRecv::addMulticast(const Networking::IpEndpoint& endpoint) noexcept
{
    try {
        auto bndl = std::make_shared<SocketStreamBundle>(std::make_pair(std::make_unique<Networking::UdpSocket>(), std::make_shared<DataStream>(PoolAllocator<ChunkType>(_pool))));
        Networking::UdpSocket& ns = *(bndl->first.get());
        if (ns.setReuse(true) == Networking::SocketError::ERROR
            || ns.setBlocking(false) == Networking::SocketError::ERROR
            || ns.setReceiveTimeOut(100ms) == Networking::SocketError::ERROR) {
            return std::shared_ptr<DataStream>();
        }

        if(ns.bind(endpoint) == Networking::SocketError::ERROR) {
            return std::shared_ptr<DataStream>();
            /*log_ -> log(ZLOG_DEBUG, 5, "    binded %x, [%s]\n", ip, strerror(errno));*/
        }

        if(ns.addMembership(endpoint.ipAddress())  == Networking::SocketError::ERROR) {
            /*log_ -> log(ZLOG_DEBUG, 5, "    joined [%s]\n", strerror(errno));*/
            return std::shared_ptr<DataStream>();
        }

        auto res = bndl->second;
        _jobQueue.postJob(std::move([bndl=bndl, this]() mutable {
            this->addToSockets(bndl);
        }));
        //log_ -> log(ZLOG_DEBUG, 5, "    finished %p, [%s]\n", ns, strerror(errno));
        return res;
    } catch (std::bad_alloc&) {
        return std::shared_ptr<DataStream>();
    }
}

int UDPRecv::getSocketsCount() noexcept
{
    return _socketCount.load();
}

void UDPRecv::run()
{
    _jobQueue.dispatchJob();

    bool idle = true;

    for (auto& bndl: _sockets) {
        UdpPacket udpPacket;
        Networking::SocketError res = readFromSocket(*(bndl.first.get()), udpPacket);
        if (res == Networking::SocketError::SUCCESS) {
            if (bndl.second->write(udpPacket.getData().data(), udpPacket.getSize())) {
//                log_->log(ZLOG_DEBUG, 2, "UDPRecv::run()    read ip %x data res %i\n", (*it)->getBindedIp(),
  //                        res);
            } else {
    //            log_->log(ZLOG_WARNING, 2,
      //                    "UDPRecv::run()     buffer overflow dropping packet %x, size %i [%s]\n", ntohl(u->ip),
        //                  res, strerror(errno));
            }
            idle = false;
        } else if (res == Networking::SocketError::ERROR) {
//            log_->log(ZLOG_WARNING, 5, "UDPRecv::run()    errno != EAGAIN: [%s]\n", strerror(errno));
        }
    }

    if(idle) {
        std::this_thread::sleep_for(1ms);
    }
}
}