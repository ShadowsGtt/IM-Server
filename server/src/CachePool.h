//
// Created by lonnyliu(刘卓) on 2019-05-22.
//

#ifndef IM_CACHEPOOL_H
#define IM_CACHEPOOL_H

#include <vector>
#include <list>
#include <string>
#include <list>
#include <map>
#include <google/protobuf/message.h>
#include <hiredis.h>
#include "Mutex.h"
#include "IM.pb.h"
#include <hircluster.h>

using namespace std;

class CachePool;

class CacheConn
{
public:
    CacheConn(CachePool *pCachePool);
    virtual ~CacheConn();
    int Init();
    const char *GetPoolName();
    string get(string key);
    string setex(string key, int timeout, string value);
    int setnx(string key, int time);
    string expire(string key, int time);
    string set(string key, string &value);
    string setpb(string key, int uiLifeTimeSec, const ::google::protobuf::Message &objPBValue);
    string getpb(string key, ::google::protobuf::Message &objPBValue);
    string getset(string key, string &value);
    string del(string key);
    //批量获取
    bool mget(const vector<string> &keys, map<string, string> &ret_value);
    // 判断一个key是否存在
    bool isExists(string &key);
    // Redis hash structure
    long hdel(string key, string field);
    string hget(string key, string field);
    bool hgetAll(string key, map<string, string> &ret_value);
    long hset(string key, string field, string value);
    long hincrBy(string key, string field, long value);
    long incrBy(string key, long value);
    string hmset(string key, map<string, string> &hash);
    bool hmget(string key, list<string> &fields, list<string> &ret_value);
    //原子加减1
    long incr(string key);
    long decr(string key);
    // Redis list structure
    long lpush(string key, string value);
    long rpush(string key, string value);
    long llen(string key);
    bool lrange(string key, long start, long end, list<string> &ret_value);
private:
    CachePool *m_pCachePool;
    redisClusterContext *m_pContext;
    uint64_t m_last_connect_time;
};

class CachePool
{
public:
    CachePool(const char *pool_name, const char *server_ip, int server_port, int db_num, int max_conn_cnt);

    virtual ~CachePool();

    int Init();

    CacheConn *GetCacheConn();

    void RelCacheConn(CacheConn *pCacheConn);

    const char *GetPoolName() { return m_pool_name.c_str(); }

    const char *GetServerIP() { return m_server_ip.c_str(); }

    int GetServerPort() { return m_server_port; }

    int GetDBNum() { return m_db_num; }

private:
    string m_pool_name;
    string m_server_ip;
    int m_server_port;
    int m_db_num;

    int m_cur_conn_cnt;
    int m_max_conn_cnt;
    list<CacheConn *> m_free_list;
    CacheNotify m_lock;
};

class CacheManager
{
public:
    virtual ~CacheManager();

    static CacheManager *getInstance();

    int Init();

    CacheConn *GetCacheConn(const char *pool_name);

    void RelCacheConn(CacheConn *pCacheConn);

private:
    CacheManager();

private:
    static CacheManager *cache_manager;
    map<string, CachePool*> m_cache_pool_map;
};

#endif //IM_CACHEPOOL_H
