#ifndef _LEEF_NET_SOCKETSOPS_H
#define _LEEF_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace leef
{
    namespace net
    {
        namespace sockets
        {

            // 创建一个非阻塞 socket（失败直接 abort）
            // family: AF_INET / AF_INET6
            // 返回：socket fd（已设置 O_NONBLOCK + CLOEXEC）
            int createNonblockingOrDie(sa_family_t family);

            // 主动发起连接（客户端）
            // sockfd: 已创建的 socket
            // addr: 目标地址
            // 返回：0 成功 / -1 失败（errno）
            int connect(int sockfd, const struct sockaddr *addr);

            // 绑定地址（失败直接 abort）
            // 用于服务器绑定 IP + 端口
            void bindOrDie(int sockfd, const struct sockaddr *addr);

            // 开始监听（失败直接 abort）
            // backlog 使用系统默认（内部可能封装）
            void listenOrDie(int sockfd);
            // 接受新连接（非阻塞）
            // sockfd: 监听 socket
            // addr: 输出参数，返回对端地址（IPv6兼容）
            // 返回：新连接 fd（已设置 non-blocking）
            // 注意：muduo 会处理 EINTR / EAGAIN 等错误
            int accept(int sockfd, struct sockaddr_in6 *addr);
            // 读取数据（封装 read）
            // 返回：读取字节数 / 0（对端关闭）/ -1（错误）
            ssize_t read(int sockfd, void *buf, size_t count);

            // 分散读（readv）
            // 用于一次读取到多个 buffer（muduo Buffer 优化关键）
            // iov: 多个缓冲区
            ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

            // 写数据（封装 write）
            // 返回：写入字节数 / -1（错误）
            ssize_t write(int sockfd, const void *buf, size_t count);
            // 关闭 socket（封装 close）
            // 释放 fd
            void close(int sockfd);

            // 半关闭写端（TCP FIN）
            // 表示“不再发送数据”，但仍可接收
            void shutdownWrite(int sockfd);

            // 将 sockaddr 转换为 "ip:port" 字符串
            // buf: 输出缓冲区
            void toIpPort(char *buf, size_t size,
                          const struct sockaddr *addr);

            // 将 sockaddr 转换为 "ip"
            void toIp(char *buf, size_t size,
                      const struct sockaddr *addr);

            // 从 ip + port 构造 IPv4 地址结构
            void fromIpPort(const char *ip, uint16_t port,
                            struct sockaddr_in *addr);

            // 从 ip + port 构造 IPv6 地址结构
            void fromIpPort(const char *ip, uint16_t port,
                            struct sockaddr_in6 *addr);

            // 获取 socket 的错误状态（SO_ERROR）
            // 常用于非阻塞 connect 结果判断
            int getSocketError(int sockfd);

            // 各种 sockaddr 类型转换（避免强制转换带来的警告）
            // 本质是 reinterpret_cast 的封装

            const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
            const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
            struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);

            const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);
            const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr);

            // 获取本地地址（getsockname）
            // 返回：当前 socket 绑定的 IP + port
            struct sockaddr_in6 getLocalAddr(int sockfd);

            // 获取对端地址（getpeername）
            // 返回：连接对方的 IP + port
            struct sockaddr_in6 getPeerAddr(int sockfd);
            // 判断是否是“自连接”（自己连自己）
            // 条件：localAddr == peerAddr
            bool isSelfConnect(int sockfd);

        } // namespace sockets
    } // namespace net
} // namespace leef

#endif // _LEEF_NET_SOCKETSOPS_H
