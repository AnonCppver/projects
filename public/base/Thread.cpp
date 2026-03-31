#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
#include "timeUtil.h"
//#include "Logging.h"

#include <type_traits>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace leef
{
namespace detail
{
// 内核线程id
pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}
// fork 后重置线程状态
void afterFork()
{
  leef::CurrentThread::t_cachedTid = 0;
  leef::CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    leef::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData
{
  typedef leef::Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  std::string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(ThreadFunc func,
             const std::string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : func_(std::move(func)),
      name_(name),
      tid_(tid),
      latch_(latch)
  { }

  void runInThread()
  {
    *tid_ = leef::CurrentThread::tid();
    tid_ = NULL;
    latch_->countDown();
    latch_ = NULL;

    leef::CurrentThread::t_threadName = name_.empty() ? "leefThread" : name_.c_str();
    ::prctl(PR_SET_NAME, leef::CurrentThread::t_threadName);
    try
    {
      func_();
      leef::CurrentThread::t_threadName = "finished";
    }
    catch (const Exception& ex)
    {
      leef::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      leef::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      leef::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw; // rethrow
    }
  }
};

void* startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

}  // namespace detail
// 初始化线程id
void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread()
{
  return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}

std::atomic<int> Thread::numCreated_;

Thread::Thread(ThreadFunc func, const std::string& n)
  : m_started(false),
    m_joined(false),
    m_pthreadId(0),
    m_tid(0),
    m_tFunc(std::move(func)),
    m_tName(n),
    m_latch(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (m_started && !m_joined)
  {
    pthread_detach(m_pthreadId);
  }
}

void Thread::setDefaultName()
{
  int num = ++numCreated_;
  if (m_tName.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    m_tName = buf;
  }
}

void Thread::start()
{
  assert(!m_started);
  m_started = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(m_tFunc, m_tName, &m_tid, &m_latch);
  if (pthread_create(&m_pthreadId, NULL, &detail::startThread, data))
  {
    m_started = false;
    delete data; // or no delete?
    //LOG_SYSFATAL << "Failed in pthread_create";
  }
  else
  {
    m_latch.wait();
    assert(m_tid > 0);
  }
}

int Thread::join()
{
  assert(m_started);
  assert(!m_joined);
  m_joined = true;
  return pthread_join(m_pthreadId, NULL);
}

}  // namespace leef
