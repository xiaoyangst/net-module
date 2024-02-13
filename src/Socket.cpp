//
// Created by xy on 2024-02-13.
//

#include <unistd.h>
#include <netinet/tcp.h>
#include "Socket.h"

// 创建一个非阻塞的socket。
int createnonblocking()
{
  // 创建服务端用于监听的listenfd。
  int listenfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
  if (listenfd < 0)
  {
    printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno); exit(-1);
  }
  return listenfd;
}

Socket::Socket(int fd) : fd_(fd) {

}
Socket::~Socket() {
  ::close(fd_);
}
int Socket::getfd() {
  return fd_;
}
std::string Socket::getIP() {
  return ip_;
}
uint16_t Socket::getPort() {
  return port_;
}
void Socket::setIpPort(const std::string &ip, uint16_t port) {
  ip_ = ip;
  port_ = port;
}
void Socket::setreuseaddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}
void Socket::setreuseport(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}
void Socket::settcpnodelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}
void Socket::setkeepalive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
void Socket::bind(InetAddress servaddr) {
  if (::bind(fd_, servaddr.getAddr(), sizeof(sockaddr)) < 0) {
    perror("bind failed !");
    close(fd_);
    exit(-1);
  }

  setIpPort(servaddr.getIP(),servaddr.getPort());
}
void Socket::listen(int n) {
  if (::listen(fd_,n) != 0){
    perror("listen failed !");
    close(fd_);
    exit(-1);
  }
}
int Socket::accept(InetAddress &clientaddr) {
  sockaddr_in peeraddr;
  socklen_t socklen = sizeof(peeraddr);

  int connfd = accept4(fd_,(sockaddr*)&peeraddr,&socklen,SOCK_NONBLOCK);

  clientaddr.setAddr(peeraddr);

  return connfd;
}






