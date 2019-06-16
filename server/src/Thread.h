
#ifndef _THREAD_H
#define _THREAD_H

#include <functional>
#include <pthread.h>
#include <string>
#include <atomic>

#include "CountDownLatch.h"
#include "Types.h"
#include "Timestamp.h"
#include "nocopyable.h"

using namespace std;

class Timestamp;


class Thread : noncopyable 
{
 public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(const ThreadFunc&, const string& name = string()); 
  ~Thread();

  /* 线程开始运行 */
  void start();

  /* 等待线程运行结束 */
  int join(); 

  /* 是否正在运行 */
  bool started() const { return started_; }

  /* 返回线程id */
  pid_t tid() const { return tid_; }
  
  /* 返回线程名字 */
  const string& name() const { return name_; }

  /* 返回线程序列号 */
  const int32_t sequence() { return sequence_; }

  
 private:
  /* 返回numCreated值 */
  static int numCreated() { return numCreated_; }

  /* 设置线程的默认名字  Thread1 Thread2 ... */
  void setDefaultName();

  bool       started_;
  bool       joined_;
  pthread_t  pthreadId_;
  pid_t      tid_;
  ThreadFunc func_;
  string     name_;
  CountDownLatch latch_;

  /* 线程序列号 递增的 */
  int sequence_;

  static std::atomic<int32_t> numCreated_;
};

#endif
