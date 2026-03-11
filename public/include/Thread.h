#ifndef _PUBLIC_THREADPOOL_H
#define _PUBLIC_THREADPOOL_H

#include "CountDownLatch.h"
#include "noncopyable.h"

#include <functional>
#include <memory>
#include <pthread.h>
#include <atomic>


namespace prj
{
    class Thread : noncopyable
    {
    public:
        typedef std::function<void()> ThreadFunc;

        explicit Thread(ThreadFunc, const std::string &name = std::string());
        Thread(Thread &&thread);
        ~Thread();

        void start();
        int join(); // return pthread_join()

        bool started() const { return started_; }
        // pthread_t pthreadId() const { return pthreadId_; }
        pid_t tid() const { return tid_; }
        const std::string &name() const { return name_; }

       static int numCreated() { return numCreated_.load(); }
    private:
        void setDefaultName();

        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
        CountDownLatch latch_;

        static std::atomic<int> numCreated_;
    };

} // namespace prj

#endif // _PUBLIC_THREADPOOL_H