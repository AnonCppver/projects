#ifndef _LEEF_RPC_LRPCCHANNEL_H
#define _LEEF_RPC_LRPCCHANNEL_H

#include <google/protobuf/service.h>

#include <atomic>

#include "lrpc.pb.h"
#include "../net/Endian.h"
#include "../base/Logging.h"

namespace leef
{
    namespace rpc
    {
        class TcpStream
        {
        public:
            bool connectToServer(const std::string &ip, int port);
            bool reconnect(const std::string &ip, int port);
            void close();
            bool isConnected() const;
            void setTimeout(int timeout);
            int getRetryCount();
            bool send(const std::string& message);
            bool read(char * buffer);
            ~TcpStream(){close();}
        private:
            bool doConnect(const std::string &ip, int port);
            int m_timeout = 5;
            int m_retryCount = 0;
            int m_fd;
        };
        class LRpcChannel : public google::protobuf::RpcChannel
        {
        public:
            LRpcChannel(const std::string &ip, int port, size_t bufferSize = 4 * 1024 * 1024) : m_ip(ip), m_port(port), m_bufferSize(bufferSize), m_id(0)
            {
                LOG_INFO << "LRpcChannel initialized with IP: " << m_ip << ", Port: " << m_port;
            }
            void CallMethod(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller,
                            const google::protobuf::Message *request,
                            google::protobuf::Message *response,
                            google::protobuf::Closure *done) override
            {
                const auto *serviceDescriptor = method->service();

                lrpc::RpcMessage rpcMessage;
                rpcMessage.set_service(serviceDescriptor->name());
                rpcMessage.set_method(method->name());
                rpcMessage.set_type(lrpc::REQUEST);

                rpcMessage.set_request(request->SerializeAsString());

                rpcMessage.set_id(++m_id);

                std::string strMessage;
                uint32_t msgLen = rpcMessage.ByteSizeLong();
                uint32_t net_msgLen = leef::net::sockets::hostToNetwork32(msgLen);
                strMessage.resize(sizeof(uint32_t) + rpcMessage.ByteSizeLong());

                memcpy(strMessage.data(), &net_msgLen, sizeof(uint32_t));
                rpcMessage.SerializeToArray(strMessage.data() + sizeof(uint32_t), msgLen);
                // TODO: send(packet)

                done->Run();
            }

        private:
            size_t m_bufferSize; // 4MB
            std::string m_ip;
            int m_port;
            std::atomic<int64_t> m_id;
            double m_timeout; // 单位:秒
            TcpStream m_client;
        };
    }
}

#endif // _LEEF_RPC_LRPCCHANNEL_H