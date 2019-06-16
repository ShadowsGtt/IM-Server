#include "Client.h"
#include <unistd.h>
//#include "IM.pb.h"
#include "../../server/src/EventLoopThread.h"

using namespace IM;

void test(InetAddress serverAddr);
 map<string,string > redPackets;

//Client *GClient ;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s host_ip port [q|e]\n", argv[0]);
        return -1;
    }
    //LOG_INFO << "pid = " << getpid();
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    const char *ip = argv[1];
    InetAddress serverAddr(ip, port);
    test(serverAddr);
//    Client *client = new Client(serverAddr);
//    GClient  = client;
//    client->start();


//    testRegister();
//    testLogin();
//    testSingleChat();
    //testHongbao();


//    Login login;
//    login.set_username("IMUser0");
//    login.set_password("pwd12345");
//    login.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
//    client.send(&login);
//


//    SingleChat singleChat;
//    singleChat.set_receiver("IMUser0");
//    singleChat.set_sender("IMUser1");
//    singleChat.set_message("hello,在吗?");
//    singleChat.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
//    client.send(&singleChat);
//
//    GroupChat groupChat;
//    groupChat.set_sender("IMUser0");
//    groupChat.set_groupid(99);
//    groupChat.set_message("大家好，我是新群成员,Jack");
//    groupChat.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
//    client.send(&groupChat);



    while(1)
    {
    }
}


