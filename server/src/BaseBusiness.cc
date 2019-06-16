//
// Created by lonnyliu(刘卓) on 2019-05-19.
//

#include <string>

#include "google/protobuf/message.h"
#include "BaseBusiness.h"
#include "MysqlLinkPool.h"
#include "IM.pb.h"
#include "IMComm.h"
#include "CachePool.h"
#include "Logging.h"
#include "RedisLock.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include "Timestamp.h"
#include "../proto/IM.pb.h"
#include "Server.h"
#include <functional>

extern Server *GServer;

using namespace IM;
using namespace std;

namespace Business
{

    void StopRobHongbao(string key, string sender)
    {
        int hashkey = DJBHash(sender);
        int redisid = hashkey % 3 + 1;
        int ret = 0;
        string redisname = "Master" + to_string(redisid);

        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn = pCacheManager->GetCacheConn(redisname.c_str());

        IM::SendHongbao hongbao;
        pCacheConn->getpb(key, hongbao);
        pCacheConn->del(key);

        string roblog;
        hongbao.SerializeToString(&roblog);
        replaceFromZero(roblog,hongbao.ByteSize());

        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();
        if( !ml )
        {
            LOG_ERROR << "get mysql connection failed!";
            return;
        }
        char sql[1024];
        snprintf(sql,sizeof(sql),"update Hongbao set leftmoney=%lf, leftcount = %d,roblog= '%s'  where hbkey = '%s'",
                hongbao.leftmoney(),hongbao.leftcount(), roblog.c_str(),key.c_str());
        ret = ml->Execute(sql);

        LOG_INFO << "[HONGBAO] sql = " << sql;

        if(0 != hongbao.leftcount())
        {
            LOG_INFO << "[HONGBAO-RETURN] username = " << sender << " return money = " << hongbao.leftmoney();
            snprintf(sql, sizeof(sql),"update IMUser set money=money+%lf where name = '%s' ", hongbao.leftmoney(), hongbao.sender().c_str());
            ret = ml->Execute(sql);
        }


        //TODO 计算运气王

        pCacheManager->RelCacheConn(pCacheConn);
        linkPool->ReleaseConnection(ml);

    }

    int32_t Register(const IM::Register &info_register, IM::Response &response)
    {
        bool registerResult = false;
        string registerInfo = "";
        char sSql[1024] = {0};
        double money = 0.0;
        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();


        do{
            if (0 != CheckDuplication(info_register))
            {
                registerResult = false;
                registerResult = "username repeated!";
                LOG_INFO << "[REGISTER-FAILED] username=\"" << info_register.username() << "\" repeated" ;
                break;
            }
            money = 10000.0;

            snprintf(sSql, sizeof(sSql),"insert into IMUser(sex,age,name,nick,password,phone,email,created,sign_info,money) values(%d, %d,'%s','%s','%s','%s','%s',%lld,'%s',%lf)", \
            info_register.sex(), \
            info_register.age(),\
            info_register.username().c_str(), \
            info_register.nickname().c_str(), \
            info_register.password().c_str(), \
            info_register.mobile().c_str(), \
            info_register.email().c_str(), \
            info_register.timestamp(),\
            info_register.sign_info().c_str(),
            money);
            LOG_INFO << "[REGISTER] sql = " << sSql ;
            if( 0 == ml->Execute(sSql))
            {
                registerResult = true;
                registerResult = "register success!";
                LOG_INFO << "[REGISTER-SUCCESS] username=\"" << info_register.username() << "\" write MySQL success" ;
            } else {
                response.set_result(false);
                response.set_description("server error!");
                LOG_ERROR << "[REGISTER-FAILED] username=\"" << info_register.username() << "\"  write MySQL failed" ;
            }
        }while(0);

        linkPool->ReleaseConnection(ml);
        response.set_result(registerResult);
        response.set_description(registerInfo.c_str());

        return registerResult ? 0 : -1;
    }

    // return 0 not repeated
    int32_t CheckDuplication(const IM::Register &info_register)
    {
        int ret = 0;
        char sql[1024] = {0};

        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();
        do{
            if(!ml)
            {
                ret = -1;
                LOG_ERROR << "mysql get connection failed!" << sql ;
                break;
            }
            snprintf(sql,sizeof(sql),"select * from IMUser where name = '%s'",info_register.username().c_str());
            if( 0 != ml->Query(sql) )
            {
                ret = -1;
                LOG_ERROR << "[SQL-FAILED] sql = " << sql ;
                break;
            }
            if(ml->GetRecordCount() > 0)
            {
                ret = -1;
            }
        }while (0);
        linkPool->ReleaseConnection(ml);

        return ret;
    }

    int32_t sendCacheMessage(string username, google::protobuf::Message& message)
    {
        int hashkey = 0;
        int redisid = 0;
        int ret = 0;
        string strKey = "";
        string redisname = "";
        string value = "";
        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn;

        do
        {
            hashkey = DJBHash(username);
            redisid = hashkey % 3 + 1;
            redisname = "Master" + to_string(redisid);

            pCacheConn =  pCacheManager->GetCacheConn(redisname.c_str());
            if (nullptr == pCacheConn)
            {
                LOG_ERROR << "[REDIS-FAILED] get cache connection failed,redisname= " << redisname ;
                ret = -1;
                break;
            }else{
                LOG_INFO << "[REDIS-SUCCESS] get cache connection success,redisname= " << redisname ;
            }
            strKey = username + "_message";
            list<string> msgs;
            if( !pCacheConn->lrange(strKey,0,-1,msgs) )
            {
                ret = -1;
            }
            LOG_INFO << "[list size] " << msgs.size() << " === " << strKey;
            for(auto& c : msgs)
            {
                IM::SingleChat sg;
                replaceToZero(c);
                //if( !message.ParseFromString(c) )
                if( !sg.ParseFromString(c) )
                {
                    LOG_ERROR << "cache message err , SerializeToString failed" ;
                    ret = -1;
                    break;
                }
                GServer->send(username,sg);
//                message.clear_receiver();
//                message.clear_sender();
//                message.clear_message();
//                message.clear_timestamp();

                LOG_INFO << "[DEBUG] "<< sg.DebugString();
                //LOG_INFO << "[CACHE-MESSAGE-SEND] message from "<< sc.sender() <<" has send to " <<  sc.receiver();
            }
        }while (0);


        pCacheManager->RelCacheConn(pCacheConn);

        return ret;
    }
    //缓存离线消息
    int32_t cacheMessage(string username,const google::protobuf::Message& message)
    {
        //int expiretime  = 86400 * 2;
        int hashkey = 0;
        int redisid = 0;
        int ret = 0;
        string strKey = "";
        string redisname = "";
        string value = "";
        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn;

        do
        {
            hashkey = DJBHash(username);
            redisid = hashkey % 3 + 1;
            redisname = "Master" + to_string(redisid);

            pCacheConn =  pCacheManager->GetCacheConn(redisname.c_str());

            if (nullptr == pCacheConn)
            {
                LOG_ERROR << "[REDIS-FAILED] get cache connection failed,redisname= " << redisname ;
                ret = -1;
                break;
            }else{
                LOG_INFO << "[REDIS-SUCCESS] get cache connection success,redisname= " << redisname ;
            }
            strKey = username + "_message";
            if( !message.SerializeToString(&value) )
            {
                LOG_ERROR << "cache message err , SerializeToString failed" ;
                ret = -1;
                break;
            }
            replaceFromZero(value,value.size());
            if( pCacheConn->lpush(strKey,value) > 0 )
            {
                LOG_INFO << "[redis cmd]" << "LPUSH " << strKey << value;
                ret = 0;
            }
        }while (0);


        pCacheManager->RelCacheConn(pCacheConn);

        return ret;
    }

    int32_t Login(const IM::Login &login, IM::Response &response)
    {
        bool loginResult = false;
        string loginInfo = "";
        char sql[1024] = {0};
        string strKey = "";
        int expiretime = 0;
        int hashkey = 0;
        int redisid = 0;
        string redisname = "";
        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();
        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn = nullptr;

        do{
            if(!ml || !pCacheManager)
            {
                LOG_ERROR << "[SQL-FAILED] sql=" << sql ;
                loginResult = false;
                loginInfo = "server error";
                break;
            }
            snprintf(sql, sizeof(sql), "select * from IMUser where name = '%s' and password = '%s'", \
                  login.username().c_str(), \
                  login.password().c_str());
            LOG_INFO << "[LOGIN] sql = " << sql;
            if( 0 != ml->Query(sql))
            {
                LOG_ERROR << "[SQL-FAILED] sql=" << sql ;
                loginResult = false;
                loginInfo = "server error";
                break;
            }
            if(0 == ml->GetRecordCount())
            {
                loginResult = false;
                loginInfo = "Login failed! username or password not matched!";
                LOG_INFO << "[LOGIN-FAILED] username = " << login.username() << " login failed,username or password not matched!";
                break;
            }
            loginResult = true;
            loginInfo = "Login success!";
            LOG_INFO << "[LOGIN-SUCCESS] username = " << login.username() << " login success";

            hashkey = DJBHash(login.username());
            redisid = hashkey % 3 + 1;
            redisname = "Master" + to_string(redisid);
            pCacheConn = pCacheManager->GetCacheConn(redisname.c_str());

            if (nullptr == pCacheConn)
            {
                LOG_ERROR << "[REDIS-FAILED] get cache connection failed,redisname= " << redisname ;
                break;
            }else{
                LOG_INFO << "[REDIS-SUCCESS] get cache connection success,redisname= " << redisname ;
            }
            strKey = login.username() + "_session";
            expiretime = 86400 * 2;

            IM::Session session;
            IM::UserInfo *user = session.mutable_userinfo();
            user->set_id(atoi(ml->GetField(0,"id")));
            user->set_sex(atoi(ml->GetField(0,"sex")));
            user->set_age(atoi(ml->GetField(0,"age")));
            user->set_name(ml->GetField(0,"name"));
            user->set_nick(ml->GetField(0,"nick"));
            user->set_phone(ml->GetField(0,"passwd"));
            user->set_email(ml->GetField(0,"email"));
            user->set_createtime(atoi(ml->GetField(0,"createtime")));
            user->set_updatetime(atoi(ml->GetField(0,"updatetime")));
            user->set_sign_info(ml->GetField(0,"sign_info"));
            user->set_money(atof(ml->GetField(0,"money")));

            session.set_islogin(true);
            session.set_hashkey(hashkey);
            session.set_redisname(redisname);
            pCacheConn->setpb(strKey, expiretime, session);
        }while (0);

        response.set_result(loginResult);
        response.set_description(loginInfo);

        pCacheManager->RelCacheConn(pCacheConn);
        linkPool->ReleaseConnection(ml);

        return loginResult ? 0 : -1;
    }


    int32_t GetFriendInfo(const IM::GetFriendInfo &reqinfo, IM::FriendInfo &respinfo)
    {
        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();

        int ret = 0;
        int hashkey = DJBHash(reqinfo.sender());
        int redisid = hashkey % 3 + 1;
        string redisname = "Master" + to_string(redisid);


        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn = pCacheManager->GetCacheConn(redisname.c_str());
        if (nullptr == pCacheConn)
        {
            LOG_ERROR << "get pCacheConn failed";
            return 0;
        }

        string strKey = reqinfo.sender() + "_session";

        IM::UserInfo userinfo;
        pCacheConn->getpb(strKey,userinfo);

        int id = userinfo.id();

        char sql[1024];
        snprintf(sql,sizeof(sql),"select * from IMRelationShip where ");

        pCacheManager->RelCacheConn(pCacheConn);
        linkPool->ReleaseConnection(ml);

        return 0;
    }


    int32_t SendHongbao(const IM::Hongbaoreq &req,IM::HongbaoResp &resp)
    {
        IM::SendHongbao hongbao;
        hongbao.set_sender(req.sender());
        hongbao.set_money(req.money());
        hongbao.set_leftmoney(req.money());
        hongbao.set_leftcount(req.count());
        hongbao.set_count(req.count());
        hongbao.set_receiver(req.receiver());
        hongbao.set_category(req.category());
        hongbao.set_timestamp(req.timestamp());

        int ret = 0;
        int hashkey = DJBHash(hongbao.sender());
        int redisid = hashkey % 3 + 1;
        string redisname = "Master" + to_string(redisid);

        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn = pCacheManager->GetCacheConn(redisname.c_str());
        if (nullptr == pCacheConn)
        {
            LOG_ERROR << "get pCacheConn failed";
            return -1;
        }

        string strKey = hongbao.sender() + "_session";
        int expiretime = 86400 * 2;
        IM::Session session;

        pCacheConn->getpb(strKey,session);

        //check mysql moneny
        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();
        if(!ml)
        {
            LOG_ERROR << "get mysql connection failed!";
            cout <<  "get mysql connection failed!";
            return -1;
        }

        char sql[1024];
        const double &money = session.userinfo().money();

        if(money-hongbao.money() < 0.000001)
        {
            resp.set_isok(false);
            resp.set_description("money not enough");
            LOG_INFO << "[HONGBAO] username = " <<  hongbao.sender() << \
                " send failed,money not enough,money = "<< hongbao.money() << "account money = " << money;
            return -1;
        }

        //sub user money
        snprintf(sql,sizeof(sql),"update IMUser set money=money-%lf where name = '%s'" ,hongbao.money(),session.userinfo().name().c_str());
        LOG_INFO << "Sub user money sql = " << sql;
        ret = ml->Execute(sql);

        (session.mutable_userinfo())->set_money(money - hongbao.money());

        //缓存暂存此次红包的信息
        uint32_t curTime = time(NULL);
        //往redis写入记录
        strKey = hongbao.sender() + "_" + to_string(curTime)+"_HBcount";
        LOG_INFO << "[REDIS] Hongbao key = " << strKey;
        pCacheConn->setpb(strKey,90000,hongbao);

        //记录用户的发送红包的记录
        snprintf(sql,sizeof(sql),"insert into Hongbao(hbkey,sender,money,category,count,receiver,time) values('%s','%s',%lf,%d,%d,'%s',%lld)",strKey.c_str(),hongbao.sender().c_str(), hongbao.money(), hongbao.category(), hongbao.count(), hongbao.receiver().c_str(),hongbao.timestamp());
        LOG_INFO << "[HONGBAO] sql = " << sql;

        ret = ml->Execute(sql);
        GServer->runAfter(6, std::bind(Business::StopRobHongbao, strKey, hongbao.sender()));

        resp.set_isok(true);
        resp.set_sender(hongbao.sender());
        resp.set_hongbaokey(strKey);
        resp.set_description("send ok");
        LOG_INFO << "[HONGBAO] " <<  hongbao.sender()<< " -> " << hongbao.receiver() << " send success,money = " << hongbao.money();

        pCacheManager->RelCacheConn(pCacheConn);
        linkPool->ReleaseConnection(ml);

        return 0;
    }



    int getRandomMoney(const int &remainMoney, const int &left_MoneyPacksize)//获取随机金额红包
    {
        if(left_MoneyPacksize == 0)
        {
            LOG_INFO << "[HONGBAO] redpacket is zero ";
            return 0;
        }

        if(remainMoney/left_MoneyPacksize == 1)
        {
            return remainMoney/left_MoneyPacksize;
        }
        int money;
        if(left_MoneyPacksize == 1)
        {
            money =   remainMoney;
            return money;
        }
        money = remainMoney*RAND(1,199)/100/left_MoneyPacksize;

        return money;
    }


    int32_t RobHongbao(const IM::ReqRobHongbao &req, IM::RespRobHongbao &resp)
    {
        int ret;

        const string &strKey  = req.hongbaokey();
        cout << "rob hongbao key=" <<strKey << endl;
        if(strKey.size() == 0)
        {
            LOG_ERROR << "rob hongbao key is NULL";
            resp.set_hongbaokey(strKey);
            resp.set_sender(req.sender());
            resp.set_isok(false);
            resp.set_description("hongbao key is NULL");
            return -1;
        }

        int hashkey = DJBHash(req.sender());
        int redisid = hashkey % 3 + 1;
        string redisname = "Master" + to_string(redisid);

        MysqlLinkPool *linkPool = MysqlLinkPool::GetInstance();
        Comm::Mysql *ml = linkPool->GetConnection();

        CacheManager *pCacheManager = CacheManager::getInstance();
        CacheConn *pCacheConn = pCacheManager->GetCacheConn(redisname.c_str());

        RedisLock redisLock(pCacheConn, redisname, strKey, 50000);
        if(redisLock.lock())
        {
            IM::SendHongbao hongbao;
            string retstr = pCacheConn->getpb(strKey, hongbao);
            if (retstr.empty())
            {
                resp.set_hongbaokey(strKey);
                resp.set_sender(req.sender());
                resp.set_isok(false);
                resp.set_description("sorry redpacket expired");
                redisLock.unlock();
                LOG_INFO << "[Hongbao] redis get pb empty Hbkey = " << strKey;
                return 0;
            }

            for(auto temp : hongbao.robitem())
            {
                if(req.selfname() == temp.name())
                {
                    resp.set_hongbaokey(strKey);
                    resp.set_sender(req.sender());
                    resp.set_isok(false);
                    resp.set_description("you have robed");
                    redisLock.unlock();
                    LOG_INFO << "[Hongbao] redis get pb empty Hbkey = " << strKey;
                    return 0;
                }
            }

            if(hongbao.leftcount() == 0)
            {
                resp.set_hongbaokey(strKey);
                resp.set_sender(req.sender());
                resp.set_isok(false);
                resp.set_description("sorry The red packets are gone");
                redisLock.unlock();
                LOG_INFO << "[Hongbao] redis get pb empty Hbkey = " << strKey;
                return 0;
            }

            int left_count = 0;
            //没有抢过
            const double &leftmoney = hongbao.leftmoney();
            const int &leftcount = hongbao.leftcount();
            int getmoney = getRandomMoney(static_cast<int>(leftmoney*100), leftcount);
            if(0 != leftcount)
            {
                left_count = leftcount - 1;
            }

            hongbao.set_leftmoney(leftmoney - static_cast<double>(getmoney/100.00));
            hongbao.set_leftcount(left_count);

            IM::RobLog *roblog = hongbao.add_robitem();
            roblog->set_name(req.selfname());
            roblog->set_money(static_cast<double>(getmoney/100.00));
            int64_t Time = time(NULL);
            roblog->set_time(Time);
            int expiretime = 86400;

            pCacheConn->setpb(strKey, expiretime, hongbao);


            LOG_INFO << "[Hongbao] rob item --name = " << req.selfname() << " money = " << static_cast<double>(getmoney/100.00) << "   time =  " << Time;

            char sql[1024];
            //add user money
            snprintf(sql,sizeof(sql),"update IMUser set money=money+%lf where name = '%s' ", static_cast<double>(getmoney/100.00), req.selfname().c_str());
            ret = ml->Execute(sql);
            LOG_INFO << "[Hongbao] add user money sql = " << sql ;
            redisLock.unlock();

            resp.set_hongbaokey(strKey);
            resp.set_sender(req.sender());
            resp.set_robmoney(static_cast<double>(getmoney/100.00));
            resp.set_totalcount(hongbao.count());
            resp.set_leftcount(left_count);
            resp.set_isok(true);
            resp.set_description("congratulation");
        }


        pCacheManager->RelCacheConn(pCacheConn);
        linkPool->ReleaseConnection(ml);

        return 0;
    }

    int RAND(int n,int m)//生成n到m之内的随机数
    {
        srand(time(NULL));
        return rand()%(m-n+1)+n;
    }

};
