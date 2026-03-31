#include "../base/ThreadPool.h"
#include "../base/CountDownLatch.h"
#include "../base/CurrentThread.h"

#include <stdio.h>
#include <unistd.h> // usleep

void print()
{
    printf("tname=%s\n", leef::CurrentThread::name());
}

void printString(const std::string &str)
{
    usleep(100 * 1000);
    printf("tname=%s, %s\n", leef::CurrentThread::name(), str.c_str());
}

void test(int maxSize)
{
    printf("main thread name=%s\n", leef::CurrentThread::name());
    printf("Test ThreadPool with max queue size = %d\n", maxSize);
    leef::ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);
    pool.start(5);

    printf("Adding tasks...\n");
    pool.run(print);
    pool.run(print);

    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(std::bind(printString, std::string(buf)));
    }
    printf("Done\n");

    // 截至任务 只有这个latch停止前接取的task会被处理
    leef::CountDownLatch latch(1);
    pool.run(std::bind(&leef::CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
}

// void testMove()
// {
//   leef::ThreadPool pool;
//   pool.start(2);

//   std::unique_ptr<int> x(new int(42));
//   pool.run([y = std::move(x)]{ printf("%d: %d\n", leef::CurrentThread::tid(), *y); });
//   pool.stop();
// }


void longTask(int num)
{
    printf("tid=%d, longTask %d\n", leef::CurrentThread::tid(), num);
    leef::CurrentThread::sleepUsec(3000000);
}

void test2()
{
    printf("Test ThreadPool by stoping early.\n");
    leef::ThreadPool pool("ThreadPool");
    pool.setMaxQueueSize(5);
    pool.start(3);

    leef::Thread thread1([&pool]()
                         {
    for (int i = 0; i < 20; ++i)
    {
      pool.run(std::bind(longTask, i));
    } }, "thread1");
    thread1.start();

    leef::CurrentThread::sleepUsec(5000000);
    printf("stop\n");
    pool.stop(); // early stop

    thread1.join();
    // run() after stop()
    pool.run(print);
    printf("test2 done\n");
}

int main()
{
    test(0);
    //test(1);
    //test(5);
    //test(10);
    //test(50);
    //test2();
}
