#include "Timer.h"

using namespace leef::net;

#include <atomic>

std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = Timestamp::invalid();
  }
}
