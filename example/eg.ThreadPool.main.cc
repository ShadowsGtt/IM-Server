#include <iostream>
#include "../server/src/Thread.h"
#include "../server/src/ThreadPool.h"
#include <functional>

using namespace std;

MutexLock lk;

void initCallBack()
{
    cout << "thread [" << pthread_self() << "] running ! " << endl;
}

void task(int i)
{
    MutexLockGuard mg(lk);
    cout << "thread [" << pthread_self() << "] get task ! " << endl;
    cout << " number is :" << i << endl;

}

int main(void)
{

    /* 创建线程池并命名 */
    ThreadPool tp("shadows thread pool");

    /* 设置线程池任务队列大小 */
    tp.setMaxQueueSize(1024);

    /* 设置线程池中的线程启动时运行的初始化函数 */
    tp.setThreadInitCallback(std::bind(&initCallBack));

    /* 设置线程池中线程数目并启动线程池 */
    tp.start(5);

    /* 打印线程池名字 */
    cout << "name of thread pool :" << tp.name() << endl;

    /* 打印任务队列中任务个数 */
    cout << "count of task :" << tp.queueSize() << endl;

    /* 向线程池中添加任务 */
    for ( int i = 0 ;i < 10 ; ++i )
        tp.addTask(std::bind(&task,i));

    /* 线程池停止 */
    tp.stop();

    return 0;
}
