#include "Client.h"
#include <utility>

extern map<string,string > redPackets;


Client::Client(InetAddress serverAddr) : servAddr_(serverAddr), 
            loop_(NULL), tcpClient_(NULL),
            dispatcher_(NULL), codec_(NULL),
            latch_(1),
            latch2_(1)
{}

void Client::runLoop()
{
    loop_ = std::make_shared<EventLoop>();
    tcpClient_ = std::make_shared<TcpClient>(loop_.get(), servAddr_, "Client");
    dispatcher_ = std::make_shared<ProtobufDispatcher>(std::bind(&Client::onUnknownMessage, this, _1, _2, _3) );
    codec_ = std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage, dispatcher_.get(), _1, _2, _3));

    //收到服务器对应消息的回调 
    dispatcher_->registerMessageCallback<IM::Response>(
        std::bind(&Client::onResponse, this, _1, _2, _3));
    dispatcher_->registerMessageCallback<IM::HongbaoResp>(
            std::bind(&Client::HongbaoResponse, this, _1, _2, _3));
    dispatcher_->registerMessageCallback<IM::RespRobHongbao>(
            std::bind(&Client::RobHongbaoResponse, this, _1, _2, _3));


    //注册单聊、群聊回调
    dispatcher_->registerMessageCallback<IM::SingleChat>(
            std::bind(&Client::SingleChat, this, _1, _2, _3));
    dispatcher_->registerMessageCallback<IM::GroupChat>(
            std::bind(&Client::GroupChat, this, _1, _2, _3));



    tcpClient_->setConnectionCallback(
        std::bind(&Client::onConnection, this, _1));
    tcpClient_->setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, codec_.get(), _1, _2, _3));
    latch_.countDown();
    loop_->loop();
}

void Client::start()
{
    thread_ = std::make_shared<Thread>(std::bind(&Client::runLoop,this),"loop Thread");
    thread_->start();
    
    /* 等待新线程运行 */
    latch_.wait();

    /* 尝试与服务器建立连接 */
    tcpClient_->connect();
    
    /* 等待连接建立 */
    latch2_.wait();
}

void Client::send(google::protobuf::Message *mesg)
{
    if(conn_)
    {
        if(conn_->connected())
        {
            codec_->send(conn_,*mesg);
        }
        else
        {

        }
    }
    else
    {
        cout << "发送失败...(连接不存在)" << endl;
    }
}

void Client::onConnection(const TcpConnectionPtr &conn)
{
	if(conn->connected())
    {
        conn_ = conn;
        latch2_.countDown();
        LOG_INFO << " connect to " << conn->peerAddress().toIpPort() ;
    }
    else 
    {
		LOG_INFO << " disconnect with " << conn_->peerAddress().toIpPort() ;
		conn_ = NULL;    
	}
}

void Client::onUnknownMessage(const TcpConnectionPtr&,
                        const MessagePtr& message,
                        Timestamp timestamp)
{
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
}


/* 客户端收到的回应 */
void Client::onResponse(const net::TcpConnectionPtr&,
                const ResponsePtr& message,
                Timestamp timestamp)
{
    //LOG_INFO << message->description() ;
	cout << endl;
	cout << "--------------------protobuf----response----------------" << endl;
    cout << "protobuf message type : "<< message->GetTypeName() <<endl
         << message->DebugString();
  	cout << "--------------------protobuf----response----------------" << endl;
}

void Client::HongbaoResponse(const net::TcpConnectionPtr& conn,
                             const HongbaoRespPtr& message,
                             Timestamp)
{
//    cout << endl;
//    cout << "--------------------protobuf----response----------------" << endl;
//    cout << "protobuf message type : "<< message->GetTypeName() <<endl
//         << message->DebugString();
//    cout << "--------------------protobuf----response----------------" << endl;
//
//    LOG_INFO << "\""  << message->sender() << "\"发送了一个红包" ;

    // 去抢红包
    IM::ReqRobHongbao req;
    req.set_hongbaokey(message->hongbaokey());
    req.set_sender(message->sender());
    req.set_selfname(username_);
    codec_->send(conn,req);
}

void Client::RobHongbaoResponse(const net::TcpConnectionPtr& conn,
                                const RobHongbaoRespPtr& message,
                                Timestamp)
{
    if(message->isok())
    {
        LOG_INFO << "[抢红包提醒] " << username_ << "抢到了 \"" << message->sender() \
                 << "\"的红包  金额:" << message->robmoney() << "元";

    } else{
        LOG_INFO << "[抢红包提醒] " << username_ << "未抢到 \"" << message->sender() \
                 << "\"的红包,原因:" << message->description();
    }

}


void Client::SingleChat(const net::TcpConnectionPtr& conn,
                        const SingleChatPtr& message,
                        Timestamp timestamp)
{
    int type = message->type();
    cout << message->DebugString() << endl;
    string st = message->hbrsp().isok() ? "成功":"失败";
    string name = (message->hbrsp()).sender();
    string hbkey = (message->hbrsp()).hongbaokey();
    string receiver = message->receiver();
    switch (type)
    {
        case 0:
            LOG_INFO << "[Friend Message] " << message->sender() \
             << ":\"" << message->message() << "\"[" \
             << Timestamp(message->timestamp()).toFormattedString() << "]";
            break;
        case 2:

            //LOG_INFO << "hbresp" << (message->hbrsp()).sender() << "发送了一个红包,状态:" <<st ;
            if((message->hbrsp()).isok())
            {
                LOG_INFO << "client:" << username_ << "  \""  << (message->hbrsp()).sender() \
                         << "\"发送了一个红包 key=" << (message->hbrsp()).hongbaokey();
                redPackets[(message->hbrsp()).sender()] = (message->hbrsp()).hongbaokey();

                IM::SingleChat cc;
                cc.set_type(3);
                cc.set_sender(username_);
                cc.set_receiver(receiver);
                IM::ReqRobHongbao *req = cc.mutable_robhbreq();
                req->set_hongbaokey(hbkey);
                req->set_sender(name);
                req->set_selfname(username_);
                codec_->send(conn,cc);
                //redPackets.insert(make_pair(name,hbkey));
               // cout << "rbsize = " << redPackets.size() << endl;
            }
            break;
        case 4:
            if(message->robhbrsp().isok())
            {
                LOG_INFO << "[抢红包提醒] " << username_ << "抢到了 \"" << message->robhbrsp().sender() \
                 << "\"的红包  金额:" << message->robhbrsp().robmoney() << "元";

            } else{
                LOG_INFO << "[抢红包提醒] " << username_ << "未抢到 \"" << message->robhbrsp().sender() \
                 << "\"的红包,原因:" << message->robhbrsp().description();
            }
            break;
        default:
            break;
    }

}
void Client::GroupChat(const net::TcpConnectionPtr&,
                        const GroupChatPtr& message,
                        Timestamp timestamp)
{
    LOG_INFO << "[Group Message <-  " << message->groupid() << "] "<< message->sender() \
             << ":\"" << message->message() << "\"[" \
             << Timestamp(message->timestamp()).toFormattedString();
}
