#ifndef _LEEF_BOUNDEDBLOCKINGQUEUE_H
#define _LEEF_BOUNDEDBLOCKINGQUEUE_H

#include <condition_variable>
#include <mutex>

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace leef
{

template<typename T>
class BoundedBlockingQueue : noncopyable
{
 public:
  explicit BoundedBlockingQueue(int maxSize)
    :m_queue(maxSize)
  {
  }

  void put(const T& x)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.full())
    {
      m_notFull.wait(lock);
    }
    assert(!m_queue.full());
    m_queue.push_back(x);
    m_notEmpty.notify_one();
  }

  void put(T&& x)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.full())
    {
      m_notFull.wait(lock);
    }
    assert(!m_queue.full());
    m_queue.push_back(std::move(x));
    m_notEmpty.notify_one();
  }

  T take()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty())
    {
      m_notEmpty.wait(lock);
    }
    assert(!m_queue.empty());
    T front(std::move(m_queue.front()));
    m_queue.pop_front();
    m_notFull.notify_one();
    return front;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  bool full() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.full();
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
  }

  size_t capacity() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.capacity();
  }

 private:
  mutable std::mutex          m_mutex;
  std::condition_variable     m_notEmpty;
  std::condition_variable     m_notFull;
  boost::circular_buffer<T>  m_queue;
};

}  // namespace leef

#endif  // _LEEF_BOUNDEDBLOCKINGQUEUE_H
