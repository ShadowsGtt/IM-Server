
#ifndef _SOCKETSOPS_H
#define _SOCKETSOPS_H

#include <arpa/inet.h>


namespace net
{
namespace sockets
{


/* 创建指定地址族的sockfd */
int createNonblockingOrDie(sa_family_t family);

/* 封装一些sockfd的操作 */
int  connect(int sockfd, const struct sockaddr* addr);
void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
//int  accept(int sockfd, struct sockaddr_in6* addr);
int  accept(int sockfd, struct sockaddr_in* addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);

/* 从addr结构中获得ip和port写入buf中 */
void toIpPort(char* buf, size_t size, const struct sockaddr* addr);

/* 从addr结构中获得ip写入buf中 */
void toIp(char* buf, size_t size, const struct sockaddr* addr);

/* 将IP和port填入addr结构中 */
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);


/* 获得socket错误,返回错误码 */
int getSocketError(int sockfd);


/* 类型转换 */
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
/* 新添加 */
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

/* 从 sockfd 中得到本端信息 */
struct sockaddr_in getLocalAddr(int sockfd);

/* 从 sockfd 中得到对端信息 */
struct sockaddr_in getPeerAddr(int sockfd);


bool isSelfConnect(int sockfd);

}
}

#endif  // MUDUO_NET_SOCKETSOPS_H
