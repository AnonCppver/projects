#ifndef _LEEF_LRPC_RPC_SERVICE_H
#define _LEEF_LRPC_RPC_SERVICE_H

#include "../base/noncopyable.h"
#include "../base/Logging.h"

#include <string>

namespace leef
{
    namespace rpc
    {
        class RpcService : public noncopyable
        {
        public:
            RpcService();
            ~RpcService();
            bool init(int argc, char *argv[]);
            void provideService();
            void start();
            void stop();

        private:
        void _help();
        };
    }
}

#endif // _LEEF_LRPC_RPC_SERVICE_H