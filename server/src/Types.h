#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <cstring>
#include <string>
#include <assert.h>


using namespace std;


inline void memZero(void* p, size_t n)
{
  memset(p, 0, n);
}

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
  return f;
}


template<typename To, typename From>     // use like this: down_cast<T*>(foo);
inline To down_cast(From* f)                     // so we only accept pointers
{
  if (false)
  {
    implicit_cast<From*, To>(0);
  }

  return static_cast<To>(f);
}


#endif
