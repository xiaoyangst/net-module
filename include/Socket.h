
//
// Created by xy on 2024-02-13.
//

#ifndef NET__SOCKET_H_
#define NET__SOCKET_H_

#include <string>
#include "InetAddress.h"

class Socket {
 public:
  Socket(int fd);
  ~Socket();

  int getfd();
  std::string getIP();
  uint16_t getPort();
  void setIpPort(const std::string &ip,uint16_t port);

  void setreuseaddr(bool on);         // 设置SO_REUSEADDR选项，true-打开，false-关闭。
  void setreuseport(bool on);         // 设置SO_REUSEPORT选项。
  void settcpnodelay(bool on);        // 设置TCP_NODELAY选项。
  void setkeepalive(bool on);         // 设置SO_KEEPALIVE选项。

  void bind(InetAddress servaddr);
  void listen(int n = 128);
  int accept(InetAddress& clientaddr);
 private:
  const int fd_;
  std::string ip_;
  uint16_t port_;
};

#endif //NET__SOCKET_H_
