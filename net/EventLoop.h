#ifndef _LEEF_NET_EVENTLOOP_H
#define _LEEF_NET_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>
#include <mutex> 
#include <boost/any.hpp>

#include "../base/CurrentThread.h"
#include "../base/timeUtil.h"
#include "../base/noncopyable.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace leef
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

// 核心事件循环类
// 管理 I/O 多路复用、定时器事件以及跨线程任务调度
class EventLoop : noncopyable
{
 public:
  // 回调函数类型，用于 runInLoop / queueInLoop
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();  // 析构函数强制 out-of-line，保证 std::unique_ptr 成员正确析构

  // 核心循环函数
  // 调用该函数后进入循环，监听 I/O 事件、执行定时器和待执行任务
  void loop();

  // 退出 loop 循环
  // 调用后 loop 会结束，但可能还有待处理的事件
  void quit();

  // 最近一次 poll 返回的时间戳
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  // 当前 loop 循环迭代次数
  int64_t iteration() const { return iteration_; }

  // 线程安全：在 loop 所在线程中立即执行回调
  // 若调用线程就是 loop 所在线程，则直接执行
  // 若调用线程是其他线程，则将任务加入队列并唤醒 loop
  void runInLoop(Functor cb);

  // 线程安全：将回调加入队列，待 loop 下一次循环处理
  void queueInLoop(Functor cb);

  // 获取待处理队列大小
  size_t queueSize() const;

  // --- 定时器相关接口 ---
  
  // 在指定时间执行回调
  TimerId runAt(Timestamp time, TimerCallback cb);

  // 延迟 delay 秒执行回调
  TimerId runAfter(double delay, TimerCallback cb);

  // 每 interval 秒重复执行回调
  TimerId runEvery(double interval, TimerCallback cb);

  // 取消定时器
  void cancel(TimerId timerId);

  // --- 内部使用接口 ---

  // epoll_wait立刻返回（主要用于其他线程调用 runInLoop / queueInLoop 时）
  void wakeup();

  // 更新某个 channel 在 poller 中的监听事件
  void updateChannel(Channel* channel);

  // 从 poller 中移除 channel
  void removeChannel(Channel* channel);

  // 判断 channel 是否在 poller 中
  bool hasChannel(Channel* channel);

  // --- 线程相关检查 ---

  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }

  // 判断调用线程是否是 loop 所在线程
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

  // 当前是否正在处理事件
  bool eventHandling() const { return eventHandling_; }

  // 上下文管理，允许用户绑定任意对象到 EventLoop 上
  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  // 获取当前线程的 EventLoop
  static EventLoop* getEventLoopOfCurrentThread();

 private:
  // 当不在 loop 所在线程调用 loop 相关操作时 abort
  void abortNotInLoopThread();

  // 处理 wakeupFd 可读事件
  void handleRead();  // loop 被唤醒时触发

  // 执行 pendingFunctors_ 队列中的任务
  void doPendingFunctors();

  // 打印活跃 channel，调试用
  void printActiveChannels() const;

  typedef std::vector<Channel*> ChannelList;

  std::atomic<bool> looping_; // 是否正在 loop 中
  std::atomic<bool> quit_; // 是否请求退出 loop
  bool eventHandling_; // 是否正在处理 I/O 事件
  bool callingPendingFunctors_; // 是否正在执行待处理任务队列
  int64_t iteration_; // loop 循环迭代计数
  const pid_t threadId_; // loop 所在线程 ID
  Timestamp pollReturnTime_; // poll 返回时间
  std::unique_ptr<Poller> poller_; // I/O 多路复用器
  std::unique_ptr<TimerQueue> timerQueue_; // 定时器管理器
  int wakeupFd_; // mainLoop通过轮询将新连接派发给subLoop
  std::unique_ptr<Channel> wakeupChannel_; // wakeupFd 对应的 channel
  boost::any context_; // 用户自定义上下文

  ChannelList activeChannels_; // poller 返回的活跃 channel 列表
  Channel* currentActiveChannel_; // 当前正在处理的 channel

  mutable std::mutex mutex_; // 保护 pendingFunctors_
  std::vector<Functor> pendingFunctors_; // 待执行回调队列
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_EVENTLOOP_H