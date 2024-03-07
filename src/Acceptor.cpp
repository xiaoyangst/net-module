//
// Created by xy on 2024-02-13.
//

#include "Acceptor.h"
Acceptor::Acceptor(EventLoop *loop,const std::string &ip, const uint16_t port)
  : loop_(loop)
  , servSock_(createNonblocking())
  , acceptChannel_(loop_,servSock_.getfd())
{
  InetAddress servaddr(ip,port);             // 服务端的地址和协议。
  servSock_.setreuseaddr(true);
  servSock_.settcpnodelay(true);
  servSock_.setreuseport(true);
  servSock_.setkeepalive(true);
  servSock_.bind(servaddr);
  servSock_.listen();

  //从此刻开始 Acceptor的工作就是 ： 持续监听新来的连接 并为该连接相应的回调函数，且把对应的读事件注册到epoll树上

  acceptChannel_.setreadCallback(std::bind(&Acceptor::newConnection, this));    //简单一句话 绑定回调函数newConnection
  acceptChannel_.enableReading(); //把 acceptChannel_ 交给 epoll监视 其读事件
}
Acceptor::~Acceptor() {

}


void Acceptor::newConnection() {
  InetAddress clientAddr;

  std::unique_ptr<Socket> clientSock(new Socket(servSock_.accept(clientAddr)));   //调用accept方法得到 conn
  clientSock->setIpPort(clientAddr.getIP(),clientAddr.getPort());

  NewConnCb_(std::move(clientSock));    // 回调TcpServer::newconnection()
}
void Acceptor::setNewConnCb(Acceptor::NewConnCb cb) {
  NewConnCb_ = cb;
}
