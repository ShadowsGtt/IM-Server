#ifndef _SERVER_H
#define _SERVER_H

#include "codec.h"
#include "dispatcher.h"
#include "IM.pb.h"
#include "Logging.h"
#include "Mutex.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "AsyncLogging.h"
#include "ThreadPool.h"
#include <unordered_map>

using namespace net;
using namespace IM;

typedef std::shared_ptr <IM::Login> LoginPtr;
typedef std::shared_ptr <IM::Register> RegisterPtr;
typedef std::shared_ptr <IM::SingleChat> SingleChatPtr;
typedef std::shared_ptr <IM::GroupChat> GroupChatPtr;
typedef std::shared_ptr <IM::Hongbaoreq> HongbaoPtr;
typedef std::shared_ptr <IM::ReqRobHongbao> RobHongbaoPtr;


class Server : noncopyable {
public:
    typedef std::function<void()> Task;

    Server(map<string, string>& config);

    void onConnection(const TcpConnectionPtr &conn);

    void onUnknownMessage(const TcpConnectionPtr &conn,
                          const MessagePtr &message,
                          Timestamp) {
        LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
        //conn->shutdown();
    }

    void start();


    TimerId runAt(Timestamp time, TimerCallback cb)
    {
        return loop_->runAt(time,cb);
    }

    TimerId runAfter(double delay, TimerCallback cb)
    {
        return loop_->runAfter(delay,cb);
    }

    TimerId runEvery(double interval, TimerCallback cb)
    {
        return loop_->runEvery(interval,cb);
    }

    //void addTask(const Task &task);

    void Login(const net::TcpConnectionPtr &conn,
               const LoginPtr &message,
               Timestamp timestamp);

    void Register(const net::TcpConnectionPtr &conn,
                  const RegisterPtr &message,
                  Timestamp timestamp);

    void SingleChat(const net::TcpConnectionPtr &conn,
                    const SingleChatPtr &message,
                    Timestamp timestamp);

    void GroupChat(const net::TcpConnectionPtr &conn,
                   const GroupChatPtr &message,
                   Timestamp timestamp);

    void SendHongbao(const net::TcpConnectionPtr &conn,
                    const HongbaoPtr& message,
                    Timestamp timestamp);
    void RobHongbao(const net::TcpConnectionPtr &conn,
                    const RobHongbaoPtr& message,
                    Timestamp timestamp);


    void send(string& username, const google::protobuf::Message& message)
    {
        TcpConnectionPtr conn = getConnByUsername(username);
        if(conn){
            codec_.send(conn,message);
        }else{

        }
    }

    TcpConnectionPtr& getConnByUsername(string username) { return  connMap_[username]; }
    void saveUsernameConn(string username, const TcpConnectionPtr& conn) {connMap_[username] = conn;}
    void delUsernameConn(const TcpConnectionPtr& conn)
    {
        for(auto it =  connMap_.begin(); it != connMap_.end(); it++)
        {
            if(conn == it->second)
            {
                LOG_INFO << "[LOGOUT] username = " << it->first << "log out server";
                connMap_.erase(it);
            }
        }
    }
    static AsyncLogging *log_;                     // 异步日志

    //static unordered_map<string,TcpConnectionPtr > connMap;
private:
    ThreadPool *threadPool_;                // 线程池
    map <string, string> configMap_;        // 配置
    EventLoop *loop_;                       // 主线程循环
    InetAddress addr_;                      // 服务器地址
    TcpServer *server_;                     // tcp server
    ProtobufDispatcher dispatcher_;         // 协议包分发
    ProtobufCodec codec_;                   // 编解码
    unordered_map<string,TcpConnectionPtr > connMap_;
};

#endif
