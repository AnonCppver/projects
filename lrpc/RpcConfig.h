#ifndef _LEEF_LRPC_RPC_CONFIG_H
#define _LEEF_LRPC_RPC_CONFIG_H

#include <string>
#include <unordered_map>

namespace leef
{
    namespace rpc
    {
        class RpcConfig
        {
        public:
            bool load(const std::string &configPath = "./rpc_config.json");
            std::string getValue(const std::string &key) const;

        private:
            typedef std::unordered_map<std::string, std::string> ConfigMap;
            ConfigMap m_configMap;
        };
    }
}

#endif // _LEEF_LRPC_RPC_CONFIG_H