
//
// Created by xy on 2024-02-13.
//

#ifndef NET__ACCEPTOR_H_
#define NET__ACCEPTOR_H_

#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"


class Acceptor {
 public:
  using NewConnCb = std::function<void(std::unique_ptr<Socket>)>;
  
  Acceptor(EventLoop* loop,const std::string &ip, const uint16_t port);
  ~Acceptor();
  
  void newConnection();   // 处理客户端 连接的请求
  void setNewConnCb(NewConnCb cb);    // 设置与客户端建立新连接的 回调函数

 private:
  EventLoop* loop_;
  Socket servSock_;
  Channel acceptChannel_;
  NewConnCb NewConnCb_;
};

#endif //NET__ACCEPTOR_H_
