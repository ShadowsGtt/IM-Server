//
// Created by shadowgao on 2019/6/7.
//

#include "Client.h"
#include <algorithm>
#include <cstdlib>

void testRegister(string username);
void testLogin(string username);
void testSingleChat(string sender,string receiver);
void testGroupChat();
void createClients(int count,InetAddress serverAddr);
void testSendHongbao(string sender,string receiver);
void testRobHongbao();

int RAND(int n,int m);


// username -> hbkey
extern map<string,string > redPackets;

map<string,Client* > clients;

int RAND(int n,int m)//生成n到m之内的随机数
{
    srand(time(NULL)+rand()+CurrentThread::tid());
    return rand()%(m-n+1)+n;
}


void test(InetAddress serverAddr)
{
    cout << "initsize" << redPackets.size() << endl;
    createClients(20,serverAddr);
    cout << "clients size = " << clients.size() << endl;

    for(auto &username : clients)
    {
        testRegister(username.first);
        testLogin(username.first);
    }
    //sleep(3);
    map<string,Client* >::iterator it,itNext;

    for( it = clients.begin() ; it != clients.end() ; it++ )
    {

        if((++it) == clients.end())
            break;
        --it;
        string sender = it->first;
        string receiver = (++it)->first;
        --it;

        testSingleChat(sender,receiver);
        testSendHongbao(sender,receiver);
    }
    testGroupChat();
    //sleep(3);
    //testRobHongbao();

}

void testRegister(string username)
{
    Timestamp timestamp = Timestamp::now();
    //cout << "get time :" << timestamp.toFormattedString() << endl;
    IM::Register reg;
    reg.set_username(username);
    reg.set_password("pwd12345");
    reg.set_mobile("110");
    reg.set_nickname("Nick");
    reg.set_sex(0);
    reg.set_age(20);
    reg.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    reg.set_sign_info("sign_info");
    string email = username+"@gmail.com";
    reg.set_email(email);
    clients[username]->send(&reg);
}

void testLogin(string username)
{
    IM::Login login;
    login.set_username(username);
    login.set_password("pwd12345");
    login.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    clients[username]->send(&login);
    clients[username]->saveUserName(username);
}

void testSingleChat(string sender,string receiver)
{
    string message = "hello," + receiver + ",i am " + sender + "!";

    IM::SingleChat chat;
    chat.set_type(0);
    chat.set_sender(sender);
    chat.set_receiver(receiver);
    chat.set_message(message);
    chat.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    clients[sender]->send(&chat);
}

void testGroupChat()
{
    IM::GroupChat chat;
    chat.set_sender("IMUser_7");
    chat.set_message("hi,I am group member");
    chat.set_groupid("G2253188");
    chat.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    clients["IMUser_7"]->send(&chat);
    
}

void testSendHongbao(string sender,string receiver)
{
    cout << "testSendHongbao " << sender << " -> " << receiver << endl;
    double money = RAND(1,10000) / 100.00;
    int count = RAND(1,100);


    //单聊红包
    IM::SingleChat chat;
    chat.set_sender(sender);
    chat.set_receiver(receiver);
    chat.set_type(1);
    chat.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    IM::Hongbaoreq *hongbao = chat.mutable_hbreq();
    hongbao->set_sender(sender);
    hongbao->set_receiver(receiver);
    hongbao->set_money(money);
    hongbao->set_category(1);
    hongbao->set_count(1);
    hongbao->set_timestamp(Timestamp::now().microSecondsSinceEpoch());
    clients[sender]->send(&chat);
    //cout << "line 131" << cha

}

void testRobHongbao()
{
    map<string,Client* >::iterator it,itNext;

    cout << "map ---- size = " << redPackets.size() << endl;
    for(auto k : redPackets)
    {
        cout << "map " << k.first << "  -> " << k.second << endl;
    }
    for( it = clients.begin() ; it != clients.end() ; it++ )
    {

        if((++it) == clients.end())
            break;
        --it;
        string sender = it->first;
        string receiver = (++it)->first;
        --it;

        if(!redPackets[sender].empty())
        {
            IM::SingleChat chat;
            chat.set_sender(sender);
            chat.set_receiver(receiver);
            chat.set_timestamp(Timestamp::now().microSecondsSinceEpoch());
            chat.set_type(3);
            IM::ReqRobHongbao *req = chat.mutable_robhbreq();
            req->set_hongbaokey(redPackets[sender]);
            req->set_sender(sender);
            req->set_selfname(receiver);

            cout << receiver << " rob " << sender << "'s hongbao, key=" << redPackets[sender] << endl;
            cout << "line 158 -------" << chat.DebugString() << endl;
            clients[receiver]->send(&chat);
        }
    }
}

void createClients(int count,InetAddress serverAddr)
{
    for(int i = 0 ;i < count;i++)
    {
        string username = "IMUser_" + to_string(i);
        cout  << username << endl;
        Client *client = new Client(serverAddr);
        client->start();
        clients[username] = client;
    }
}



