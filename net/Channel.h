#ifndef _LEEF_NET_CHANNEL_H
#define _LEEF_NET_CHANNEL_H

#include "../base/noncopyable.h"
#include "../base/timeUtil.h"

#include <functional>
#include <memory>
#include <string>

namespace leef
{
namespace net
{

class EventLoop;

///
/// Channel 表示一个可监听 I/O 的文件描述符通道
/// - 不拥有 fd，本身只是封装事件和回调
/// - fd 可以是 socket、eventfd、timerfd、signalfd 等
/// - 主要用于 EventLoop + Poller 的 I/O 事件处理
///
class Channel : noncopyable
{
 public:
  // 回调类型
  typedef std::function<void()> EventCallback;               // 无参数事件回调
  typedef std::function<void(Timestamp)> ReadEventCallback;  // 可带时间戳的读事件回调

  Channel(EventLoop* loop, int fd);  // 构造函数，绑定 loop 和 fd
  ~Channel();                         // 析构函数

  // 事件发生时的统一入口
  void handleEvent(Timestamp receiveTime);

  // 回调设置接口
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  // 将 Channel 与拥有者对象绑定，防止在 handleEvent 时拥有者被析构
  void tie(const std::shared_ptr<void>&);

  // 文件描述符访问
  int fd() const { return fd_; }

  // 当前关注的事件
  int events() const { return events_; }

  // Poller 设置的实际发生事件
  void set_revents(int revt) { revents_ = revt; } // Poller 填充

  bool isNoneEvent() const { return events_ == kNoneEvent; }

  // 启用/禁用读写事件
  void enableReading() { events_ |= kReadEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }

  // 判断是否正在监听写/读事件
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const { return events_ & kReadEvent; }

  // Poller 内部使用，用于索引管理
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  // 调试输出事件字符串
  std::string reventsToString() const;
  std::string eventsToString() const;

  // HUP 事件是否记录日志
  void doNotLogHup() { logHup_ = false; }

  // 获取所属 EventLoop
  EventLoop* ownerLoop() { return loop_; }

  // 从 loop 中移除
  void remove();

 private:
  static std::string eventsToString(int fd, int ev); // 调试用

  // 更新事件到 Poller（内部调用）
  void update();

  // 带安全检查的事件处理
  void handleEventWithGuard(Timestamp receiveTime);

  // 常量
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;        // 所属 EventLoop
  const int  fd_;          // 文件描述符
  int        events_;      // 关注的事件
  int        revents_;     // Poller 返回的实际事件
  int        index_;       // Poller 内部使用
  bool       logHup_;      // 是否记录 HUP 日志

  std::weak_ptr<void> tie_;  // 弱引用，防止 handleEvent 时拥有者被析构，比如TcpConnection
  bool tied_;                // 是否绑定了拥有者对象
  bool eventHandling_;       // 是否正在处理事件
  bool addedToLoop_;         // 是否已经加入 EventLoop
  // 每个channel监听特定fd，根据revents_的值，调用不同的回调函数
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_CHANNEL_H