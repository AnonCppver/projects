#include "LRpcChannel.h"

#include "../base/timeUtil.h"
#include "../net/SocketsOps.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#include <sys/poll.h>

#include <errno.h>

#include <thread>
#include <algorithm>

using namespace leef::net;
namespace leef
{
    namespace rpc
    {
        LRpcChannel::LRpcChannel(const std::string &ip, int port, int timeout, size_t bufferSize)
            : m_addr(ip, port), m_timeout(timeout), m_bufferSize(bufferSize)
        {
            LOG_INFO << "init RpcChannel " << m_addr.toIpPort();
        }
        void LRpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                     google::protobuf::RpcController *controller,
                                     const google::protobuf::Message *request,
                                     google::protobuf::Message *response,
                                     google::protobuf::Closure *done)
        {
            const auto *serviceDescriptor = method->service();

            lrpc::RpcMessage rpcMessage;
            rpcMessage.set_service(serviceDescriptor->name());
            rpcMessage.set_method(method->name());
            rpcMessage.set_type(lrpc::REQUEST);
            // move
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
    } // namespace rpc
}