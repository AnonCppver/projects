#ifndef _LEEF_LRPC_RPC_SERVER_H
#define _LEEF_LRPC_RPC_SERVER_H

#include "../base/noncopyable.h"
#include "../base/Logging.h"

#include "../net/EventLoop.h"
#include "../net/TcpServer.h"
#include "../net/InetAddress.h"

#include "lrpc.pb.h"

#include <google/protobuf/service.h>

#include <string>
#include <memory>

namespace leef
{
    namespace rpc
    {
        class LambdaClosure : public google::protobuf::Closure
        {
        public:
            explicit LambdaClosure(std::function<void()> fn) : fn_(std::move(fn)) {}
            void Run() override
            {
                fn_();
                delete this;
            }

        private:
            std::function<void()> fn_;
        };
        struct ServiceInfo
        {
            google::protobuf::Service *m_service;
            // key: methodName, value: methodDescriptor
            std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;
        };
        class RpcServer : public noncopyable
        {
        public:
            RpcServer(int port = 6000, const std::string &nameStr = "RpcServer") : m_port(port), m_name(nameStr) {}
            void setThreadNum(int numThreads);
            void addService(google::protobuf::Service *service);
            void run();

        private:
            void onConnection(const leef::net::TcpConnectionPtr &conn);
            void onMessage(const leef::net::TcpConnectionPtr &conn, leef::net::Buffer *buf, leef::Timestamp receiveTime);
            // LambdaClosure 持有了 response 的 shared_ptr，确保在发送响应前 response 不被销毁，这里可以用引用
            void sendResponse(const leef::net::TcpConnectionPtr &conn, const std::shared_ptr<google::protobuf::Message> &response, int64_t id);
            void sendError(const leef::net::TcpConnectionPtr &conn, int64_t id, lrpc::ErrorCode errorCode);
            std::unique_ptr<leef::net::TcpServer> m_tcpServerPtr;
            std::unique_ptr<leef::net::EventLoop> m_eventLoopPtr;
            // key: serviceName, value: ServiceInfo
            std::unordered_map<std::string, ServiceInfo> m_serviceMap;
            int m_threadNum = 0; // 默认线程数
            int m_port = 0;      // 监听端口
            std::string m_name;
        };
    } // namespace rpc
} // namespace leef

#endif // _LEEF_LRPC_RPC_SERVER_H