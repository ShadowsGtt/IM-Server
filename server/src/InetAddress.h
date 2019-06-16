#ifndef _INETADDRESS_H
#define _INETADDRESS_H


#include <netinet/in.h>
#include <string>

using namespace std;


namespace net
{

namespace sockets
{
 const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
}


/// This is an POD interface class.
class InetAddress
{
public:

  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

  /* 构造函数  传入ip 和 port */
  InetAddress(string ip, uint16_t port);

  void setIpPort(string ip, uint16_t port);

  /* 构造函数  传入struct sockaddr_in结构体 */
  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr){}

  /* 返回地址族 用到的都是IPV4 AF_INET */
  sa_family_t family() const { return addr_.sin_family; }

  /* 以string形式返回IP地址  */
  /* eg: 0.0.0.0 */
  string toIp() const;

  /* 以string形式返回IP 及 port */
  /* eg: 0.0.0.0:8888 */
  string toIpPort() const;

  /* 返回port */
  uint16_t toPort() const;


  /* 返回网络端(大端)IP */
  uint32_t ipNetEndian() const;
  
  /* 返回网络端(大端)端口 */
  uint16_t portNetEndian() const { return addr_.sin_port; }
  
  /* 以struct sockaddr形式返回地址 */
  const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr_); }

   /* 设置sockaddr_in地址 */
   void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }


 private:
    struct sockaddr_in addr_;

};

}

#endif  // _INETADDRESS_H

