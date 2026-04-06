#ifndef _LEEF_NET_TCPSERVER_H
#define _LEEF_NET_TCPSERVER_H

#include "../base/noncopyable.h"
#include "TcpConnection.h"

#include <atomic>
#include <map>

namespace leef
{
namespace net
{

class Acceptor;              // 负责监听 socket + accept 新连接
class EventLoop;             // Reactor 核心（事件循环）
class EventLoopThreadPool;   // IO 线程池（多 Reactor）

// TcpServer：对外的 TCP 服务器封装（类似 Muduo TcpServer）
// 职责：
// 1. 管理监听 socket（Acceptor）
// 2. 管理所有 TcpConnection
// 3. 分发连接到 IO 线程（EventLoopThreadPool）
// 4. 提供回调接口（连接、消息、写完成）
class TcpServer : noncopyable   // 禁止拷贝（资源唯一拥有）
{
 public:
  // 线程初始化回调：每个 IO 线程启动时执行
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  // 是否开启 SO_REUSEPORT（多进程/多线程监听同一端口）
  enum Option
  {
    kNoReusePort,
    kReusePort,
  };

  // 构造函数
  // loop: 主 Reactor（accept 所在线程）
  // listenAddr: 监听地址
  // nameArg: 服务器名称（用于日志/标识）
  // option: 是否开启端口复用
  TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option option = kNoReusePort);

  ~TcpServer();

  // 返回 ip:port 字符串
  const std::string& ipPort() const { return ipPort_; }

  // 返回服务器名称
  const std::string& name() const { return name_; }

  // 获取主 EventLoop（accept 所在线程）
  EventLoop* getLoop() const { return loop_; }

  /// 设置 IO 线程数量（必须在 start() 前调用）
  ///
  /// 设计说明：
  /// - 0：单线程 Reactor（所有 IO 在主线程）
  /// - 1：单独 IO 线程
  /// - N：多 Reactor（round-robin 分配连接）
  void setThreadNum(int numThreads);

  // 设置线程初始化回调（在每个 IO 线程启动时执行）
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }

  // 获取线程池（start 后有效）
  std::shared_ptr<EventLoopThreadPool> threadPool()
  { return threadPool_; }

  /// 启动服务器
  ///
  /// 行为：
  /// - 启动线程池
  /// - 开始监听（Acceptor::listen）
  void start();

  /// 设置连接建立/断开回调（非线程安全，需在 start 前设置）
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// 设置消息回调
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  /// 设置写完成回调
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

 private:
  /// 新连接到来（Acceptor 回调）
  ///
  /// 流程：
  /// 1. 选择一个 IO loop（线程池）
  /// 2. 创建 TcpConnection
  /// 3. 设置回调
  /// 4. 注册到 connections_
  /// 5. 启动连接（建立 Channel 事件）
  void newConnection(int sockfd, const InetAddress& peerAddr);

  /// 移除连接
  ///
  /// 可被任意线程调用，会转发到 loop 线程执行
  void removeConnection(const TcpConnectionPtr& conn);

  /// 真正执行移除连接
  ///
  /// 流程：
  /// 1. 从 connections_ 删除
  /// 2. 销毁 TcpConnection（触发关闭）
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  // 所有连接的容器
  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;   // 主 Reactor

  const std::string ipPort_;
  const std::string name_;

  std::unique_ptr<Acceptor> acceptor_;

  // IO 线程池
  std::shared_ptr<EventLoopThreadPool> threadPool_;

  // 各类回调
  ConnectionCallback connectionCallback_;       // 连接建立/关闭
  MessageCallback messageCallback_;             // 收到数据
  WriteCompleteCallback writeCompleteCallback_; // 写完成
  ThreadInitCallback threadInitCallback_;       // 线程初始化

  // 是否已启动
  std::atomic<int32_t> started_;

  // 下一个连接 id
  int nextConnId_;

  // 当前所有连接（只在 loop 线程操作）
  ConnectionMap connections_;
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_TCPSERVER_H