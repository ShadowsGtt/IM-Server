#include <iostream>
#include "../server/src/LogFile.h"
#include "../server/src/Logging.h"

using namespace std;



LogFile file("./log/filename",1024,false);


void setOutPut(const char *msg ,int len )
{
    file.append(msg,len);
}

int main(void)
{
    Logger::setLogLevel(Logger::TRACE);

    /* 这段日志会输出到stdout */
    LOG_TRACE << "This TRACE";
    LOG_DEBUG << "This DEBUG";
    LOG_INFO << "This is INFO";
    LOG_WARN << "This is WARN";
    LOG_ERROR << "This is ERROR";
    errno = 1;
    LOG_SYSERR << "This is SYSERR";

    Logger::setOutput(::setOutPut);
    /* 这段日志会输出到文件中 */
    LOG_TRACE << "This TRACE";
    LOG_DEBUG << "This DEBUG";
    LOG_INFO << "This is INFO";
    LOG_WARN << "This is WARN";
    LOG_ERROR << "This is ERROR";
    errno = 1;
    LOG_SYSERR << "This is SYSERR";
    return 0;
}
