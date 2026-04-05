#ifndef _LEEF_NET_POLLER_H
#define _LEEF_NET_POLLER_H

#include <map>
#include <vector>

#include "../base/timeUtil.h"
#include "EventLoop.h"

namespace leef
{
namespace net
{

class Channel;

///
/// Poller 是 I/O 多路复用器的抽象基类
/// 负责管理多个 Channel（每个 Channel 对应一个文件描述符）的 I/O 事件
/// 注意：
/// 不拥有 Channel 对象，只保存指针
/// 所有操作必须在 EventLoop 所在线程执行
///
class Poller : noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList;  // 活跃 channel 列表

  // 构造函数
  // loop: Poller 所属的 EventLoop
  Poller(EventLoop* loop);

  // 析构函数
  virtual ~Poller();

  // 核心接口：轮询 I/O 事件
  // timeoutMs: poll 等待超时时间（毫秒）
  // activeChannels: 输出参数，返回发生事件的 Channel
  // 返回值：轮询返回的时间戳
  // 调用要求：必须在 loop 所在线程
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

  // 更新某个 channel 的关注事件（可读/可写等）
  // 调用要求：必须在 loop 所在线程
  virtual void updateChannel(Channel* channel) = 0;

  // 从 poller 中移除 channel（channel 析构时调用）
  // 调用要求：必须在 loop 所在线程
  virtual void removeChannel(Channel* channel) = 0;

  // 判断 channel 是否在当前 Poller 管理中
  virtual bool hasChannel(Channel* channel) const;

  // 创建默认 Poller
  // 不同平台可以返回不同实现，例如 epoll/kqueue/select
  static Poller* newDefaultPoller(EventLoop* loop);

  // 断言当前线程是 loop 所在线程
  void assertInLoopThread() const
  {
    ownerLoop_->assertInLoopThread();
  }

 protected:
  typedef std::map<int, Channel*> ChannelMap;  // fd -> Channel 映射
  ChannelMap channels_;  // 管理所有注册的 channel

 private:
  EventLoop* ownerLoop_;  // Poller 所属 EventLoop
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_POLLER_H