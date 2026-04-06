#ifndef _LEEF_NET_EVENTLOOPTHREAD_H
#define _LEEF_NET_EVENTLOOPTHREAD_H

#include "../base/Thread.h"
#include "../base/noncopyable.h"

#include <mutex>
#include <condition_variable>

namespace leef
{
namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_EVENTLOOPTHREAD_H

