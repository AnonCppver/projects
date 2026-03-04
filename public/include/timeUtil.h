#ifndef _TIMEUTIL_H
#define _TIMEUTIL_H

#include <chrono>
#include <ctime>
#include <cstdint>
#include <string>

namespace prj
{

    class Timer
    {
    public:
        using clock = std::chrono::steady_clock;
        using time_point = clock::time_point;

        // 构造时记录时间
        Timer()
            : m_start(clock::now())
        {
        }

        // 返回当前时间
        static time_point now()
        {
            return clock::now();
        }

        double elapsedS() const
        {
            return std::chrono::duration<double>(clock::now() - m_start).count();
        }

        long long elapsedMS() const
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       clock::now() - m_start)
                .count();
        }

        // 更新存储时间戳
        void reset()
        {
            m_start = clock::now();
        }

    private:
        time_point m_start;
    };

    class Timestamp
    {
    public:
        static const int kMicroSecondsPerSecond = 1000 * 1000;
        
        // 构造无效时间
        Timestamp();

        // 构造指定微秒时间
        explicit Timestamp(int64_t microSecondsSinceEpoch);

        // 获取当前时间（Unix epoch，微秒）
        static Timestamp now();

        // 从 time_t 构造
        static Timestamp fromUnixTime(std::time_t t);

        static Timestamp fromUnixTime(std::time_t t, int microseconds);

        // 是否有效
        bool valid() const;

        // 返回微秒数
        int64_t microSecondsSinceEpoch() const;

        // 返回秒数
        std::time_t secondsSinceEpoch() const;

        // 返回格式化字符串
        std::string toString() const;

        std::string toFormattedString(bool showMicroseconds = true) const;

        // 交换
        void swap(Timestamp &other);

        // 计算时间差（秒）
        static double timeDifference(const Timestamp &high,
                                     const Timestamp &low);

        // 增加时间（秒）
        static Timestamp addTime(const Timestamp &timestamp,
                                 double seconds);

    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator<=(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() <= rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

    inline bool operator!=(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() != rhs.microSecondsSinceEpoch();
    }

    inline bool operator>(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
    }

    inline bool operator>=(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() >= rhs.microSecondsSinceEpoch();
    }

    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }
}

#endif // _TIMEUTIL_H