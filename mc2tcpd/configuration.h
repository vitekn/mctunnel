#ifndef MC2TCPD_CONFIGURATION_H
#define MC2TCPD_CONFIGURATION_H

#include "ipaddress.h"
#include <vector>

#include <istream>

namespace McTunnel
{

class Configuration
{
public:
    Configuration (): _groupsToTcp(), _recvThreads(0), _bufferSize(0){}

    bool read(std::istream& stream)
    {
        bool res=true;
        while (!stream.eof() && res) {
            std::string line;
            std::getline(stream, line);
            removeSpacesAndComments(line);
            if (line.find_first_of(BUFFER_SIZE)==0){
                res=readValues(BUFFER_SIZE, line, &_bufferSize);
            } else
            if (line.find_first_of(RECEIVE_THREADS)==0){
                res=readValues(RECEIVE_THREADS, line, &_recvThreads);
            } else
            if (line.find_first_of(GROUP_TO_TCP)==0){
                Networking::IpEndpoint eps[2];
                res=readValues(GROUP_TO_TCP, line, &eps[0], 2);
            }
        }
        return res && _recvThreads > 0 && _bufferSize > 0 && !_groupsToTcp.empty();
    }

    [[nodiscard]] size_t bufferSize() const { return _bufferSize;}
    [[nodiscard]] size_t receiveThreads() const { return _recvThreads; }
    [[nodiscard]] const std::vector<std::pair<Networking::IpEndpoint, Networking::IpEndpoint>>& mcGroupsToTcp() const { return _groupsToTcp;}

private:
    template<class T>
    bool readValues(const std::string& name, const std::string& line, T* result, size_t count = 1, const std::string& delim = ",")
    {
        std::string::size_type pos = line.find_first_not_of('=',name.size());
        if (pos != std::string::npos) {
            std::istringstream iss(line.substr(pos));
            size_t ct = 0;
            while (ct < count) {
                iss >> (*(result + ct));
                ++ct;
                if (!iss.eof() || iss.fail()) {
                    return false;
                }
            }
        }
        return true;
    }

    static void removeSpacesAndComments(std::string& str)
    {
        int nl=0;
        for (int i=0; i<str.size() && str[i]!='#'; ++i){
            char cc = str[i];
            if (!(cc==' ' || cc=='\t' || cc=='\r' || cc=='\n')){
                str[nl]=str[i];
                ++nl;
            }
        }
        str.resize(nl);
    }

    std::vector<std::pair<Networking::IpEndpoint, Networking::IpEndpoint>> _groupsToTcp;
    size_t _recvThreads;
    size_t _bufferSize;
    constexpr static const char BUFFER_SIZE[] = "buffer_size";
    constexpr static const char RECEIVE_THREADS[] = "receive_threads_count";
    constexpr static const char GROUP_TO_TCP[] = "group_to_tcp";
};

}

#endif //MC2TCPD_CONFIGURATION_H
