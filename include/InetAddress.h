
//
// Created by xy on 2024-02-13.
//


#ifndef NET__INETADDRESS_H_
#define NET__INETADDRESS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

class InetAddress {
 public:
  InetAddress();
  InetAddress(const std::string &ip, uint16_t port);   //监听fd（listenfd） 使用 该构造函数
  InetAddress(const sockaddr_in addr);           //客户端连接上来的fd（connfd） 使用 该构造函数

  char *getIP();
  uint16_t getPort();
  sockaddr *getAddr();
  void setAddr(sockaddr_in clientAddr);
 private:
  struct sockaddr_in Addr_;
};

#endif //NET__INETADDRESS_H_
