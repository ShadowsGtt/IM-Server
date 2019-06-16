
#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <deque>
#include <vector>
#include <functional>
#include <memory>

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "Types.h"
#include "nocopyable.h"


class ThreadPool : noncopyable
{
 public:
  typedef std::function<void ()> Task;

  explicit ThreadPool(const string& nameArg = string("ThreadPool"));
  ~ThreadPool();

  // Must be called before start().
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
  void setThreadInitCallback(const Task& cb)
  { threadInitCallback_ = cb; }

  void start(int numThreads);
  void stop();

  const string& name() const
  { return name_; }

  size_t queueSize() const;

  // Could block if maxQueueSize > 0
  void addTask(const Task& f);

 private:
  bool isFull() const;
  void runInThread();
  Task take();

  mutable MutexLock mutex_;
  Condition notEmpty_;   /* 任务队列非空的条件变量 */
  Condition notFull_;    /* 任务队列未满的条件变量 */
  string name_;
  Task threadInitCallback_;  /* 初始化函数，由用户指定，每个线程都会运行 */
  //boost::ptr_vector<Thread> threads_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<Task> queue_;
  size_t maxQueueSize_;
  bool running_;
};

#endif
