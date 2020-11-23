#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include "udprecvpool.h"
#include "tcpmcoutput.h"
#include "commandlineparameters.h"
#include <fstream>

std::atomic_bool running = true;
namespace
{
void signal_handler(int sig)
{
    running = false;
}
}

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
    auto output
        = std::make_shared<McTunnel::TcpMCOutput>(
                configuration,
                [output, reconnectionJobs](const std::shared_ptr<UDPRecv::DataStream>& ds,
                                           const std::pair<Networking::IpEndpoint, Networking::IpEndpoint>& group)
                {
                   reconnectionJobs->postJob(
                       [ds,group,output]()
                       {
                           if (output->addChannelForGroup(ds,group)){
                               std::cerr << "failed to reconnect\n";
                           }
                       });
                });

    receivers.addGroups([](const std::shared_ptr<UDPRecv::DataStream>& ds,
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
