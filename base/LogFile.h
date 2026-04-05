#ifndef _LEEF_BASE_LOGFILE_H
#define _LEEF_BASE_LOGFILE_H

#include <mutex>
#include <memory>

#include "noncopyable.h"

namespace leef
{

namespace FileUtil
{
class AppendFile;
}

class LogFile : noncopyable
{
 public:
  LogFile(const std::string& basename,
          off_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1024);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool rollFile();

 private:
  void append_unlocked(const char* logline, int len);

  static std::string getLogFileName(const std::string& basename, time_t* now);

  const std::string m_basename;
  const off_t m_rollSize;       // 最大文件大小
  const int m_flushInterval;    // 刷新时间间隔
  const int m_checkEveryN;      // 每N次append调用，检查一次是否需要滚动日志文件

  int m_count;

  std::unique_ptr<std::mutex> m_pMutex;
  time_t m_startOfPeriod;       // 当前日志文件的起始时间，单位为秒
  time_t m_lastRoll;            // 上次滚动日志文件的时间，单位为秒
  time_t m_lastFlush;           // 上次刷新日志文件的时间，单位为秒
  std::unique_ptr<FileUtil::AppendFile> m_file;

  const static int kRollPerSeconds_ = 60*60*24;
};

}  // namespace leef
#endif  // _LEEF_BASE_LOGFILE_H
