#include <iostream>
#include "../server/src/Mutex.h"

using namespace std;


int main(void)
{
    /* 新建一个互斥锁 */
    MutexLock mtx;

    /* 上锁 */
    mtx.lock();

    /* 未上锁时，程序会退出 */
    mtx.assertLocked();

    /* 解锁 */
    mtx.unlock();

    /* 获得原生锁 */
    pthread_mutex_t *mt = mtx.getPthreadMutex();

    /* 锁是否被本线程锁住 */
    if(mtx.isLockedByThisThread())
        cout << "is locked by this thread" << endl;
    else
        cout << "is not locked by this thread" << endl;

    {
        /* 对mtx加锁 */
        MutexLockGuard mx(mtx);
        if(mtx.isLockedByThisThread())
            cout << "MutexLockGuard locked" << endl;
        else
            cout << "MutexLockGuard unlocked" << endl;
    }
    if(mtx.isLockedByThisThread())
        cout << "MutexLockGuard locked" << endl;
    else
        cout << "MutexLockGuard unlocked" << endl;

    return 0;
}
