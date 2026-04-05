#ifndef _LEEF_BLOCKINGQUEUE_H
#define _LEEF_BLOCKINGQUEUE_H

#include <condition_variable>
#include <mutex>

#include <deque>
#include <assert.h>

#include "noncopyable.h"

namespace leef
{

template<typename T>
class BlockingQueue : public noncopyable
{
 public:
  using queue_type = std::deque<T>;

  BlockingQueue()
    : m_queue()
  {
  }

  void put(const T& x)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(x);
    m_notEmpty.notify_one(); // wait morphing saves us
    // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
  }

  void put(T&& x)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push_back(std::move(x));
    m_notEmpty.notify_one();
  }

  T take()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    // always use a while-loop, due to spurious wakeup
    while (m_queue.empty())
    {
      m_notEmpty.wait(lock);
    }
    assert(!m_queue.empty());
    T front(std::move(m_queue.front()));
    m_queue.pop_front();
    return front;
  }

  queue_type drain()
  {
    std::deque<T> queue;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      queue = std::move(m_queue);
      assert(m_queue.empty());
    }
    return queue;
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
  }

 private:
  mutable std::mutex m_mutex;
  std::condition_variable m_notEmpty;
  queue_type        m_queue;
};  // __attribute__ ((aligned (64)));

}  // namespace leef
#endif  // _LEEF_BLOCKINGQUEUE_H
