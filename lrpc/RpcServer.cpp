#include "RpcServer.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

using namespace std::placeholders;

namespace leef
{
    namespace rpc
    {
        class BufCleaner
        {
        public:
            BufCleaner(leef::net::Buffer *buf, size_t len) : m_buf(buf), m_len(len) {}
            ~BufCleaner()
            {
                if (m_buf)
                {
                    m_buf->retrieve(sizeof(int32_t) + m_len);
                }
            }

        private:
            leef::net::Buffer *m_buf;
            size_t m_len;
        };

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

        // 使用协议 size(4字节) + RpcMessage 来解析请求 多一次内存拷贝，但更清晰且易于调试，也更容易扩展协议
        // header_size args_size protobuf args
        void RpcServer::onMessage(const leef::net::TcpConnectionPtr &conn, leef::net::Buffer *buf, leef::Timestamp receiveTime)
        {
            while (true)
            {
                std::string tcpInfo = conn->getTcpInfoString();
                // 报文长度解析
                if (buf->readableBytes() < 4)
                {
                    return; // 不够读取消息长度
                }
                int32_t msgLen = buf->peekInt32();
                if (msgLen > 4 * 1024 * 1024) // 限制消息大小，防止恶意攻击
                {
                    double mib = static_cast<double>(msgLen) / (1024.0 * 1024.0);
                    double mib2 = static_cast<long long>(mib * 100 + 0.5) / 100.0;

                    std::string tcpInfo = conn->getTcpInfoString();
                    LOG_ERROR << tcpInfo << " Message size too large: " << mib2 << " MiB";
                    conn->shutdown();
                    return;
                }
                if (buf->readableBytes() < sizeof(int32_t) + msgLen)
                {
                    return; // 不够读取消息内容
                }
                BufCleaner cleaner(buf, msgLen); // 确保函数退出时正确移动读指针
                // 请求头解析
                lrpc::RpcMessage rpcMessage;
                if (!rpcMessage.ParseFromArray(buf->peek() + sizeof(int32_t), msgLen))
                {
                    sendError(conn, 0, lrpc::PROTO_ERROR);
                    LOG_ERROR << "Failed to parse RpcMessage";
                    return;
                }
                const std::string &strService = rpcMessage.service();
                const std::string &strMethod = rpcMessage.method();
                int64_t id = rpcMessage.id();

                auto it = m_serviceMap.find(strService);
                if (it == m_serviceMap.end())
                {
                    // 没有找到对应的服务
                    sendError(conn, id, lrpc::NO_SERVICE);
                    return;
                }
                const ServiceInfo &serviceInfo = it->second;
                auto mit = serviceInfo.m_methodMap.find(strMethod);
                if (mit == serviceInfo.m_methodMap.end())
                {
                    // 没有找到对应的方法
                    sendError(conn, id, lrpc::NO_METHOD);
                    return;
                }
                const google::protobuf::MethodDescriptor *methodDesc = mit->second;
                LOG_INFO << "Received request for service: " << strService << ", method: " << strMethod << ", message ID: " << id;

                google::protobuf::Service *servicePtr = serviceInfo.m_service;
                auto request = std::shared_ptr<google::protobuf::Message>(servicePtr->GetRequestPrototype(methodDesc).New());
                auto response = std::shared_ptr<google::protobuf::Message>(servicePtr->GetResponsePrototype(methodDesc).New());

                if (rpcMessage.type() != lrpc::REQUEST)
                {
                    sendError(conn, id, lrpc::INVALID_REQUEST);
                    return;
                }

                // 绑定closure回调，调用service的CallMethod时会执行这个回调来发送成功响应
                google::protobuf::Closure *done =
                    new LambdaClosure([this, conn, response, id]()
                                      { this->sendResponse(conn, response, id); });
                // google::protobuf::Closure *done =
                //     google::protobuf::NewCallback
                //     <RpcServer, const leef::net::TcpConnectionPtr &, google::protobuf::Message *, int64_t>
                //     (this, &RpcServer::sendResponse, conn, response.get(), id);

                // 请求参数解析
                const std::string &reqData = rpcMessage.request();

                if (!request->ParseFromArray(reqData.data(), reqData.size()))
                {
                    sendError(conn, id, lrpc::INVALID_REQUEST);
                    return;
                }

                servicePtr->CallMethod(methodDesc, nullptr, request.get(), response.get(), done);
            }
        }

        void RpcServer::run()
        {
            m_eventLoopPtr = std::make_unique<leef::net::EventLoop>();
            m_tcpServerPtr = std::make_unique<leef::net::TcpServer>(m_eventLoopPtr.get(), leef::net::InetAddress(m_port), m_name, leef::net::TcpServer::kReusePort);
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

            LOG_INFO << "Starting RPC Server on port " << m_port << " with " << m_threadNum << " threads.";
            m_tcpServerPtr->start();
            m_eventLoopPtr->loop();
        }

        void RpcServer::sendResponse(
            const leef::net::TcpConnectionPtr &conn,
            const std::shared_ptr<google::protobuf::Message> &response,
            int64_t id)
        {
            lrpc::RpcMessage rpcMsg;
            rpcMsg.set_type(lrpc::RESPONSE);
            rpcMsg.set_id(id);
            rpcMsg.set_error(lrpc::SUCCESS);

            std::string respData;
            if (!response->SerializeToString(&respData))
            {
                LOG_ERROR << "Failed to serialize response";
                rpcMsg.set_error(lrpc::INVALID_RESPONSE);
            }
            else
            {
                rpcMsg.set_response(respData);
            }

            // 3. 序列化 RpcMessage
            int32_t len = static_cast<int32_t>(rpcMsg.ByteSizeLong());
            int32_t netLen = leef::net::sockets::hostToNetwork32(len);

            std::string buffer;
            buffer.resize(sizeof(int32_t) + len);

            memcpy(buffer.data(), &netLen, sizeof(int32_t));

            rpcMsg.SerializeToArray(buffer.data() + sizeof(int32_t), len);

            conn->send(buffer);
        }

        void RpcServer::sendError(const leef::net::TcpConnectionPtr &conn,
                                  int64_t id,
                                  lrpc::ErrorCode errorCode)
        {
            lrpc::RpcMessage errMsg;
            errMsg.set_type(lrpc::RESPONSE);
            errMsg.set_id(id);
            errMsg.set_error(errorCode);

            int32_t len = errMsg.ByteSizeLong();
            int32_t netLen = leef::net::sockets::hostToNetwork32(len);

            std::string buffer;
            buffer.resize(sizeof(int32_t) + len);

            memcpy(buffer.data(), &netLen, sizeof(int32_t));
            errMsg.SerializeToArray(buffer.data() + sizeof(int32_t), len);

            conn->send(buffer);
        }
    }
}