#include "tcpmcoutput.h"
#include "udppacket.h"
#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

namespace McTunnel{
using namespace std::chrono_literals;

TcpMCOutput::TcpMCOutput(std::shared_ptr<Configuration> conf) noexcept
: _conf(conf)
, _dataStreamToSocket()
, _jobQueue()
, _connectionLostCb()
, _dataThread([this](){this->run();})
{}

void TcpMCOutput::setConnectionLostClb(const ConnectionLostClb &clclb)
{
    _connectionLostCb = clclb;
}

bool TcpMCOutput::addChannelForGroup(const std::shared_ptr <UDPRecv::DataStream>& ds, const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>& group)
{
    auto sockPtr = std::make_unique<Networking::TcpSocket>();

    if (sockPtr->setTransmitTimeOut(2000ms) == Networking::SocketError::ERROR
        || sockPtr->setReceiveTimeOut(2000ms)  == Networking::SocketError::ERROR
        || sockPtr->setSndBufSize(6 * 1024 * 1024) == Networking::SocketError::ERROR
        || sockPtr->setRcvBufSize(6 * 1024 * 1024) == Networking::SocketError::ERROR) {
        return false;
    }

    if (sockPtr->connect(group.second) == Networking::SocketError::ERROR) {
        return false;
    }
    auto ptr = std::make_shared<DataChannel>(DataChannel{ds,std::move(sockPtr),UDPRecv::DataStream::ChunkPtr(), group.first});
    _jobQueue.postJob(std::move([ptrl=std::move(ptr),this]() mutable {
        this->createTcpSocketFor(std::move(ptrl));
    }));
    return true;
}

void TcpMCOutput::createTcpSocketFor(std::shared_ptr<DataChannel> dch)
{
    _dataStreamToSocket.emplace_back(std::move(dch));
}

void TcpMCOutput::run()
{
    _jobQueue.dispatchJob();

    bool needCleanup=false;
    bool dryRun = true;
    for (auto& dch: _dataStreamToSocket) {
        if (!dch->chunkInUse || dch->chunkInUse->readPos == dch->chunkInUse->size) {
            (*dch->stream) >> dch->chunkInUse;
        }
        if (dch->chunkInUse) {
            dryRun = false;
            size_t written;
            Networking::SocketError err;
            std::tie(err,written) = dch->socket->writeData(dch->chunkInUse->buf.data() + dch->chunkInUse->readPos,
                                                           dch->chunkInUse->size - dch->chunkInUse->readPos);
            if (err == Networking::SocketError::SUCCESS){
                dch->chunkInUse->readPos += written;
            } else {
                dch->stream->revertChunk(std::move(dch->chunkInUse));
                Networking::IpEndpoint gr = dch->mcGroup;
                Networking::IpEndpoint ip = dch->socket->connectedEndpoint();
                auto ds = dch->stream;
                dch = nullptr;
                needCleanup = true;
                if (_connectionLostCb) {
                    _connectionLostCb(ds, std::make_pair(gr, ip));
                }
            }
        }
    }
    if (needCleanup) {
        auto it = std::find(_dataStreamToSocket.begin(), _dataStreamToSocket.end(), nullptr);
        while (it!=_dataStreamToSocket.end()){
            it = _dataStreamToSocket.erase(it);
            it = std::find(it, _dataStreamToSocket.end(), nullptr);
        }
    }
    if (dryRun){
        std::this_thread::sleep_for(1ms);
    }

}
}