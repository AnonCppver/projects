#ifndef _LEEF_RPC_LRPCCHANNEL_H
#define _LEEF_RPC_LRPCCHANNEL_H

#include <google/protobuf/service.h>

#include <atomic>

#include "lrpc.pb.h"
#include "../net/Endian.h"
#include "../net/InetAddress.h"
#include "../base/Logging.h"

namespace leef
{
    namespace rpc
    {
        class LRpcChannel : public google::protobuf::RpcChannel
        {
        public:
            LRpcChannel(const std::string &ip, int port, int timeout = 5, size_t bufferSize = 4 * 1024 * 1024);
            void CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done) override;

        private:
            size_t m_bufferSize; // 4MB
            leef::net::InetAddress m_addr;
            std::atomic<int64_t> m_id;
            int m_timeout; // 单位:秒
            int m_fd=-1;
        };
    }
}

#endif // _LEEF_RPC_LRPCCHANNEL_H