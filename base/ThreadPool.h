#ifndef _LEEF_BASE_THREADPOOL_H
#define _LEEF_BASE_THREADPOOL_H

#include "Thread.h"

#include <condition_variable>
#include <mutex>

#include <deque>
#include <vector>

namespace leef
{

class ThreadPool : noncopyable
{
 public:
  typedef std::function<void ()> Task;

  explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { m_maxQueueSize = maxSize; }
  void setThreadInitCallback(const Task& cb)
  { m_threadInitCallback = cb; }

  void start(int numThreads);
  void stop();

  const std::string& name() const
  { return m_name; }

  size_t queueSize() const;

  // Could block if maxQueueSize > 0
  // Call after stop() will return immediately.
  // There is no move-only version of std::function in C++ as of C++14.
  // So we don't need to overload a const& and an && versions
  // as we do in (Bounded)BlockingQueue.
  // https://stackoverflow.com/a/25408989
  void run(Task f);

 private:
  bool isFull() const;
  void runInThread();
  Task take();

  mutable std::mutex m_mutex;
  std::condition_variable m_notEmpty;
  std::condition_variable m_notFull;
  std::string m_name;
  Task m_threadInitCallback;
  std::vector<std::unique_ptr<leef::Thread>> m_threads;
  std::deque<Task> m_queue;
  size_t m_maxQueueSize;
  bool m_running;
};

}  // namespace leef

#endif  // _LEEF_BASE_THREADPOOL_H
