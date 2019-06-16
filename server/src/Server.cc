#include "Server.h"
#include "ReadConfig.h"
#include "BaseBusiness.h"
#include <iostream>
#include <fcntl.h>

using namespace std;

Server *GServer;

AsyncLogging* Server::log_ = NULL;
//unordered_map<string,TcpConnectionPtr > Server::connMap;

void logOutput(const char *msg, int len)
{
    Server::log_->append(msg,len);
}

Server::Server(map<string, string>& config) : configMap_(config),
                   dispatcher_(std::bind(&Server::onUnknownMessage, this, _1, _2, _3)),
                   codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
{
    //创建异步日志系统
    log_ = new AsyncLogging("im",toByte(configMap_["logRollSie"]),std::stoi(configMap_["logFlushInterval"]));
    // 启动异步日志系统
    log_->start();

    //创建线程池
    threadPool_ = new ThreadPool("ThreadPool");
    threadPool_->setMaxQueueSize(1024);

    // 创建事件循环
    loop_ = new EventLoop();
    addr_.setIpPort(configMap_["host"], std::stoi(configMap_["port"]));
    server_ = new TcpServer(loop_,addr_,configMap_["serverName"]);

    // 设置日志输出
    Logger::setOutput(logOutput);

    server_->setThreadNum(std::stoi(configMap_["ioThreadNumber"]));

    /* 根据消息类型注册该类型消息回调 */
	dispatcher_.registerMessageCallback<IM::Login>(
		std::bind(&Server::Login, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<IM::Register>(
            std::bind(&Server::Register, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<IM::SingleChat>(
            std::bind(&Server::SingleChat, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<IM::ReqRobHongbao>(
            std::bind(&Server::RobHongbao, this, _1, _2, _3));

    dispatcher_.registerMessageCallback<IM::GroupChat>(
            std::bind(&Server::GroupChat, this, _1, _2, _3));
    dispatcher_.registerMessageCallback<IM::Hongbaoreq>(
            std::bind(&Server::SendHongbao, this, _1, _2, _3));


    /* 连接状态发生改变时的回调函数 */
    server_->setConnectionCallback(
            std::bind(&Server::onConnection, this, _1));

    /* 接受到新消息时的回调函数 */
    server_->setMessageCallback(
            std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
}

//void Server::addTask(const Task &task)
//{
//    threadPool_->addTask(task);
//}

void Server::start()
{
    threadPool_->start(std::stoi(configMap_["tpNum"]));
    LOG_INFO << "thread pool start running";
    server_->start();
    LOG_INFO << "server start running";
	loop_->loop();
}

void Server::onConnection(const TcpConnectionPtr& conn)
{
	if(conn->connected())
	{
		LOG_INFO << conn->peerAddress().toIpPort() << "  connected! ";
	}
	else
	{	
		LOG_INFO << conn->peerAddress().toIpPort() << "  disconnected! ";
        //delUsernameConn(conn);
	}
}
void Server::Login(const net::TcpConnectionPtr& conn,
               const LoginPtr& message,
               Timestamp timestamp)
{
    string username(message->username());
    saveUsernameConn(username,conn);

    Response answer;
    //验证用户名密码 并缓存
    int isOK = Business::Login(*message,answer);
    codec_.send(conn, answer);

    IM::SingleChat chat;
    if( 0 == isOK )
    {
        if( 0 == Business::sendCacheMessage(username,chat) )
        {
            LOG_INFO << "[CACHE-MESSAGE-SEND] all message has send to " << username;
        }

    }


}



void Server::Register(const net::TcpConnectionPtr &conn,
        const RegisterPtr& message, Timestamp timestamp)
{
    cout << endl;
    cout << "-----------------------protobuf--start-------------------------------" << endl;
    cout << "protobuf message type :" << message->GetTypeName() <<endl
         << message->DebugString();
    cout << "------------------------protobuf---end-------------------------------" << endl;

    Response answer;
    Business::Register(*message, answer);

    codec_.send(conn, answer);
}
void Server::SingleChat(const net::TcpConnectionPtr &conn,
                      const SingleChatPtr& message, Timestamp timestamp)
{
    cout << endl;
    cout << "-----------------------protobuf--start-------------------------------" << endl;
    cout << "protobuf message type :" << message->GetTypeName() <<endl
         << message->DebugString();
    cout << "------------------------protobuf---end-------------------------------" << endl;
    TcpConnectionPtr& connPtr = getConnByUsername(message->receiver());
    if(connPtr)
    {
        IM::SingleChat chat;
        IM::HongbaoResp *hbrsp;
        IM::RespRobHongbao *robhbrsp;
        int type = message->type();
        switch (type)
        {
            case 0:
                codec_.send(connPtr,*message);
                LOG_INFO << "[SINGAL-CHAT-SEND] " << message->sender() << " -> " \
                         << message->receiver() << " \"" << message->message() << "\"";
                LOG_INFO << "[SINGAL-CHAT-SEND " << message->DebugString();
                cout << "[SINGAL-CHAT-SEND] " << message->DebugString()<< endl;

                break;
            case 1:
                LOG_INFO << "[SEND-HONGBAO] " << message->sender() << " -> " << message->receiver();
                hbrsp = chat.mutable_hbrsp();
                Business::SendHongbao((*message).hbreq(),*hbrsp);
                chat.set_type(2);
                codec_.send(connPtr, chat);
                LOG_INFO << "[HONGBAO-RESPONSE] " << chat.hbrsp().description();
                //cout << "[HONGBAO-RESPONSE] " << chat.DebugString() << endl;
                break;
            case 3:
                LOG_INFO << "[ROB-HONGBAO-REQ] " << message->sender() << " -> " << message->receiver();
                robhbrsp = chat.mutable_robhbrsp();
                Business::RobHongbao(message->robhbreq(),*robhbrsp);
                chat.set_type(4);
                codec_.send(connPtr, chat);
                LOG_INFO << "[ROB-HONGBAO-RESPONSE] " << chat.DebugString();
                cout << "[ROB-HONGBAO-RESPONSE] " << chat.DebugString()<< endl;

                break;
            default:
                break;
        }


    }
    else{
        if( 0 == Business::cacheMessage(message->receiver(),*message) )
        {
            LOG_INFO << "[SINGAL-CHAT-CACHED] " << message->sender() << " -> " << message->receiver() << " message has cached";
        }
    }


}
void Server::GroupChat(const net::TcpConnectionPtr &conn,
        const GroupChatPtr& message, Timestamp timestamp)
{
    cout << endl;
    cout << "-----------------------protobuf--start-------------------------------" << endl;
    cout << "protobuf message type :" << message->GetTypeName() <<endl
         << message->DebugString();
    cout << "------------------------protobuf---end-------------------------------" << endl;
//
//    Response answer;
//    answer.set_result(true);
//    answer.set_description("GroupChat success");
//
//    codec_.send(conn, answer);

}

void Server::SendHongbao(const net::TcpConnectionPtr &conn,
        const HongbaoPtr& message, Timestamp timestamp)
{
    cout << endl;
    cout << "-----------------------protobuf--start-------------------------------" << endl;
    cout << "protobuf message type :" << message->GetTypeName() <<endl
         << message->DebugString();
    cout << "------------------------protobuf---end-------------------------------" << endl;


    IM::HongbaoResp answer;

    Business::SendHongbao(*message,answer);


    codec_.send(conn, answer);

}
void Server::RobHongbao(const net::TcpConnectionPtr &conn,
                        const RobHongbaoPtr& message, Timestamp timestamp)
{
    cout << endl;
    cout << "-----------------------protobuf--start-------------------------------" << endl;
    cout << "protobuf message type :" << message->GetTypeName() << endl
         << message->DebugString();
    cout << "------------------------protobuf---end-------------------------------" << endl;

    IM::RespRobHongbao answer;

    Business::RobHongbao(*message, answer);

    codec_.send(conn, answer);
}

//
//void Client::HongbaoResponse(const net::TcpConnectionPtr& conn,
//                             const HongbaoRespPtr& message,
//                             Timestamp timestamp)
//{
//    cout << endl;
//    cout << "--------------------protobuf----response----------------" << endl;
//    cout << "protobuf message type : "<< message->GetTypeName() <<endl
//         << message->DebugString();
//    cout << "--------------------protobuf----response----------------" << endl;
//
//}
//
