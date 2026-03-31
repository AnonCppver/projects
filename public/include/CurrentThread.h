// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef _PUBLIC_CURRENTTHREAD_H
#define _PUBLIC_CURRENTTHREAD_H

#include<stdint.h>
#include<string>

namespace prj
{
namespace CurrentThread
{
  // internal
  extern thread_local int tl_cachedTid;
  extern thread_local char tl_tidString[32];
  extern thread_local int tl_tidStringLength;
  extern thread_local const char* tl_threadName;
  void cacheTid();

  inline int tid()
  {
    if (__builtin_expect(tl_cachedTid == 0, 0))
    {
      cacheTid();
    }
    return tl_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return tl_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return tl_tidStringLength;
  }

  inline const char* name()
  {
    return tl_threadName;
  }

  bool isMainThread();

  void sleepUsec(int64_t usec);  // for testing

  std::string stackTrace(bool demangle);
}  // namespace CurrentThread
}  // namespace prj

#endif  // _PUBLIC_CURRENTTHREAD_H
