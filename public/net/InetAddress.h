#ifndef _LEEF_NET_INETADDRESS_H
#define _LEEF_NET_INETADDRESS_H

#include "../base/copyable.h"
#include "../base/StringPiece.h"

#include <netinet/in.h>

namespace leef
{
namespace net
{
namespace sockets
{
// 将 sockaddr_in6 转换为通用 sockaddr*（用于系统调用）
// 本质是类型转换封装，避免到处写 reinterpret_cast
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
}

class InetAddress : public leef::copyable
{
 public:
  // loopbackOnly = true,绑定到回环地址，仅本地访问
  // loopbackOnly = false,绑定到INADDR_ANY，开放
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

  // 指定监听ip
  /// @c ip should be "1.2.3.4"
  InetAddress(StringPiece ip, uint16_t port, bool ipv6 = false);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  explicit InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
  { }

  explicit InetAddress(const struct sockaddr_in6& addr)
    : addr6_(addr)
  { }

  sa_family_t family() const { return addr_.sin_family; }
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t port() const;

  const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }
  void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

  uint32_t ipv4NetEndian() const;
  uint16_t portNetEndian() const { return addr_.sin_port; }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(StringPiece hostname, InetAddress* result);
  // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

  // set IPv6 ScopeID
  void setScopeId(uint32_t scope_id);

 private:
  union
  {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_INETADDRESS_H
