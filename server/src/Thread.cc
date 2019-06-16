#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
#include "Logging.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>


namespace CurrentThread
{
  __thread int t_cachedTid = 0;
  __thread char t_tidString[32];
  __thread int t_tidStringLength = 6;
  __thread const char* t_threadName = "unknown";
  static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");
}


namespace detail
{

pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
  CurrentThread::t_cachedTid = 0;
  CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData
{
  typedef Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(const ThreadFunc& func,
             const string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : func_(func),
      name_(name),
      tid_(tid),
      latch_(latch)
  { }


/* 线程最终运行的函数 */
  void runInThread()
  {
    *tid_ = CurrentThread::tid();
    tid_ = NULL;
    latch_->countDown();
    latch_ = NULL;

    CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
    ::prctl(PR_SET_NAME, CurrentThread::t_threadName);
    CurrentThread::t_threadName = "finished";
       try
       {
       func_();
       CurrentThread::t_threadName = "finished";
       }
       catch (const Exception& ex)
       {
       CurrentThread::t_threadName = "crashed";
       fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
       fprintf(stderr, "reason: %s\n", ex.what());
       fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
       abort();
       }
       catch (const std::exception& ex)
       {
       CurrentThread::t_threadName = "crashed";
       fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
       fprintf(stderr, "reason: %s\n", ex.what());
       abort();
       }
       catch (...)
       {
       CurrentThread::t_threadName = "crashed";
       fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
       throw; // rethrow
       }
  }
};


/* pthread_create 的回调函数 */
void* startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

}   // end namespace detail




void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread()
{
  return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}




// 下面是Thread类相关


std::atomic<int32_t> Thread::numCreated_;




Thread::Thread(const ThreadFunc& func, const string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(n),
    latch_(1),
    sequence_(++numCreated_)
{
  setDefaultName();
}


Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadId_);
  }
}



void Thread::setDefaultName()
{
  int num = sequence_;
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);

/* 创建新线程失败 */
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
  {
    started_ = false;
    delete data; // or no delete?
    LOG_SYSFATAL << "Failed in pthread_create";
  }
  else
  {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}
