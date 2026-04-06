#ifndef _LEEF_NET_SOCKET_H
#define _LEEF_NET_SOCKET_H

#include "../base/noncopyable.h"


// Acceptor 创建监听 fd → 通过 bind 绑定 InetAddress → listen 开始监听 → 有连接到达时调用 accept 创建连接 fd

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace leef
{

namespace net
{

class InetAddress;

class Socket : noncopyable
{
 public:
  explicit Socket(int sockfd)
    : sockfd_(sockfd)
  { }

  ~Socket();

  int fd() const { return sockfd_; }
  // return true if success.
  bool getTcpInfo(struct tcp_info*) const;
  bool getTcpInfoString(char* buf, int len) const;

  /// abort if address in use
  void bindAddress(const InetAddress& localaddr);
  /// abort if address in use
  void listen();

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress* peeraddr);

  void shutdownWrite();

  ///
  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  ///
  bool setTcpNoDelay(bool on);

  ///
  /// Enable/disable SO_REUSEADDR
  ///
  bool setReuseAddr(bool on);

  ///
  /// Enable/disable SO_REUSEPORT
  ///
  bool setReusePort(bool on);

  ///
  /// Enable/disable SO_KEEPALIVE
  ///
  bool setKeepAlive(bool on);

 private:
  const int sockfd_;
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_SOCKET_H
