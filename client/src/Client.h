#ifndef _CLIENT_H
#define _CLIENT_H

#include "../../server/src/dispatcher.h"
#include "../../server/src/CountDownLatch.h"
#include "../../server/src/codec.h"
#include "../../server/src/Thread.h"
#include "../../server/src/IM.pb.h"

#include "../../server/src/Logging.h"
#include "../../server/src/Mutex.h"
#include "../../server/src/EventLoop.h"
#include "TcpClient.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace IM;
using namespace net;

typedef std::shared_ptr <IM::Response> ResponsePtr;
typedef std::shared_ptr <IM::HongbaoResp> HongbaoRespPtr;
typedef std::shared_ptr <IM::RespRobHongbao> RobHongbaoRespPtr;
typedef std::shared_ptr <IM::SingleChat> SingleChatPtr;
typedef std::shared_ptr <IM::GroupChat> GroupChatPtr;
//google::protobuf::Message* messageToSends;

class Client : noncopyable {
public:

    //Client(EventLoop* loop , InetAddress& serverAddr);
    Client(InetAddress serverAddr);

    void start();

    void runLoop();

    void onConnection(const TcpConnectionPtr &conn);

    void onUnknownMessage(const TcpConnectionPtr &,
                          const MessagePtr &message,
                          Timestamp);

    void onResponse(const net::TcpConnectionPtr &,
                    const ResponsePtr &message,
                    Timestamp);

    void HongbaoResponse(const net::TcpConnectionPtr &,
                         const HongbaoRespPtr &message,
                         Timestamp);

    void RobHongbaoResponse(const net::TcpConnectionPtr &,
                            const RobHongbaoRespPtr &message,
                            Timestamp);

    void SingleChat(const net::TcpConnectionPtr &,
                    const SingleChatPtr &message,
                    Timestamp);

    void GroupChat(const net::TcpConnectionPtr &,
                   const GroupChatPtr &message,
                   Timestamp);

    void send(google::protobuf::Message *);

    void saveUserName(string username) { username_ = username; }
    string getUserName() { return username_; }
private:
    string username_;
    InetAddress servAddr_;
    std::shared_ptr <EventLoop> loop_;
    std::shared_ptr <TcpClient> tcpClient_;
    std::shared_ptr <ProtobufDispatcher> dispatcher_;
    std::shared_ptr <ProtobufCodec> codec_;
    CountDownLatch latch_;
    CountDownLatch latch2_;
    std::shared_ptr <Thread> thread_;
    TcpConnectionPtr conn_;
};

#endif

