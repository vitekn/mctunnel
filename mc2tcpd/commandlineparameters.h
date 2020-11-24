#ifndef MC2TCPD_COMMANDLINEPARAMETERS_H
#define MC2TCPD_COMMANDLINEPARAMETERS_H

#include <string>
#include <vector>

class CommandLineParameters
{
public:
    CommandLineParameters(int argc, char** argv):_parameters(){
        for (int i=0; i<argc; ++i) {
            _parameters.emplace_back(argv[i]);
        }
    }

    std::optional<std::string> getConfigFilePathName()
    {
        std::optional<std::string> res;
        auto it = find(_parameters.begin(), _parameters.end(), CONF_FILE_NAME_PARARM);
        if (std::distance(it, _parameters.end()) > 1) {
            ++it;
            res = *it;
        }
        return res;
    }

private:
    std::vector<std::string> _parameters;
    constexpr static char CONF_FILE_NAME_PARARM[] = "--config";
};

#endif //MC2TCPD_COMMANDLINEPARAMETERS_H
