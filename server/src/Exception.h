#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "Types.h"
#include <exception>
#include <string>

using namespace std;

class Exception : public std::exception
{
 public:
  explicit Exception(const char* what);
  explicit Exception(const string& what);
  virtual ~Exception() throw();
  virtual const char* what() const throw();
  const char* stackTrace() const throw();

 private:
  void fillStackTrace();

  string message_;
  string stack_;
};

#endif  // _EXCEPTION_H
