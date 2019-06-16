#ifndef _TIMER_H
#define _TIMER_H

#include <atomic>
#include <functional>
#include "Timestamp.h"
#include "nocopyable.h"

namespace net
{

typedef std::function<void()> TimerCallback;


class Timer : noncopyable
{
 public:
  Timer(const TimerCallback& cb, Timestamp when, double interval)
    : callback_(cb),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(++s_numCreated_)
  { }

  /* 运行回调 */
  void run() const
  {
    callback_();
  }


  /* 返回定时器的截止时间 */
  Timestamp expiration() const  { return expiration_; }
  
  /* 是否设置以一定的时间间隔重复激活的定时器 */
  bool repeat() const { return repeat_; }
  
  /* 返回定时器的序列号 */
  int64_t sequence() const { return sequence_; }

  /* 重启定时器  用于设置repeat的Timer */
  void restart(Timestamp now);        

  //static int64_t numCreated() { return s_numCreated; }

 private:
  const TimerCallback callback_;	//定时器到时的回调	
  Timestamp expiration_;            //定时器截止时间
  const double interval_;			//定时器时长
  const bool repeat_;				//是否重复
  const int64_t sequence_;			//当前定时器的序列号

  static std::atomic<int64_t>  s_numCreated_; //多线程安全的原子增长的序列号
};

} // end namespace net
#endif  // _TIMER_H
