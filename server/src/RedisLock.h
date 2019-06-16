#include <utility>

//
// Created by lonnyliu(刘卓) on 2019-05-23.
//

#ifndef LONNYIM_REDISLOCK_H
#define LONNYIM_REDISLOCK_H

#include    "CachePool.h"
#include "Logging.h"

class RedisLock
{
public:
    RedisLock(CacheConn* cacheconn, string redisname, string key, int expiretime = 86400);

    ~RedisLock();

    string GetLockKey();

    void clearLock(const string &key);

    int unlock();

    int64_t getLockTimestamp();

    bool lock();
private:
    bool tryLock(int time);

private:
    CacheConn *m_cacheconn;
    string key;
    int32_t expiretime;
    string redisname;
    bool locked;

    int64_t lockTimestamp;

};

RedisLock::RedisLock(CacheConn* cacheconn, string redisname, string key, int expiretime): redisname(std::move(redisname)), key(std::move(key)),expiretime(expiretime)
{
    m_cacheconn = cacheconn;
    locked = false;
}

string RedisLock::GetLockKey()
{
    string lockkey = key + "_Lock";
    return lockkey;
}

RedisLock::~RedisLock() = default;

bool RedisLock::tryLock(int time)
{
    LOG_INFO << "[REDISLOCK] trylock redisname " << redisname << " rediskey " <<  key ;

    if (!locked)
    {
        string tmp = GetLockKey();
        int value = m_cacheconn->setnx(tmp, time);
        LOG_INFO << "[REDISLOCK] setnx key= " << tmp << " time = " << time ;

        if ( 1 == value )
        {
            locked = true;
        }
    }
    else
    {
        LOG_INFO << "[REDISLOCK] not setnx ";
        return locked;
    }

    return locked;
}

int64_t RedisLock::getLockTimestamp()
{
    return lockTimestamp;
}

void RedisLock::clearLock(const string &key)
{
    string tmp = GetLockKey();
    m_cacheconn->del(tmp);
}

int32_t RedisLock::unlock()
{
    if (locked)
    {
        try
        {
            string tmp = GetLockKey();
            m_cacheconn->del(tmp);
            locked = false;
        }
        catch (exception e)
        {
            LOG_ERROR << "Excesption delete key" << e.what();
            return -1;
        }

    }
    return  0;
}

bool RedisLock::lock()
{
    uint32_t uiCurTime = (uint32_t) time(NULL);
    // 保存超时的时间
    int time = (uiCurTime + expiretime + 1);
    if (tryLock(time))
    {
        lockTimestamp = time;
    }
    else
    {
        // 锁失败，看看 timestamp 是否超时
        string value = m_cacheconn->get(key);
        string newtime  = to_string(time);
        LOG_INFO << "---lock value " << value << " curtime  = " << uiCurTime <<  "newtime = " << time;
        if (uiCurTime > stoi(value))
        {
            // 锁已经超时，尝试 GETSET 操作
            value = m_cacheconn->getset(key, newtime);
            // 返回的时间戳如果仍然是超时的，那就说明，如愿以偿拿到锁，否则是其他进程/线程设置了锁
            if (uiCurTime > stoi(value))
            {
                locked = true;
            }
            else
            {
                LOG_ERROR<< "GETSET 锁的旧值是：" << value << ", key=" << key;
            }
        }
        else
        {
            LOG_ERROR << "GET 锁的当前值是：" << value  << ", key="  << key;
        }
        lockTimestamp = stoi(value);
    }
    return locked;
}

#endif //LONNYIM_REDISLOCK_H
