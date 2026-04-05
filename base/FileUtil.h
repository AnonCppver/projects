#ifndef _LEEF_BASE_FILEUTIL_H
#define _LEEF_BASE_FILEUTIL_H

#include "noncopyable.h"
#include "StringPiece.h"
#include <sys/types.h>  // for off_t

namespace leef
{
namespace FileUtil
{

// read small file < 64KB
class ReadSmallFile : noncopyable
{
 public:
  ReadSmallFile(StringPiece filename);
  ~ReadSmallFile();

  // return errno
  template<typename String>
  int readToString(int maxSize,
                   String* content,
                   int64_t* fileSize,
                   int64_t* modifyTime,
                   int64_t* createTime);

  /// Read at maxium kBufferSize into buf_
  // return errno
  int readToBuffer(int* size);

  const char* buffer() const { return m_buf; }

  static const int kBufferSize = 64*1024;

 private:
  int m_fd;
  int m_err;
  char m_buf[kBufferSize];
};

// read the file content, returns errno if error happens.
template<typename String>
int readFile(StringPiece filename,
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
  ReadSmallFile file(filename);
  return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

// not thread safe
class AppendFile : noncopyable
{
 public:
  explicit AppendFile(StringPiece filename);

  ~AppendFile();

  void append(const char* logline, size_t len);

  void flush();

  off_t writtenBytes() const { return m_writtenBytes; }

 private:

  size_t write(const char* logline, size_t len);

  FILE* m_fp;
  char m_buffer[64*1024];
  off_t m_writtenBytes;
};

}  // namespace FileUtil
}  // namespace leef

#endif  // _LEEF_BASE_FILEUTIL_H

