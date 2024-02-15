//
// Created by xy on 2024-02-13.
//

#include "InetAddress.h"
InetAddress::InetAddress() {

}

InetAddress::InetAddress(const sockaddr_in addr)
    : Addr_(addr) {}

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
  Addr_.sin_family = AF_INET;
  Addr_.sin_port = htons(port);
  Addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

char *InetAddress::getIP() {
  return inet_ntoa(Addr_.sin_addr);
}

uint16_t InetAddress::getPort() {
  return ntohs(Addr_.sin_port);
}
sockaddr *InetAddress::getAddr() {
  return (sockaddr *) &Addr_;
}
// TODO 有待分析
void InetAddress::setAddr(sockaddr_in clientAddr) {
  Addr_ = clientAddr;
}



