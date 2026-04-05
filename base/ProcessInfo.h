#ifndef _LEEF_BASE_PROCESSINFO_H
#define _LEEF_BASE_PROCESSINFO_H

#include "StringPiece.h"
#include "timeUtil.h"
#include <vector>
#include <sys/types.h>

namespace leef
{

namespace ProcessInfo
{
  pid_t pid();
  std::string pidString();
  uid_t uid();
  std::string username();
  uid_t euid();
  Timestamp startTime();
  int clockTicksPerSecond();
  int pageSize();
  constexpr bool isDebugBuild();

  std::string hostname();
  std::string procname();
  StringPiece procname(const std::string& stat);

  /// read /proc/self/status
  std::string procStatus();

  /// read /proc/self/stat
  std::string procStat();

  /// read /proc/self/task/tid/stat
  std::string threadStat();

  /// readlink /proc/self/exe
  std::string exePath();

  int openedFiles();
  int maxOpenFiles();

  struct CpuTime
  {
    double userSeconds;
    double systemSeconds;

    CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }

    double total() const { return userSeconds + systemSeconds; }
  };
  CpuTime cpuTime();

  int numThreads();
  std::vector<pid_t> threads();
}  // namespace ProcessInfo

}  // namespace leef

#endif  // _LEEF_BASE_PROCESSINFO_H
