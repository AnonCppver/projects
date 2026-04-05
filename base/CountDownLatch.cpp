#include "CountDownLatch.h"

using namespace leef;

CountDownLatch::CountDownLatch(int count)
  : m_count(count)
{
}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_cv.wait(lock, [this] {
        return m_count == 0;
    });
}

void CountDownLatch::countDown()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (--m_count == 0)
    {
        m_cv.notify_all();
    }
}

int CountDownLatch::getCount() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_count;
}

