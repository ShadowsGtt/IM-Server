//
// Created by lonnyliu(刘卓) on 2019-05-22.
//

#include "CachePool.h"
#include "ConfigFileReader.h"
#include "Logging.h"
#include <iostream>
#include <google/protobuf/message.h>
#include <cstring>
#include "IMComm.h"


#define MIN_CACHE_CONN_CNT    2

CacheManager *CacheManager::cache_manager = NULL;

CacheConn::CacheConn(CachePool *pCachePool)
{
    m_pCachePool = pCachePool;
    m_pContext = NULL;
    m_last_connect_time = 0;
}

CacheConn::~CacheConn()
{
    if (m_pContext)
    {
        redisClusterFree(m_pContext);
        m_pContext = NULL;
    }
}

/*
 * redis初始化连接和重连操作，类似mysql_ping()
 */
int CacheConn::Init()
{
    m_pContext = redisClusterContextInit();
    if (!m_pContext)
    {
        LOG_ERROR << "redisClusterContextInit failed";
        return -1;
    }

    string cluster = string(m_pCachePool->GetServerIP()) + ":" +  to_string(m_pCachePool->GetServerPort());
    redisClusterSetOptionAddNodes(m_pContext,cluster.c_str());
    redisClusterConnect2(m_pContext);
    if ( NULL != m_pContext && m_pContext->err)
    {
        LOG_ERROR << "redis error :" << m_pContext->errstr;
//        redisClusterFree(m_pContext);
//        m_pContext = NULL;
    }

    return 0;
}


const char *CacheConn::GetPoolName()
{
    return m_pCachePool->GetPoolName();
}


string CacheConn::get(string key)
{
    string value;

    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return nullptr;
    }
    cout << " get 72" << endl;

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "GET %s", key.c_str());
    if (!reply)
    {
        cout << "redisClusterCommand failed: " << m_pContext->errstr << endl;
        LOG_ERROR << "redisClusterCommand failed: " << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return value;
    }
    cout << "get 83" << endl;

    if (reply->type == REDIS_REPLY_STRING)
    {
        cout << "get 87" << endl;
        cout << "reply value " << reply->str << "  len = " <<  reply->len << endl;
        value.append(reply->str, reply->len);

    }
    cout << " get " <<  key.c_str() <<" value = " << value;

    freeReplyObject(reply);
    return value;
}

string CacheConn::setex(string key, int timeout, string value)
{
    string ret_value;

    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return ret_value;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "SETEX %s %d %s", key.c_str(), timeout, value.c_str());
    LOG_INFO << "[redis cmd] SETEX " << key << " " << timeout << " " << value;

    if (nullptr == reply)
    {
        cout << "redisClusterCommand failed:" <<  m_pContext->errstr << endl;
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}

int CacheConn::setnx(string key, int time)
{
    int ret_value = -1;

    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return ret_value;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "SETNX %s %d", key.c_str(), time);
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }
    ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::expire(string key, int timeout)
{
    string ret_value;

    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return ret_value;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "EXPIRE %s %d", key.c_str(), timeout);
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}


string CacheConn::set(string key, string &value)
{
    string ret_value;

    if (Init())
    {
        LOG_ERROR << "Init() error";
        return ret_value;
    }
    LOG_INFO << "[redis cmd] SET " << key << " " << value;

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "SET %s %s", key.c_str(), value.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::getset(string key, string &value)
{
    string ret_value;

    if (Init())
    {
        return ret_value;
    }
    LOG_INFO << "[redis cmd] GETSET " << key << " " << value;

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "GETSET %s %s", key.c_str(), value.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::del(string key)
{
    string ret_value;

    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return ret_value;
    }
    LOG_INFO << "[redis cmd] del " << key ;

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "DEL %s", key.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    ret_value.append(reply->str, reply->len);
    freeReplyObject(reply);
    return ret_value;
}


bool CacheConn::mget(const vector<string> &keys, map<string, string> &ret_value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return false;
    }
    if (keys.empty())
    {
        return false;
    }

    string strKey;
    bool bFirst = true;
    for (vector<string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        if (bFirst)
        {
            bFirst = false;
            strKey = *it;
        }
        else
        {
            strKey += " " + *it;
        }
    }

    if (strKey.empty())
    {
        return false;
    }
    strKey = "MGET " + strKey;
    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, strKey.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" <<  m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return false;
    }
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; ++i)
        {
            redisReply *child_reply = reply->element[i];
            if (child_reply->type == REDIS_REPLY_STRING)
            {
                ret_value[keys[i]] = child_reply->str;
            }
        }
    }
    freeReplyObject(reply);
    return true;
}

bool CacheConn::isExists(string &key)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return false;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "EXISTS %s", key.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        return false;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    if (0 == ret_value)
    {
        return false;
    }
    else
    {
        return true;
    }
}

long CacheConn::hdel(string key, string field)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return 0;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "HDEL %s %s", key.c_str(), field.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed: "  << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return 0;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::hget(string key, string field)
{
    string ret_value;
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return ret_value;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "HGET %s %s", key.c_str(), field.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed: " << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    if (reply->type == REDIS_REPLY_STRING)
    {
        ret_value.append(reply->str, reply->len);
    }

    freeReplyObject(reply);
    return ret_value;
}

bool CacheConn::hgetAll(string key, map<string, string> &ret_value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return false;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "HGETALL %s", key.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return false;
    }

    if ((reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0))
    {
        for (size_t i = 0; i < reply->elements; i += 2)
        {
            redisReply *field_reply = reply->element[i];
            redisReply *value_reply = reply->element[i + 1];

            string field(field_reply->str, field_reply->len);
            string value(value_reply->str, value_reply->len);
            ret_value.insert(make_pair(field, value));
        }
    }

    freeReplyObject(reply);
    return true;
}

long CacheConn::hset(string key, string field, string value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "HSET %s %s %s", key.c_str(), field.c_str(),
                                                    value.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::hincrBy(string key, string field, long value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "HINCRBY %s %s %ld", key.c_str(), field.c_str(), value);
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::incrBy(string key, long value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "INCRBY %s %ld", key.c_str(), value);
    if (!reply)
    {
        LOG_ERROR << "redis Command failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

string CacheConn::hmset(string key, map<string, string> &hash)
{
    string ret_value;

    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return ret_value;
    }

    int argc = hash.size() * 2 + 2;
    const char **argv = new const char *[argc];
    if (!argv)
    {
        return ret_value;
    }

    argv[0] = "HMSET";
    argv[1] = key.c_str();
    int i = 2;
    for (map<string, string>::iterator it = hash.begin(); it != hash.end(); it++)
    {
        argv[i++] = it->first.c_str();
        argv[i++] = it->second.c_str();
    }

    redisReply *reply = (redisReply *) redisClusterCommandArgv(m_pContext, argc, argv, NULL);
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        delete[] argv;

        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return ret_value;
    }

    ret_value.append(reply->str, reply->len);

    delete[] argv;
    freeReplyObject(reply);
    return ret_value;
}

bool CacheConn::hmget(string key, list<string> &fields, list<string> &ret_value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return false;
    }

    int argc = fields.size() + 2;
    const char **argv = new const char *[argc];
    if (!argv)
    {
        return false;
    }

    argv[0] = "HMGET";
    argv[1] = key.c_str();
    int i = 2;
    for (list<string>::iterator it = fields.begin(); it != fields.end(); it++)
    {
        argv[i++] = it->c_str();
    }

    redisReply *reply = (redisReply *) redisClusterCommandArgv(m_pContext, argc, (const char **) argv, NULL);
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        delete[] argv;

        redisClusterFree(m_pContext);
        m_pContext = NULL;

        return false;
    }

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply *value_reply = reply->element[i];
            string value(value_reply->str, value_reply->len);
            ret_value.push_back(value);
        }
    }

    delete[] argv;
    freeReplyObject(reply);
    return true;
}

long CacheConn::incr(string key)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "INCR = %s" , key.c_str());
    if (!reply)
    {
        LOG_ERROR << "redis Command failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::decr(string key)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return (long )0;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "DECR %s", key.c_str());
    if (!reply)
    {
        LOG_ERROR << "redis Command failed " << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }
    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::lpush(string key, string value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "LPUSH %s %s", key.c_str(), value.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::rpush(string key, string value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "RPUSH %s %s", key.c_str(), value.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

long CacheConn::llen(string key)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return -1;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "LLEN %s", key.c_str());
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return -1;
    }

    long ret_value = reply->integer;
    freeReplyObject(reply);
    return ret_value;
}

bool CacheConn::lrange(string key, long start, long end, list<string> &ret_value)
{
    if (Init())
    {
        LOG_ERROR << "Init() error" ;
        return false;
    }

    redisReply *reply = (redisReply *) redisClusterCommand(m_pContext, "LRANGE %s %d %d", key.c_str(), start, end);
    if (!reply)
    {
        LOG_ERROR << "redisClusterCommand failed:" << m_pContext->errstr;
        redisClusterFree(m_pContext);
        m_pContext = NULL;
        return false;
    }

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < reply->elements; i++)
        {
            redisReply *value_reply = reply->element[i];
            string value(value_reply->str, value_reply->len);
            ret_value.push_back(value);
        }
    }

    freeReplyObject(reply);
    return true;
}


string CacheConn::getpb(string key, ::google::protobuf::Message &objPBValue)
{
    string value;
    value = get(key);
    int i ;
    for( i = 0; i < value.size();i++)
    {
        if(value[i] == '~')
        {
            value[i] = '\0';
        }
    }
    value[i] = '\0';
    if (!value.empty())
    {
        if (!objPBValue.ParseFromString(value))
        {
            cout  << "ParsePb failed" << endl;
            LOG_ERROR << "ParsePb failed";
            cout << " get strvalue(err) = " << objPBValue.DebugString() << " size = " << objPBValue.ByteSize() << endl;

            return nullptr;
        }

    }
    cout << " get strvalue(succ) = " << objPBValue.DebugString() << " size = " << objPBValue.ByteSize() << endl;

    return value;
}

string CacheConn::setpb(string key, int LifeTimeSec, const ::google::protobuf::Message &objPBValue)
{
    char value[2048];
    int byte_size = objPBValue.ByteSize();

    uint8_t* start = reinterpret_cast<uint8_t*>(value);
    uint8_t* end = objPBValue.SerializeWithCachedSizesToArray(start);
    if (end - start != byte_size)
    {
        LOG_ERROR << "pb Serialize failed ! ";
    }
    int i = 0;
    for( i = 0; i < byte_size;i++)
    {
        if(value[i] == '~')
        {
            LOG_ERROR << "replace error : '~' has exist";
        }
        if(value[i] == '\0')
        {
            value[i] = '~';
        }
    }
    value[i] = '\0';

    return setex(key, LifeTimeSec, value);
}

///////////////
CachePool::CachePool(const char *pool_name, const char *server_ip, int server_port, int db_num, int max_conn_cnt)
{
    m_pool_name = pool_name;
    m_server_ip = server_ip;
    m_server_port = server_port;
    m_db_num = db_num;
    m_max_conn_cnt = max_conn_cnt;
    m_cur_conn_cnt = MIN_CACHE_CONN_CNT;
}

CachePool::~CachePool()
{
    m_lock.Lock();
    for (list<CacheConn *>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++)
    {
        CacheConn *pConn = *it;
        delete pConn;
    }

    m_free_list.clear();
    m_cur_conn_cnt = 0;
    m_lock.Unlock();
}

int CachePool::Init()
{
    for (int i = 0; i < m_cur_conn_cnt; i++)
    {
        CacheConn *pConn = new CacheConn(this);
        if (pConn->Init())
        {
            delete pConn;
            return 1;
        }

        m_free_list.push_back(pConn);
    }

    //LOG_INFO << "cache pool: " << m_pool_name <<  "    list size: " <<  m_free_list.size();
    return 0;
}

CacheConn *CachePool::GetCacheConn()
{
    m_lock.Lock();

    while (m_free_list.empty())
    {
        if (m_cur_conn_cnt >= m_max_conn_cnt)
        {
            m_lock.Wait();
        }
        else
        {
            CacheConn *pCacheConn = new CacheConn(this);
            int ret = pCacheConn->Init();
            if (ret)
            {
                LOG_ERROR << "Init CacheConn failed";
                delete pCacheConn;
                m_lock.Unlock();
                return NULL;
            }
            else
            {
                m_free_list.push_back(pCacheConn);
                m_cur_conn_cnt++;
                LOG_INFO << "new cache connection:" << m_pool_name << "conn_cnt: %d" << m_cur_conn_cnt;
            }
        }
    }

    CacheConn *pConn = m_free_list.front();
    m_free_list.pop_front();

    m_lock.Unlock();

    return pConn;
}

void CachePool::RelCacheConn(CacheConn *pCacheConn)
{
    m_lock.Lock();

    list<CacheConn *>::iterator it = m_free_list.begin();
    for (; it != m_free_list.end(); it++)
    {
        if (*it == pCacheConn)
        {
            break;
        }
    }

    if (it == m_free_list.end())
    {
        m_free_list.push_back(pCacheConn);
    }

    m_lock.Signal();
    m_lock.Unlock();
}

///////////
CacheManager::CacheManager()
{

}

CacheManager::~CacheManager()
{

}

CacheManager *CacheManager::getInstance()
{
    if (!cache_manager)
    {
        cache_manager = new CacheManager();
        if (cache_manager->Init())
        {
            delete cache_manager;
            cache_manager = NULL;
        }
    }

    return cache_manager;
}
static vector<string> split(const string &str,const string &pattern)
{
    //const char* convert to char*
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL)
    {
        resultVec.push_back(string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }

    delete[] strc;

    return resultVec;
}

static std::vector<string>  vec;
int CacheManager::Init()
{
    //ConfigFileReader config_file("/root/IM2/server/src/dbproxyserver.conf");
    ConfigFileReader config_file("../conf/dbproxyserver.conf");

    char *cache_instances = config_file.GetConfigName("CacheInstances");
    if (!cache_instances)
    {
        LOG_ERROR << "not configure CacheIntance";
        return 1;
    }

    char host[64];
    char port[64];
    char db[64];
    char maxconncnt[64];
    vec = split(string(cache_instances),",");
    for (uint32_t i = 0; i < vec.size(); i++)
    {
        snprintf(host, 64, "%s_host", vec[i].c_str());
        snprintf(port, 64, "%s_port", vec[i].c_str());
        snprintf(db, 64, "%s_db", vec[i].c_str());
        snprintf(maxconncnt, 64, "%s_maxconncnt", vec[i].c_str());

        char *cache_host = config_file.GetConfigName(host);
        char *str_cache_port = config_file.GetConfigName(port);
        char *str_cache_db = config_file.GetConfigName(db);
        char *str_max_conn_cnt = config_file.GetConfigName(maxconncnt);
        if (!cache_host || !str_cache_port || !str_cache_db || !str_max_conn_cnt)
        {
            LOG_ERROR << "not configure cache instance: " << vec[i].c_str();
            return 2;
        }

        CachePool *pCachePool = new CachePool(vec[i].c_str(), cache_host, atoi(str_cache_port),
                                              atoi(str_cache_db), atoi(str_max_conn_cnt));
        if (pCachePool->Init())
        {
            LOG_ERROR << "Init cache pool failed";
            return 3;
        }
        m_cache_pool_map.insert(make_pair(vec[i], pCachePool));
    }

    return 0;
}

CacheConn *CacheManager::GetCacheConn(const char *pool_name)
{
    map<string, CachePool *>::iterator it = m_cache_pool_map.find(pool_name);
    if (it != m_cache_pool_map.end())
    {
        return it->second->GetCacheConn();
    }
    else
    {
        return NULL;
    }
}

void CacheManager::RelCacheConn(CacheConn *pCacheConn)
{
    if (!pCacheConn)
    {
        return;
    }

    map<string, CachePool *>::iterator it = m_cache_pool_map.find(pCacheConn->GetPoolName());
    if (it != m_cache_pool_map.end())
    {
        return it->second->RelCacheConn(pCacheConn);
    }
}
