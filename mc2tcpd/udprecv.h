#ifndef UDPRECV_H
#define UDPRECV_H

#include <vector>

#include <udpsocket.h>
#include "chunckedstream.h"
#include "udppacket.h"
#include <memory>
#include <mutex>
#include <PoolAllocator.h>
#include <rjthread.h>
#include <atomic>
#include "jobqueue.h"

namespace McTunnel{

class UDPRecv
{
public:
    using ChunkType = Chunk<UdpPacket::MAX_DATA_SIZE>;
    using DataStream = ChunkedStream< ChunkType, PoolAllocator<ChunkType>, std::mutex>;

private:
    using SocketStreamBundle = std::pair<std::unique_ptr<Networking::UdpSocket>, std::shared_ptr<DataStream>>;

public:
    explicit UDPRecv(std::shared_ptr<ObjectPool<ChunkType>> pool) noexcept;
    int getSocketsCount() noexcept;
    std::shared_ptr<DataStream> addMulticast(const Networking::IpEndpoint& endpoint) noexcept;

private:
    void addToSockets(std::shared_ptr<SocketStreamBundle> bndl);
    void dispatchJob();
    void run();

//    ZLogger* log_;
    std::shared_ptr<ObjectPool<ChunkType>> _pool;
    std::vector<SocketStreamBundle>        _sockets;
    JobQueue                               _jobQueue;
    std::atomic_size_t                     _socketCount;
    RjThread                               _thread;
};
}
#endif
