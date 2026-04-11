#ifndef _LEEF_LRPC_RPC_SERVER_H
#define _LEEF_LRPC_RPC_SERVER_H

#include "../base/noncopyable.h"
#include "../base/Logging.h"

#include "../net/EventLoop.h"
#include "../net/TcpServer.h"
#include "../net/InetAddress.h"

#include <google/protobuf/service.h>

#include <string>
#include <memory>

namespace leef
{
    namespace rpc
    {
        struct ServiceInfo
        {
            google::protobuf::Service *m_service;
            // key: methodName, value: methodDescriptor
            std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;
        };
        class RpcServer : public noncopyable
        {
        public:
            void setThreadNum(int numThreads);
            void addService(google::protobuf::Service *service);
            void run();

        private:
            void onConnection(const leef::net::TcpConnectionPtr &conn);
            void onMessage(const leef::net::TcpConnectionPtr &conn, leef::net::Buffer *buf, leef::Timestamp receiveTime);
            void sendResponse(const leef::net::TcpConnectionPtr &conn, google::protobuf::Message *response);
            std::unique_ptr<leef::net::TcpServer> m_tcpServerPtr;
            std::unique_ptr<leef::net::EventLoop> m_eventLoopPtr;
            // key: serviceName, value: ServiceInfo
            std::unordered_map<std::string, ServiceInfo> m_serviceMap;
            std::string m_successResponse=""; // 缓存调用成功的响应
            int m_threadNum = 0; // 默认线程数
        };
    } // namespace rpc
} // namespace leef

#endif // _LEEF_LRPC_RPC_SERVER_H