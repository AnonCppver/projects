#include <sys/time.h>
#include <stdio.h>
#include <inttypes.h>

#include "timeUtil.h"
#include "strUtil.h"

using namespace std;

namespace prj
{
    std::string time2str(time_t t, const string &fmt)
    {
        std::tm tm{};
        localtime_r(&t, &tm);

        int y = tm.tm_year + 1900;
        int m = tm.tm_mon + 1;

        char buf[32];

        if (fmt.empty() || fmt == "yyyy-mm-dd hh24:mi:ss")
        {
            snprintf(buf, sizeof(buf),
                     "%04d-%02d-%02d %02d:%02d:%02d",
                     y, m, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
        else if (fmt == "yyyy-mm-dd hh24:mi")
        {
            snprintf(buf, sizeof(buf),
                     "%04d-%02d-%02d %02d:%02d",
                     y, m, tm.tm_mday,
                     tm.tm_hour, tm.tm_min);
        }
        else if (fmt == "yyyy-mm-dd")
        {
            snprintf(buf, sizeof(buf),
                     "%04d-%02d-%02d",
                     y, m, tm.tm_mday);
        }
        else if (fmt == "yyyymmddhh24miss")
        {
            snprintf(buf, sizeof(buf),
                     "%04d%02d%02d%02d%02d%02d",
                     y, m, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
        else if (fmt == "yyyymmdd")
        {
            snprintf(buf, sizeof(buf),
                     "%04d%02d%02d",
                     y, m, tm.tm_mday);
        }
        else if (fmt == "hh24miss")
        {
            snprintf(buf, sizeof(buf),
                     "%02d%02d%02d",
                     tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
        else
        {
            snprintf(buf, sizeof(buf),
                     "%04d-%02d-%02d %02d:%02d:%02d",
                     y, m, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec);
        }

        return std::string(buf);
    }

    time_t str2time(const string &strtime)
    {
        char buf[14];
        int n = 0;

        for (char c : strtime)
        {
            if ((unsigned)(c - '0') < 10)
            {
                buf[n++] = c;
                if (n == 14)
                    break;
            }
        }

        if (n != 14)
            return -1;

        auto to2 = [](const char *p)
        {
            return (p[0] - '0') * 10 + (p[1] - '0');
        };

        auto to4 = [](const char *p)
        {
            return (p[0] - '0') * 1000 +
                   (p[1] - '0') * 100 +
                   (p[2] - '0') * 10 +
                   (p[3] - '0');
        };

        std::tm t{};

        t.tm_year = to4(buf) - 1900;
        t.tm_mon = to2(buf + 4) - 1;
        t.tm_mday = to2(buf + 6);
        t.tm_hour = to2(buf + 8);
        t.tm_min = to2(buf + 10);
        t.tm_sec = to2(buf + 12);
        t.tm_isdst = -1;

        return std::mktime(&t);
    }

    static_assert(sizeof(Timestamp) == sizeof(int64_t),
                  "Timestamp should be same size as int64_t");

    string Timestamp::toString() const
    {
        char buf[32] = {0};
        int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
        int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
        snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
        return buf;
    }

    string Timestamp::toFormattedString(bool showMicroseconds) const
    {
        char buf[64] = {0};
        time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
        struct tm tm_time;
        gmtime_r(&seconds, &tm_time);

        if (showMicroseconds)
        {
            int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
            snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                     microseconds);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                     tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        }
        return buf;
    }

    Timestamp Timestamp::now()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int64_t seconds = tv.tv_sec;
        return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
    }

}
