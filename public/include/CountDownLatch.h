// 线程同步工具

#ifndef _PUBLIC_COUNTDOWNLATCH_H
#define _PUBLIC_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

namespace prj
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
        int m_count;
    };

} // namespace prj
#endif // _PUBLIC_COUNTDOWNLATCH_H
