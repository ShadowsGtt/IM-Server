#include <iostream>
#include "../server/src/Thread.h"
#include <functional>

using namespace std;


void initCallBack()
{
    cout << "thread [" << pthread_self() << "] running ! " << endl;
    cout << "t_threadname:" << CurrentThread::t_threadName << endl;
}


int main(void)
{

    Thread td1(std::bind(initCallBack));
    Thread td2(std::bind(initCallBack));
    Thread td3(std::bind(initCallBack));



    cout<< " td1 name : " << td1.name() << "     td1 seq:" << td1.sequence() << endl;
    cout<< " td2 name : " << td2.name() << "     td2 seq:" << td2.sequence() << endl;
    cout<< " td3 name : " << td3.name() << "     td3 seq:" << td3.sequence() << endl;

    td1.start();
    td2.start();
    td3.start();
    td1.join();
    td2.join();
    td3.join();

    return 0;
}
