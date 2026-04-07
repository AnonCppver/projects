#ifndef _LEEF_NET_TCPCONNECTION_H
#define _LEEF_NET_TCPCONNECTION_H

#include "../base/noncopyable.h"
#include "../base/StringPiece.h"

#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"

#include <memory>
#include <boost/any.hpp>

struct tcp_info;

namespace leef
{
namespace net
{

class Channel;
class EventLoop;
class Socket;

/*
 * TcpConnection
 * 1. 管理 socket 生命周期
 * 2. 绑定 EventLoop（Reactor模型）
 * 3. 处理读写事件（Channel回调）
 * 4. 缓冲输入/输出数据（Buffer）
 * 5. 触发用户回调（Connection/Message/WriteComplete）
 *
 * - enable_shared_from_this：保证异步回调安全引用自身
 * - 一个连接 = 一个 Channel + 一个 Socket + 两个 Buffer
 */
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
 public:
  /// 构造函数：仅由 TcpServer 创建
  /// 表示一个已经完成 accept 的连接
  TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);

  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }

  const InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }

  /// 连接状态判断
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected; }

  /// 获取 TCP 内核状态（TCP_INFO）
  bool getTcpInfo(struct tcp_info*) const;
  std::string getTcpInfoString() const;

  /// 发送数据（裸指针版本）
  void send(const void* message, int len);

  /// StringPiece 版本（避免拷贝）
  void send(const StringPiece& message);

  /// Buffer 版本（会 swap 或直接复用内部 buffer）
  void send(Buffer* message);

  /*
   * 关闭连接（优雅关闭）
   * NOT thread safe：必须在所属 loop 线程调用
   */
  void shutdown();

  /// 强制关闭连接（通常用于错误/超时）
  void forceClose();

  /// 延迟强制关闭（比如超时踢掉连接）
  void forceCloseWithDelay(double seconds);

  /// 设置 TCP_NODELAY（关闭 Nagle）
  void setTcpNoDelay(bool on);

  // ===== 读控制（半关闭/流控） =====

  void startRead();
  void stopRead();

  /// 当前是否允许读（注意：非线程安全）
  bool isReading() const { return reading_; };

  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  /// 连接建立/断开事件
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// 收到消息事件
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  /// 写完成事件（output buffer flush 完）
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

  /// 高水位回调（输出缓冲过大时触发流控）
  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t highWaterMark)
  {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  Buffer* inputBuffer()  { return &inputBuffer_; }
  Buffer* outputBuffer() { return &outputBuffer_; }

  /// ===== 内部使用：关闭回调 =====
  void setCloseCallback(const CloseCallback& cb)
  { closeCallback_ = cb; }

  /*
   * ===== 生命周期关键函数 =====
   *
   * connectEstablished():
   *   - accept之后调用
   *   - 状态切换 CONNECTING -> CONNECTED
   *   - 注册 EPOLLIN
   *   - 触发 connectionCallback_
   *
   * connectDestroyed():
   *   - TcpServer 移除 connection 时调用
   *   - 清理 Channel / Socket
   *   - 状态切换到 DISCONNECTED
   */
  void connectEstablished();   // 只调用一次
  void connectDestroyed();     // 只调用一次

 private:
  enum StateE {
    kDisconnected,   // 已断开
    kConnecting,     // 正在连接（通常 accept 后）
    kConnected,      // 已连接（正常状态）
    kDisconnecting   // 正在关闭（半关闭阶段）
  };

  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);

  /// shutdown 的 loop 内实现
  void shutdownInLoop();

  /// 强制关闭 loop 内版本
  void forceCloseInLoop();

  void setState(StateE s) { state_ = s; }

  const char* stateToString() const;

  /// 读控制 loop 内实现
  void startReadInLoop();
  void stopReadInLoop();

 private:
  EventLoop* loop_;          // 所属 IO 线程（one loop per thread）
  const std::string name_;   // 连接名称（debug/日志用）

  StateE state_;             // 连接状态（非 atomic，依赖 loop 线程）

  bool reading_;             // 是否监听 EPOLLIN

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;

  const InetAddress localAddr_;
  const InetAddress peerAddr_;

  // 用户自定义回调 被嵌入channel回调中调用
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;

  size_t highWaterMark_;     // 输出缓冲高水位线

  Buffer inputBuffer_;       // 已读未处理数据
  Buffer outputBuffer_;      // 待发送数据（write未完成）

  // ===== 用户扩展数据 =====
  boost::any context_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_TCPCONNECTION_H