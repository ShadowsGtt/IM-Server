//
// Created by lonnyliu(刘卓) on 2019-04-11.
//

#include "MysqlLinkPool.h"
#include "Logging.h"
#include "Mutex.h"
#include "MysqlUtil.h"
#include <list>
#include <string>

MysqlLinkPool* MysqlLinkPool::linkpool = new MysqlLinkPool("127.0.0.1", "root", "IM@tencent123", "IM", 3306, 20);

//连接池的构造函数  
MysqlLinkPool::MysqlLinkPool(string szHost, string szUser,
        string szPass, string szDB, int Port ,int maxSize)
        : Host(szHost), User(szUser), Pass(szPass), Port(Port), DB(szDB), maxSize(maxSize)
{
    this->curSize = 0;
    this->InitConnection(maxSize / 2);
}

//获取连接池对象，单例模式  
MysqlLinkPool* MysqlLinkPool::GetInstance()
{
    return linkpool;
}

//初始化连接池，创建最大连接数的一半连接数量  
void MysqlLinkPool::InitConnection(int iInitialSize)
{
    Comm::Mysql *conn;
    MutexLockGuard lock(mutex);
    for (int i = 0; i < iInitialSize; i++)
    {
        conn = this->CreateConnection();
        if (conn)
        {
            connList.push_back(conn);
            ++(this->curSize);
        }
        else
        {
            LOG_ERROR << "创建CONNECTION出错";
        }
    }
}

//创建连接,返回一个Connection  
Comm::Mysql* MysqlLinkPool::CreateConnection()
{
    Comm::Mysql* conn;
    try
    {
        conn = new Comm::Mysql(Host.c_str(), User.c_str(), Pass.c_str(), DB.c_str(), Port); //建立连接
        return conn;
    }
    catch (std::runtime_error&e)
    {
        LOG_ERROR << "运行时出错";
        return NULL;
    }
}

//在连接池中获得一个连接  
Comm::Mysql* MysqlLinkPool::GetConnection()
{
    Comm::Mysql* con;
    mutex.lock();
    if (connList.size() > 0)
    {   //连接池容器中还有连接
        con = connList.front(); //得到第一个连接  
        connList.pop_front();   //移除第一个连接  
        if (!con->Ping())
        {   //如果连接已经被关闭，删除后重新建立一个
            delete con;
            con = this->CreateConnection();
        }
        //如果连接为空，则创建连接出错  
        if (NULL == con)
        {
            --curSize;
        }
        mutex.unlock();
        return con;
    }
    else
    {
        if (curSize < maxSize)
        { //还可以创建新的连接
            con = this->CreateConnection();
            if (con)
            {
                ++curSize;
                mutex.unlock();
                return con;
            }
            else
            {
                mutex.unlock();
                return NULL;
            }
        }
        else
        { //建立的连接数已经达到maxSize
            mutex.unlock();
            return NULL;
        }
    }
}

//回收数据库连接  
void MysqlLinkPool::ReleaseConnection(Comm::Mysql* conn)
{
    if (conn)
    {
        MutexLockGuard lock(mutex);
        connList.push_back(conn);
    }
}

//连接池的析构函数  
MysqlLinkPool::~MysqlLinkPool()
{
    this->DestoryMysqlLinkPool();
}

//销毁连接池,首先要先销毁连接池的中连接  
void MysqlLinkPool::DestoryMysqlLinkPool()
{
    list<Comm::Mysql*>::iterator icon;
    {
        MutexLockGuard lock(mutex);
        for (icon = connList.begin(); icon != connList.end(); ++icon)
        {
            this->DestoryConnection(*icon); //销毁连接池中的连接
        }
        curSize = 0;
        connList.clear(); //清空连接池中的连接
    }
}

//销毁一个连接  
void MysqlLinkPool::DestoryConnection(Comm::Mysql* conn) {
    if (conn)
    {
        try
        {
            conn->Close();
        } catch (std::exception&e)
        {
            LOG_ERROR << e.what();
        }
        delete conn;
    }
}
