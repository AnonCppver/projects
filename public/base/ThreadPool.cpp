#include "ThreadPool.h"

#include "Exception.h"

#include <assert.h>
#include <stdio.h>

using namespace leef;

ThreadPool::ThreadPool(const std::string& nameArg)
  : m_name(nameArg),
    m_maxQueueSize(0),
    m_running(false)
{
}

ThreadPool::~ThreadPool()
{
  if (m_running)
  {
    stop();
  }
}

void ThreadPool::start(int numThreads)
{
  assert(m_threads.empty());
  m_running = true;
  m_threads.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i+1);
    m_threads.emplace_back(new leef::Thread(
          std::bind(&ThreadPool::runInThread, this), m_name+id));
    m_threads[i]->start();
  }
  if (numThreads == 0 && m_threadInitCallback)
  {
    m_threadInitCallback();
  }
}

void ThreadPool::stop()
{
  {
  std::lock_guard<std::mutex>lock(m_mutex);
  m_running = false;
  m_notEmpty.notify_all();
  m_notFull.notify_all();
  }
  for (auto& thr : m_threads)
  {
    thr->join();
  }
}

size_t ThreadPool::queueSize() const
{
  std::lock_guard<std::mutex>lock(m_mutex);
  return m_queue.size();
}

void ThreadPool::run(Task task)
{
  if (m_threads.empty())
  {
    task();
  }
  else
  {
    std::unique_lock<std::mutex>lock(m_mutex);
    while (isFull() && m_running)
    {
      m_notFull.wait(lock);
    }
    if (!m_running) return;
    assert(!isFull());

    m_queue.push_back(std::move(task));
    m_notEmpty.notify_one();
  }
}

ThreadPool::Task ThreadPool::take()
{
  std::unique_lock<std::mutex>lock(m_mutex);
  // always use a while-loop, due to spurious wakeup
  while (m_queue.empty() && m_running)
  {
    m_notEmpty.wait(lock);
  }
  Task task;
  if (!m_queue.empty())
  {
    task = m_queue.front();
    m_queue.pop_front();
    if (m_maxQueueSize > 0)
    {
      m_notFull.notify_one();
    }
  }
  return task;
}

bool ThreadPool::isFull() const
{
  return m_maxQueueSize > 0 && m_queue.size() >= m_maxQueueSize;
}

void ThreadPool::runInThread()
{
  try
  {
    if (m_threadInitCallback)
    {
      m_threadInitCallback();
    }
    while (m_running)
    {
      Task task(take());
      if (task)
      {
        task();
      }
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", m_name.c_str());
    throw; // rethrow
  }
}

