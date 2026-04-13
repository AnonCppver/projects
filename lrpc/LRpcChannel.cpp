#include "LRpcChannel.h"

#include "../base/timeUtil.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#include <sys/epoll.h>

#include <errno.h>

#include <thread>
#include <algorithm>

namespace leef
{
    namespace rpc
    {
        bool TcpStream::connectToServer(const std::string &ip, int port)
        {
            return doConnect(ip, port);
        }

        bool TcpStream::reconnect(const std::string &ip, int port)
        {
            close();

            int delay = 1000 * (1 << std::min(m_retryCount, 3));
            // 1s, 2s, 4s ... max 8s

            std::this_thread::sleep_for(std::chrono::milliseconds(delay));

            if (doConnect(ip, port))
            {
                m_retryCount = 0;
                return true;
            }

            m_retryCount++;
            LOG_WARN << "reconnect failed retry: " << m_retryCount << ",delay: " << delay << " ms.";
            return false;
        }

        int TcpStream::getRetryCount()
        {
            return m_retryCount;
        }

        void TcpStream::close()
        {
            if (m_fd >= 0)
                ::close(m_fd);

            m_fd = -1;
        }

        bool TcpStream::isConnected() const
        {
            if (m_fd == -1)
                return false;
            char buf;
            int ret = recv(m_fd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);

            if (ret > 0)
                return true;
            if (ret == 0)
                return false;

            return errno == EAGAIN || errno == EWOULDBLOCK;
        }

        void TcpStream::setTimeout(int timeout)
        {
            m_timeout = timeout;
        }

        bool TcpStream::doConnect(const std::string &ip, int port)
        {
            if (m_fd != -1)
            {
                ::close(m_fd);
                m_fd = -1;
            }

            m_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (m_fd < 0)
                return false;

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);

            if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
            {
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            // 设置非阻塞
            int flags = fcntl(m_fd, F_GETFL, 0);
            if (flags < 0 || fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) < 0)
            {
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            int ret = connect(m_fd, (sockaddr *)&addr, sizeof(addr));

            if (ret < 0 && errno != EINPROGRESS)
            {
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            // 立即成功
            if (ret == 0)
            {
                // 设置 TCP_NODELAY
                int flag = 1;
                setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
                return true;
            }

            int epfd = epoll_create1(0);
            if (epfd < 0)
            {
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            epoll_event ev{};
            ev.events = EPOLLOUT | EPOLLERR; // 写事件 + 错误
            ev.data.fd = m_fd;

            if (epoll_ctl(epfd, EPOLL_CTL_ADD, m_fd, &ev) < 0)
            {
                ::close(epfd);
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            // 控制超时
            leef::Timer timer;

            while (true)
            {
                long long elapsed_ms = timer.elapsedMS();

                int timeout_ms = m_timeout * 1000 - static_cast<int>(elapsed_ms);

                timeout_ms=std::max(timeout_ms,0);

                epoll_event events[1];
                int n = epoll_wait(epfd, events, 1, timeout_ms);

                if (n > 0)
                    break;

                if (n == 0)
                {
                    // 超时
                    ::close(epfd);
                    ::close(m_fd);
                    m_fd = -1;
                    return false;
                }

                if (errno == EINTR)
                    continue;

                // epoll_wait 出错
                ::close(epfd);
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            // 检查 connect 是否成功
            int err = 0;
            socklen_t len = sizeof(err);

            if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0)
            {
                ::close(epfd);
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            ::close(epfd);

            // 设置 TCP_NODELAY
            int flag = 1;
            setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

            return true;
        }

        bool TcpStream::send(const std::string& message)
        {

        }

        bool TcpStream::read(char * buffer)
        {
            
        }
    } // namespace rpc
}