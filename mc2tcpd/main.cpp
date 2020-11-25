#include "commandlineparameters.h"
#include "tcpmcoutput.h"
#include "udprecv.h"
#include "udprecvpool.h"

#include <csignal>
#include <iostream>
#include <fstream>

std::atomic_bool running = true;
namespace
{
void signal_handler(int sig)
{
    running = false;
}
}

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    CommandLineParameters cmd(argc, argv);

    const std::string cfgFileName = cmd.getConfigFilePathName().value_or("mc2tcpd.conf");
    std::ifstream cfgFileStream(cfgFileName);

    if (!cfgFileStream.is_open()) {
        return -1;
    }

    auto configuration = std::make_shared<McTunnel::Configuration>();
    if (!configuration->read(cfgFileStream)) {
        return -2;
    }

    auto reconnectionJobs = std::make_shared<JobQueue>();

    McTunnel::UDPRecvPool receivers(configuration);
    std::shared_ptr<McTunnel::TcpMCOutput> output
        = std::make_shared<McTunnel::TcpMCOutput>();
    output->setConnectionLostClb(
                [output, reconnectionJobs](const std::shared_ptr<McTunnel::UDPRecv::DataStream>& ds,
                                           const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>& group) mutable
                {
                   reconnectionJobs->postJob(
                       [ds,group,output]()
                       {
                           if (output->addChannelForGroup(ds,group)){
                               std::cerr << "failed to reconnect\n";
                           }
                       });
                });

    receivers.addGroups([output](const std::shared_ptr<McTunnel::UDPRecv::DataStream>& ds,
                           const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>& group)
                        {
                            output->addChannelForGroup(ds, group);
                        });

    while (running) {
        bool pending = reconnectionJobs->dispatchJob();
        if (!pending){
            std::this_thread::sleep_for(1ms);
        }
    }

    return 0;
}
