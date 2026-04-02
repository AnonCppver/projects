#ifndef _LEEF_BASE_TIMEUTIL_H
#define _LEEF_BASE_TIMEUTIL_H

#include <chrono>
#include <string>

#include "copyable.h"
#include <boost/operators.hpp>

namespace leef
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

        void reset()
        {
            m_start = clock::now();
        }

    private:
        time_point m_start;
    };

    std::string time2str(const time_t ttime, const std::string &fmt = "");
    void time2str(char *buf, size_t bufLen, time_t t, const std::string &fmt = "");
    time_t str2time(const std::string &strtime);

    class Timestamp : public leef::copyable
    {
    public:
        ///
        /// Constucts an invalid Timestamp.
        ///
        Timestamp()
            : microSecondsSinceEpoch_(0)
        {
        }

        ///
        /// Constucts a Timestamp at specific time
        ///
        /// @param microSecondsSinceEpoch
        explicit Timestamp(int64_t microSecondsSinceEpochArg)
            : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
        {
        }

        void swap(Timestamp &that)
        {
            std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
        }

        // default copy/assignment/dtor are Okay

        std::string toString() const;
        std::string toFormattedString(bool showMicroseconds = true) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        // for internal usage.
        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        time_t secondsSinceEpoch() const
        {
            return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
        }

        ///
        /// Get time of now.
        ///
        static Timestamp now();
        static Timestamp invalid()
        {
            return Timestamp();
        }

        static Timestamp fromUnixTime(time_t t)
        {
            return fromUnixTime(t, 0);
        }

        static Timestamp fromUnixTime(time_t t, int microseconds)
        {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }

        static const int kMicroSecondsPerSecond = 1000 * 1000;

    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline double timeDifference(Timestamp high, Timestamp low)
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }

    inline time_t utc2local(time_t utc)
    {
        // 一次调用 即使多线程
        static const int difference = []() -> int
        {
            time_t now = time(nullptr);

            struct tm tm_local{};
            struct tm tm_utc{};

            localtime_r(&now, &tm_local);
            gmtime_r(&now, &tm_utc);

            time_t local_sec = timegm(&tm_local);
            time_t utc_sec = timegm(&tm_utc);

            return static_cast<int>(local_sec - utc_sec);
        }();

        return utc + difference;
    }

}

#endif // _LEEF_BASE_TIMEUTIL_H