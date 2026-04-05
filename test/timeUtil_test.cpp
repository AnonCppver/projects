#include <iostream>
#include <ctime>
#include <chrono>
#include "../base/timeUtil.h"

using namespace std;

// ===== 自定义函数 =====
inline time_t utcToLocal_fast(time_t utc)
{
    // 一次调用 即使多线程
    static const int difference = []() -> int {
        time_t now = time(nullptr);

        struct tm tm_local{};
        struct tm tm_utc{};

        localtime_r(&now, &tm_local);
        gmtime_r(&now, &tm_utc);

        time_t local_sec = timegm(&tm_local);
        time_t utc_sec   = timegm(&tm_utc);
        cout<<"local_sec: "<<local_sec<<", utc_sec: "<<utc_sec<<endl;
        cout<<"difference: "<<local_sec - utc_sec<<endl;

        return static_cast<int>(local_sec - utc_sec);
    }();

    return utc + difference;
}

// ===== 基准：localtime_r =====
inline time_t utcToLocal_libc(time_t utc)
{
    struct tm tm_local{};
    localtime_r(&utc, &tm_local);
    return mktime(&tm_local);
}

// ===== 打印函数 =====
void printTime(const char* tag, time_t t)
{
    struct tm tm_time{};
    localtime_r(&t, &tm_time);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_time);
    cout << tag << ": " << buf << endl;
}

int main()
{
    const int N = 5'000'000;

    time_t now = time(nullptr);

    // ===== 正确性验证 =====
    time_t t1 = utcToLocal_libc(now);
    time_t t2 = utcToLocal_fast(now);

    cout << "=== Correctness Check ===" << endl;
    printTime("localtime_r", t1);
    printTime("fast       ", t2);
    cout << endl;

    // // ===== Benchmark: localtime =====
    // {
    //     volatile time_t sink = 0;

    //     auto start = chrono::high_resolution_clock::now();

    //     for (int i = 0; i < N; ++i)
    //     {
    //         sink += utcToLocal_libc(now + i);
    //     }

    //     auto end = chrono::high_resolution_clock::now();

    //     cout << "localtime_r: "
    //          << chrono::duration<double>(end - start).count()
    //          << " s" << endl;
    // }

    // // ===== Benchmark: fast =====
    // {
    //     volatile time_t sink = 0;

    //     auto start = chrono::high_resolution_clock::now();

    //     for (int i = 0; i < N; ++i)
    //     {
    //         sink += utcToLocal_fast(now + i);
    //     }

    //     auto end = chrono::high_resolution_clock::now();

    //     cout << "utcToLocal_fast: "
    //          << chrono::duration<double>(end - start).count()
    //          << " s" << endl;
    // }

    time_t t3 = time(nullptr);
    time_t t4 = utcToLocal_fast(time(nullptr));
    tm tm3{};
    gmtime_r(&t3, &tm3);
    tm tm4{};
    gmtime_r(&t4, &tm4);
    cout<<"t3 :"<<tm3.tm_year + 1900<<"-"<<tm3.tm_mon + 1<<"-"<<tm3.tm_mday<<" "<<tm3.tm_hour<<":"<<tm3.tm_min<<":"<<tm3.tm_sec<<endl;
    cout<<"t4 :"<<tm4.tm_year + 1900<<"-"<<tm4.tm_mon + 1<<"-"<<tm4.tm_mday<<" "<<tm4.tm_hour<<":"<<tm4.tm_min<<":"<<tm4.tm_sec<<endl;

    cout<<"time2str(t3): "<<leef::time2str(t3)<<endl;
    cout<<"time2str(t4): "<<leef::time2str(t4)<<endl;

    return 0;
}