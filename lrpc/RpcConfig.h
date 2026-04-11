#ifndef _LEEF_LRPC_RPC_CONFIG_H
#define _LEEF_LRPC_RPC_CONFIG_H

#include <string>
#include <unordered_map>

/*
LRPC v1.0
- 配置文件格式: key=value，每行一个配置项，支持注释（以#开头的行）和空行
- 配置项示例:
port=6000
name=RpcServer
*/

namespace leef
{
    namespace rpc
    {
        class RpcConfig
        {
        public:
            bool load(const std::string &configPath = "./rpc.cfg");
            std::string getValue(const std::string &key) const;

        private:
            typedef std::unordered_map<std::string, std::string> ConfigMap;
            ConfigMap m_configMap;
        };
    }
}

#endif // _LEEF_LRPC_RPC_CONFIG_H