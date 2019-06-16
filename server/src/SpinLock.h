#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <atomic>

class SpinLock
{

public:
    SpinLock() : flag_(ATOMIC_FLAG_INIT) {}

    bool Lock(){
        while ( flag_.test_and_set(std::memory_order_acquire) );
        return true;  
    }

    void unLock(){
        flag_.clear(std::memory_order_release);
    }
private:
    std::atomic_flag flag_;

};

#endif // end _SPINLOCK_H
