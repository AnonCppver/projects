#include "RpcConfig.h"
#include "../base/Logging.h"
#include "../base/strUtil.h"

#include <fstream>

using namespace std;
using namespace leef;
using namespace leef::rpc;

bool RpcConfig::load(const std::string &configPath)
{
    std::ifstream ifs(configPath);
    if (!ifs.is_open())
    {
        LOG_ERROR << "Failed to open config file: " << configPath;
        return false;
    }
    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty() || line[0] == '#')
            continue; // Skip empty lines and comments
        auto pos = line.find('=');
        if (pos == std::string::npos)
            continue; // Skip lines without '='
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        m_configMap[trim2(key)] = trim2(value);
    }
    return true;
}

std::string RpcConfig::getValue(const std::string &key) const
{
    static const std::string emptyStr;
    auto it = m_configMap.find(key);
    if (it != m_configMap.end())
        return it->second;
    return emptyStr;
}
