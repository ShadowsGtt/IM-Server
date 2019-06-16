#include "Timer.h"


using namespace net;

std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
  if (repeat_)   //设置重复则再次激活定时器
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = Timestamp::invalid();   //返回空的Timestamp
  }
}
