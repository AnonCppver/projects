#ifndef _LEEF_RPC_LRPCCHANNEL_H
#define _LEEF_RPC_LRPCCHANNEL_H

#include <google/protobuf/service.h>

#include "lrpc.pb.h"
#include "../base/Logging.h"

namespace leef
{
    namespace rpc
    {
        class LRpcChannel : public google::protobuf::RpcChannel
        {
        public:
            LRpcChannel(const std::string& ip, int port, size_t bufferSize = 4 * 1024) : m_ip(ip), m_port(port), m_bufferSize(bufferSize)
            {
                LOG_INFO << "LRpcChannel initialized with IP: " << m_ip << ", Port: " << m_port;
            }
            // 消息格式：[header_size][args_size][msg_proto[service,method]][msg_args]
            void CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done) override
            {
                const google::protobuf::ServiceDescriptor *sd = method->service();
                const std::string &service_name = sd->name();
                const std::string &method_name = method->name();

                // rpc header
                lrpc::RpcRequest rpcHeader;
                rpcHeader.set_service(service_name);
                rpcHeader.set_method(method_name);

                // 计算 header 大小
                int32_t header_size = rpcHeader.ByteSizeLong();
                std::string strRequest = request->SerializeAsString();
                int32_t args_size = strRequest.size();
                int64_t msgLen = ((int64_t)header_size << 32) | args_size;
                int64_t net_msgLen = htobe64(msgLen);

                std::string strMessage;
                strMessage.resize(sizeof(int64_t) + header_size + args_size);

                // header size + args size
                memcpy(strMessage.data(), &net_msgLen, sizeof(int64_t));
                // proto
                rpcHeader.SerializeToArray(strMessage.data() + sizeof(int64_t), header_size);
                // args
                memcpy(strMessage.data() + sizeof(int64_t) + header_size, strRequest.data(), args_size);

                done->Run();
            }

        private:
            size_t m_bufferSize; // 4KB
            std::string m_ip;
            int m_port;
        };
    }
}

#endif // _LEEF_RPC_LRPCCHANNEL_H