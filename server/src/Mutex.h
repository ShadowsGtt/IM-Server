
#ifndef _MUTEX_H
#define _MUTEX_H

#include "CurrentThread.h"
#include "nocopyable.h"
#include <assert.h>
#include <pthread.h>


class MutexLock : noncopyable
{
 public:
  MutexLock(): holder_(0)
  {
	pthread_mutex_init(&mutex_, NULL);
  }

  ~MutexLock()
  {
    	assert(holder_ == 0);
	pthread_mutex_destroy(&mutex_);
  }

  // must be called when locked, i.e. for assertion
  bool isLockedByThisThread() const
  {
    return holder_ == CurrentThread::tid();
  }

  void assertLocked() const
  {
    assert(isLockedByThisThread());
  }

  // internal usage

  void lock()
  {
	pthread_mutex_lock(&mutex_);
    	assignHolder();
  }

  void unlock()
  {
    	unassignHolder();
	pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t* getPthreadMutex() /* non-const */
  {
    return &mutex_;
  }

 private:
  friend class Condition;


  void unassignHolder()
  {
    holder_ = 0;
  }

  void assignHolder()
  {
    holder_ = CurrentThread::tid();
  }

  pthread_mutex_t mutex_;
  pid_t holder_;



  class UnassignGuard : noncopyable
  {
   public:
    UnassignGuard(MutexLock& owner)
      : owner_(owner)
    {
      owner_.unassignHolder();
    }

    ~UnassignGuard()
    {
      owner_.assignHolder();
    }

   private:
    MutexLock& owner_;
  };

};




class MutexLockGuard : noncopyable
{
 public:
  explicit MutexLockGuard(MutexLock& mutex)
    : mutex_(mutex)
  {
    mutex_.lock();
  }

  ~MutexLockGuard()
  {
    mutex_.unlock();
  }

 private:

  MutexLock& mutex_;
};

class CacheNotify
{
public:
    CacheNotify()
    {
      pthread_mutexattr_init(&m_mutexattr);
      pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&m_mutex, &m_mutexattr);
      pthread_cond_init(&m_cond, NULL);
    }

    ~CacheNotify()
    {
      pthread_mutexattr_destroy(&m_mutexattr);
      pthread_mutex_destroy(&m_mutex);
      pthread_cond_destroy(&m_cond);
    }

    void Lock() { pthread_mutex_lock(&m_mutex); }

    void Unlock() { pthread_mutex_unlock(&m_mutex); }

    void Wait() { pthread_cond_wait(&m_cond, &m_mutex); }

    void Signal() { pthread_cond_signal(&m_cond); }

private:
    pthread_mutex_t m_mutex;
    pthread_mutexattr_t m_mutexattr;

    pthread_cond_t m_cond;
};

#define MutexLockGuard(x) error "Missing guard object name"

#define MutexLockGuard(x) error "Missing guard object name"

#endif  // _MUTEX_H

