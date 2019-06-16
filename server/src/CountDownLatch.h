
#ifndef _COUNTDOWNLATCH_H
#define _COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"
#include "nocopyable.h"


class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

#endif  // _COUNTDOWNLATCH_H
