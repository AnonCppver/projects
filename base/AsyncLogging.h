#ifndef _LEEF_BASE_ASYNCLOGGING_H
#define _LEEF_BASE_ASYNCLOGGING_H

//#include "BlockingQueue.h"
//#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "Thread.h"
#include "LogStream.h"

#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace leef
{

class AsyncLogging : noncopyable
{
 public:

  AsyncLogging(const std::string& basename,
               off_t rollSize,
               int flushInterval = 3);

  ~AsyncLogging()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cv_.notify_all();
    thread_.join();
  }

 private:

  void threadFunc();

  typedef leef::detail::FixedBuffer<leef::detail::kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flushInterval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t rollSize_;
  leef::Thread thread_;
  leef::CountDownLatch latch_;
  std::mutex mutex_;
  std::condition_variable cv_;
  BufferPtr currentBuffer_ ;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

}  // namespace leef

#endif  // _LEEF_BASE_ASYNCLOGGING_H
