#include "LogFile.h"

#include "FileUtil.h"
#include "ProcessInfo.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace leef;

LogFile::LogFile(const std::string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
  : m_basename(basename),
    m_rollSize(rollSize),
    m_flushInterval(flushInterval),
    m_checkEveryN(checkEveryN),
    m_count(0),
    m_startOfPeriod(0),
    m_lastRoll(0),
    m_lastFlush(0)
{
  assert(basename.find('/') == std::string::npos);
  m_pMutex=threadSafe?std::make_unique<std::mutex>():nullptr;
  rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
  if (m_pMutex)
  {
    std::lock_guard<std::mutex> lock(*m_pMutex);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFile::flush()
{
  if (m_pMutex)
  {
    std::lock_guard<std::mutex> lock(*m_pMutex);
    m_file->flush();
  }
  else
  {
    m_file->flush();
  }
}

void LogFile::append_unlocked(const char* logline, int len)
{
  m_file->append(logline, len);

  if (m_file->writtenBytes() > m_rollSize)
  {
    rollFile();
  }
  else
  {
    ++m_count;
    if (m_count >= m_checkEveryN)
    {
      m_count = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      // 跨天滚动日志文件，或者超过刷新时间间隔
      if (thisPeriod_ != m_startOfPeriod)
      {
        rollFile();
      }
      else if (now - m_lastFlush > m_flushInterval)
      {
        m_lastFlush = now;
        m_file->flush();
      }
    }
  }
}

bool LogFile::rollFile()
{
  time_t now = 0;
  std::string filename = getLogFileName(m_basename, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  // 滚动间隔为1秒，防止同一秒内滚动多次日志文件
  // 但是1秒的间隔可能导致日志文件过大，超过rollSize
  // 如果不检查滚动间隔，可能会在同一秒内多次写入同一文件(basename+time_second)
  if (now > m_lastRoll)
  {
    m_lastRoll = now;
    m_lastFlush = now;
    m_startOfPeriod = start;
    m_file.reset(new FileUtil::AppendFile(filename));
    return true;
  }
  return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];

  struct tm tm;
  *now = time(NULL);
  localtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  filename += ProcessInfo::hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}

