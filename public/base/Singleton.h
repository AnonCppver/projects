#ifndef _LEEF_BASE_SINGLETON_H
#define _LEEF_BASE_SINGLETON_H

#include "noncopyable.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h> // atexit
#include <type_traits>
#include <boost/type_traits/is_detected.hpp>

namespace leef
{

  namespace detail
  {
    template <typename T>
    using has_no_destroy = decltype(std::declval<T>().no_destroy());
    template <typename T>
    constexpr bool test_no_destroy()
    {
      return boost::is_detected<has_no_destroy, T>::value;
    }
  } // namespace detail

  template <typename T>
  class Singleton : noncopyable
  {
  public:
    Singleton() = delete;
    ~Singleton() = delete;

    static T &instance()
    {
      pthread_once(&ponce_, &Singleton::init);
      assert(value_ != NULL);
      return *value_;
    }

  private:
    static void init()
    {
      value_ = new T();
      if (!detail::test_no_destroy<T>())
      {
        ::atexit(destroy);
      }
    }

    static void destroy()
    {
      typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
      T_must_be_complete_type dummy;
      (void)dummy;

      delete value_;
      value_ = NULL;
    }

  private:
    static pthread_once_t ponce_;
    static T *value_;
  };

  template <typename T>
  pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

  template <typename T>
  T *Singleton<T>::value_ = NULL;

} // namespace leef

#endif // _LEEF_BASE_SINGLETON_H
