
#ifndef _EVENTLOOPTHREAD_H
#define _EVENTLOOPTHREAD_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"


namespace net
{

class EventLoop;

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const string& name = string());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_ ;
  bool exiting_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_ ;
  ThreadInitCallback callback_;
};

}  // namespace net

#endif  // _EVENTLOOPTHREAD_H

