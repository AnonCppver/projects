#include "Exception.h"
#include "CurrentThread.h"

namespace leef
{

Exception::Exception(std:: string msg)
  : message_(std::move(msg)),
    stack_(CurrentThread::stackTrace(/*demangle=*/false))
{
}

}  // namespace leef
