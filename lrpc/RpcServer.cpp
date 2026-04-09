#include "RpcServer.h"

namespace leef
{
    namespace rpc
    {
        RpcService::RpcService()
        {
        }

        RpcService::~RpcService()
        {
        }

        bool RpcService::init(int argc, char *argv[])
        {
            return true;
        }

        void RpcService::provideService()
        {
        }

        void RpcService::start()
        {
        }

        void RpcService::stop()
        {
        }
        
        void RpcService::_help()
        {
            LOG_INFO << "Usage: ./rpc_server [options]";
            LOG_INFO << "Options:";
            LOG_INFO << "  -h, --help         Show this help message and exit";
            LOG_INFO << "  -p, --port <port>  Specify the port number to listen on (default: 8080)";
            LOG_INFO << "  -l, --log <file>   Specify the log file path (default: ./rpc_server.log)";
            LOG_INFO << "Example:";
            LOG_INFO << "  ./rpc_server -p 9090 -d -l /var/log/rpc_server.log";
        }
    }
}