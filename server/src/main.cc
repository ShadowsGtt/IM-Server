#include <iostream>

#include "TcpServer.h"
#include "Logging.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include "Server.h"
#include <signal.h>


using namespace std;
using namespace net;

extern Server *GServer;

void setDaemon();
map <string, string> readConfig(const char *);

int main(void)
{
    try
    {
        //读取配置文件
        auto configMap_ = readConfig("../conf/server.conf");
        if(configMap_["daemon"] == "yes")
            setDaemon();
        signal(SIGABRT,SIG_IGN);
        Server *server = new Server(configMap_);
        GServer = server;
        server->start();
    }
    catch(exception &e)
    {
        LOG_ERROR << e.what();
    }

    return 0;
}
