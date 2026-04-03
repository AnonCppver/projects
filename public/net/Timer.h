#ifndef _LEEF_NET_TIMER_H
#define _LEEF_NET_TIMER_H

#include "../base/timeUtil.h"
#include "../base/noncopyable.h"
#include "Callbacks.h"

#include<atomic>

namespace leef
{
namespace net
{

///
/// Internal class for timer event.
///
class Timer : noncopyable
{
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(++s_numCreated_)
  { }

  void run() const
  {
    callback_();
  }

  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  void restart(Timestamp now);

  static int64_t numCreated() { return s_numCreated_.load(); }

 private:
  const TimerCallback callback_;
  Timestamp expiration_;          // 触发点
  const double interval_;     // 间隔时间 
  const bool repeat_;         
  const int64_t sequence_;    // 生成序列号，唯一标识一个定时器

  static std::atomic<int64_t> s_numCreated_;
};

}  // namespace net
}  // namespace leef

#endif  // _LEEF_NET_TIMER_H
