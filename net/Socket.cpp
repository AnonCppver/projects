#include "Socket.h"

#include "../base/Logging.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> // snprintf

using namespace leef;
using namespace leef::net;

Socket::~Socket()
{
  sockets::close(sockfd_);
}

bool Socket::getTcpInfo(struct tcp_info *tcpi) const
{
  socklen_t len = sizeof(*tcpi);
  memset(tcpi, 0, len);
  return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char *buf, int len) const
{
  struct tcp_info tcpi;
  bool ok = getTcpInfo(&tcpi);
  if (ok)
  {
    snprintf(buf, len, "unrecovered=%u "
                       "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                       "lost=%u retrans=%u rtt=%u rttvar=%u "
                       "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits, // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,         // Retransmit timeout in usec
             tcpi.tcpi_ato,         // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,    // Lost packets
             tcpi.tcpi_retrans, // Retransmitted packets out
             tcpi.tcpi_rtt,     // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,  // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans); // Total retransmits for entire connection
  }
  return ok;
}

void Socket::bindAddress(const InetAddress &addr)
{
  sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen()
{
  sockets::listenOrDie(sockfd_);
}
// 接收连接并获取对端地址
int Socket::accept(InetAddress *peeraddr)
{
  struct sockaddr_in6 addr = {0};
  int connfd = sockets::accept(sockfd_, &addr);
  if (connfd >= 0)
  {
    peeraddr->setSockAddrInet6(addr);
  }
  return connfd;
}

void Socket::shutdownWrite()
{
  sockets::shutdownWrite(sockfd_);
}

bool Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                      &optval, static_cast<socklen_t>(sizeof optval)) == 0;
}

bool Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                      &optval, static_cast<socklen_t>(sizeof optval)) == 0;
}

bool Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on)
  {
    LOG_SYSERR << "SO_REUSEPORT failed.";
    return false;
  }
#else
  if (on)
  {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
  return true;
}

bool Socket::setKeepAlive(bool on)
{
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                      &optval, static_cast<socklen_t>(sizeof optval)) == 0;
}
