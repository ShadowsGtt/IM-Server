#include "ThreadPool.h"
#include "Exception.h"

#include <assert.h>
#include <stdio.h>


ThreadPool::ThreadPool(const string& nameArg)
  : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{}

ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}

/* 线程池启动 */
void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i+1);
	threads_.emplace_back(new Thread(
          std::bind(&ThreadPool::runInThread, this), name_+id));
    threads_[i]->start();
  }
  if (numThreads == 0 && threadInitCallback_)
  {
    threadInitCallback_();
  }
}


/* 线程池停止 */
void ThreadPool::stop()
{
  {
  MutexLockGuard lock(mutex_);
  running_ = false;
  notEmpty_.notifyAll();
  }
  /* 等待线程池中的线程 */
  for (auto& thr : threads_)
  {
    thr->join();
  }
}


/* 获取任务队列任务个数 */
size_t ThreadPool::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return queue_.size();
}


/* 向任务队列添加任务 */
void ThreadPool::addTask(const Task& task)
{
  if (threads_.empty())
  {
    task();
  }
  else
  {
    MutexLockGuard lock(mutex_);
    
	/* 如果任务队列满,则睡眠等待 */
    while (isFull())
    {
      notFull_.wait();
    }
    assert(!isFull());

	/* 添加新任务并通知等待线程 */    
    queue_.push_back(task);
    notEmpty_.notify();
  }
}


/* 从任务队列获取任务 */
ThreadPool::Task ThreadPool::take()
{
  /* 上锁，防止对条件变量产生竞争 */
  MutexLockGuard lock(mutex_);
  
  /* 使用while防止虚假唤醒 */
  while (queue_.empty() && running_)
  {
    /* 线程进入睡眠，等待任务 */
    /* 线程在睡眠前会自动释放条件变量的锁 */ 
    /* 当 wait函数退出时，再次加锁 */
    notEmpty_.wait();
  }
  Task task;
  if (!queue_.empty())
  {
    task = queue_.front();
    queue_.pop_front();
    
	/* 取出任务后就唤醒因任务队列满而等待添加任务的线程 */
    if (maxQueueSize_ > 0)
    {
      notFull_.notify();
    }
  }
  return task;
}

/* 任务队列是否已满 */
bool ThreadPool::isFull() const
{
  mutex_.assertLocked();
  return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}


/* 线程池线程启动时运行的回调函数 */
void ThreadPool::runInThread()
{
  try
  {
	/* 运行用户指定的初始化函数 */
    if (threadInitCallback_)
    {
      threadInitCallback_();
    }
	
	while(true)    
	//while (running_)
    {
      Task task(take());
      if (task)
      {
        task();
      }
	  if(!running_ && queue_.empty() )
		break;
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw; // rethrow
  }
}

