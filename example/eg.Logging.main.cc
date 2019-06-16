#include <iostream>
#include "../server/src/Logging.h"

using namespace std;


int main(void)
{
    Logger::setLogLevel(Logger::TRACE);

    LOG_TRACE << "This Trace";
    LOG_DEBUG << "This Debug";
    LOG_INFO << "This is info";
    LOG_WARN << "This is warn";
    LOG_ERROR << "This is ERROR";
    errno = 1;
    LOG_SYSERR << "This is SYSERR";
    LOG_FATAL << "This is FATAL";
    LOG_SYSFATAL << "This is SYSFATAL";
    return 0;
}
