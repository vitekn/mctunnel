#ifndef TCPMCOUTPUT_H
#define TCPMCOUTPUT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

#include "libzet/exthreads.h"
#include "libzet/ztcpsocket.h"
#include "libzet/logger.h"
#include "configuration.h"
#include "udprecv.h"
#include "tcpsocket.h"
#include "jobqueue.h"
#include "rjthread.h"

using std::map;

namespace McTunnel{

class TcpMCOutput
{
    struct DataChannel
    {
        std::shared_ptr<UDPRecv::DataStream> stream;
        std::unique_ptr<Networking::TcpSocket> socket;
        UDPRecv::DataStream::ChunkPtr chunkInUse;
        Networking::IpEndpoint mcGroup;
    };


public:
    using ConnectionLostClb = std::function<void(const std::shared_ptr<UDPRecv::DataStream>&, const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>&)>;
    TcpMCOutput(std::shared_ptr<Configuration> conf, const ConnectionLostClb& clclb);

    bool addChannelForGroup(const std::shared_ptr<UDPRecv::DataStream>& ds, const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>& group);

private:
    void run();

    void createTcpSocketFor(std::unique_ptr<DataChannel> dch);

    bool connect2Interface(u_int32_t);
    void deleteTcpSockets();

private:
    std::shared_ptr<Configuration> _conf;
    std::vector<std::unique_ptr<DataChannel>> _dataStreamToSocket;
    JobQueue _jobQueue;
    ConnectionLostClb _connectionLostCb;
    RjThread _dataThread;
};
}
#endif
