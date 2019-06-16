//
// Created by lonnyliu(刘卓) on 2019-04-11.
//

#ifndef DAO_MYSQLLINKPOOL_H
#define DAO_MYSQLLINKPOOL_H

#include "MysqlUtil.h"
#include "Mutex.h"
#include <list>
#include <string>
using namespace std;


class MysqlLinkPool
{
private:
    int curSize; //当前已建立的数据库连接数量
    int maxSize; //连接池中定义的最大数据库连接数
    list<Comm::Mysql*> connList; //连接池的容器队列  STL list 双向链表
    MutexLock mutex; //线程锁
    static MysqlLinkPool *linkpool;

    Comm::Mysql* CreateConnection(); //创建一个连接
    void InitConnection(int iInitialSize); //初始化数据库连接池
    void DestoryConnection(Comm::Mysql *conn); //销毁数据库连接对象
    void DestoryMysqlLinkPool(); //销毁数据库连接池
    MysqlLinkPool(string szHost, string szUser, string szPass, string szDB, int Port ,int maxSize);

private:
    string Host;
    string User;
    string Pass;
    string DB;
    int Port;

public:
    ~MysqlLinkPool();
    Comm::Mysql* GetConnection(); //获得数据库连接
    void ReleaseConnection(Comm::Mysql *conn); //将数据库连接放回到连接池的容器中
    static MysqlLinkPool *GetInstance(); //获取数据库连接池对象
};


#endif //DAO_MYSQLLINKPOOL_H
