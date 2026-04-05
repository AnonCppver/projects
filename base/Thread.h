#ifndef _LEEF_BASE_THREAD_H
#define _LEEF_BASE_THREAD_H

#include "CountDownLatch.h"

#include <functional>
#include <memory>
#include <pthread.h>
#include <atomic>

namespace leef
{

class Thread : noncopyable
{
 public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(ThreadFunc, const std::string& name = "");
  Thread(Thread&& other) noexcept;
  ~Thread();

  void start();
  int join(); // return pthread_join()

  bool started() const { return m_started; }
  // pthread_t pthreadId() const { return m_pthreadId; }
  pid_t tid() const { return m_tid; }
  const std::string& name() const { return m_tName; }

  static int numCreated() { return numCreated_.load(); }

 private:
  void setDefaultName();

  bool       m_started;
  bool       m_joined;
  pthread_t  m_pthreadId; // pthread库使用的线程句柄（控制线程）
  pid_t      m_tid;       // 内核线程ID（标识线程）
  ThreadFunc m_tFunc;
  std::string     m_tName;
  CountDownLatch m_latch; // 用于线程启动时同步，保证线程id已获取

  static std::atomic<int> numCreated_;
};

}  // namespace leef
#endif  // _LEEF_BASE_THREAD_H
