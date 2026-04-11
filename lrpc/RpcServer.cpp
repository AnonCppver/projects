#include "RpcServer.h"
#include "RpcConfig.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "../net/TcpConnection.h"

#include "lrpc.pb.h"

using namespace std::placeholders;

namespace leef
{
    namespace rpc
    {
        void RpcServer::setThreadNum(int numThreads)
        {
            m_threadNum = std::max(m_threadNum, numThreads);
        }

        void RpcServer::addService(google::protobuf::Service *service)
        {
            const google::protobuf::ServiceDescriptor *desc = service->GetDescriptor();
            std::string serviceName = desc->name();
            ServiceInfo serviceInfo;
            serviceInfo.m_service = service;
            int methodCount = desc->method_count();
            serviceInfo.m_methodMap.reserve(methodCount);
            for (int i = 0; i < methodCount; ++i)
            {
                const google::protobuf::MethodDescriptor *methodDesc = desc->method(i);
                serviceInfo.m_methodMap.emplace(methodDesc->name(), methodDesc);
            }
            m_serviceMap.emplace(serviceName, std::move(serviceInfo));
            LOG_INFO << "Added service: " << serviceName << " with " << methodCount << " methods.";
        }

        void RpcServer::onConnection(const leef::net::TcpConnectionPtr &conn)
        {
            if (!conn->connected())
            {
                conn->shutdown();
            }
        }

        // string_view style 处理，避免拷贝
        // 消息格式：[header_size][args_size][msg_proto[service,method]][msg_args]
        // 注意: size需要考虑主机字节序与网络字节序的转换
        void RpcServer::onMessage(const leef::net::TcpConnectionPtr &conn, leef::net::Buffer *buf, leef::Timestamp receiveTime)
        {
            while (true)
            {
                // 报文长度解析
                if (buf->readableBytes() < 8)
                {
                    return; // 不够读取消息长度
                }
                int64_t msgLen = buf->peekInt64();
                int32_t header_size = msgLen >> 32;
                int32_t args_size = msgLen & 0xFFFFFFFF;
                if (header_size + args_size > 4 * 1024) // 限制消息大小，防止恶意攻击
                {
                    LOG_ERROR << "Message size too large: " << header_size + args_size;
                    conn->shutdown();
                    return;
                }
                if (buf->readableBytes() < sizeof(int64_t) + header_size + args_size)
                {
                    return; // 不够读取消息内容
                }
                // 请求头解析
                lrpc::RpcRequest rpcRequest;
                if (!rpcRequest.ParseFromArray(buf->peek() + sizeof(int64_t), header_size))
                {
                    LOG_ERROR << "Failed to parse RpcRequest";
                    return;
                }
                const std::string &strService = rpcRequest.service();
                const std::string &strMethod = rpcRequest.method();

                auto it = m_serviceMap.find(strService);
                if (it == m_serviceMap.end())
                {
                    // 没有找到对应的服务
                    return;
                }
                const ServiceInfo &serviceInfo = it->second;
                auto mit = serviceInfo.m_methodMap.find(strMethod);
                if (mit == serviceInfo.m_methodMap.end())
                {
                    // 没有找到对应的方法
                    return;
                }
                const google::protobuf::MethodDescriptor *methodDesc = mit->second;
                LOG_INFO << "Received request for service: " << strService << ", method: " << strMethod;

                google::protobuf::Service *servicePtr = serviceInfo.m_service;
                auto request = std::unique_ptr<google::protobuf::Message>(servicePtr->GetRequestPrototype(methodDesc).New());
                auto response = std::unique_ptr<google::protobuf::Message>(servicePtr->GetResponsePrototype(methodDesc).New());

                // 绑定closure回调，调用service的CallMethod时会执行这个回调来发送响应
                google::protobuf::Closure *done =
                    google::protobuf::NewCallback<RpcServer, const leef::net::TcpConnectionPtr &, google::protobuf::Message *>(this, &RpcServer::sendResponse, conn, response.get());

                if (!request->ParseFromArray(buf->peek() + sizeof(int64_t) + header_size, args_size))
                {
                    LOG_ERROR << "Failed to parse request for service: " << strService << ", method: " << strMethod;
                    return;
                }
                servicePtr->CallMethod(methodDesc, nullptr, request.get(), response.get(), done);

                buf->retrieve(sizeof(int64_t) + header_size + args_size); // 处理完消息后，移动读指针
            }
        }

        void RpcServer::run()
        {
            RpcConfig config;
            if (!config.load("./rpc.cfg"))
            {
                LOG_ERROR << "Failed to load RPC configuration";
                exit(1);
            }
            std::string portStr = config.getValue("port");
            std::string nameStr = config.getValue("name");
            if (portStr.empty())
                portStr = "6000";
            if (nameStr.empty())
                nameStr = "RpcServer";
            int port = std::stoi(portStr);
            m_eventLoopPtr = std::make_unique<leef::net::EventLoop>();
            m_tcpServerPtr = std::make_unique<leef::net::TcpServer>(m_eventLoopPtr.get(), leef::net::InetAddress(port), nameStr, leef::net::TcpServer::kReusePort);
            m_tcpServerPtr->setThreadNum(m_threadNum);

            m_tcpServerPtr->setConnectionCallback(
                [this](const leef::net::TcpConnectionPtr &conn)
                {
                    this->onConnection(conn);
                });

            m_tcpServerPtr->setMessageCallback(
                [this](const leef::net::TcpConnectionPtr &conn,
                       leef::net::Buffer *buf,
                       leef::Timestamp receiveTime)
                {
                    this->onMessage(conn, buf, receiveTime);
                });

            LOG_INFO << "Starting RPC Server on port " << port << " with " << m_threadNum << " threads.";
            m_tcpServerPtr->start();
            m_eventLoopPtr->loop();
        }
        // response响应格式
        // [size][msg_proto[err_code,err_msg,result]]
        void RpcServer::sendResponse(const leef::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
        {

            size_t bodySize = response->ByteSizeLong();

            std::string buffer;
            buffer.resize(sizeof(int32_t) + bodySize);

            char *ptr = buffer.data();

            int32_t len = static_cast<int32_t>(bodySize);
            int32_t netLen = leef::net::sockets::hostToNetwork32(len);
            memcpy(ptr, &netLen, sizeof(int32_t));

            response->SerializeToArray(ptr + sizeof(int32_t), bodySize);

            conn->send(buffer);
        }
    }
}