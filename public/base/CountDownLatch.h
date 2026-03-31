#ifndef _LEEF_BASE_COUNTDOWNLATCH_H
#define _LEEF_BASE_COUNTDOWNLATCH_H

#include "noncopyable.h"

#include<condition_variable>
#include<mutex>

namespace leef
{

class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
  int m_count ;
};

}  // namespace leef
#endif  // _LEEF_BASE_COUNTDOWNLATCH_H
