//
// Created by xy on 2024-02-13.
//

#include "Connection.h"

Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientSock)
  : loop_(loop)
  , clientSock_(std::move(clientSock))
  , disConnect_(false)
  , clientChannel_(new Channel(loop_,clientSock_->getfd()))
{
  // 为新客户端连接准备读事件，并添加到epoll中
  clientChannel_->setreadCallback(std::bind(&Connection::onmessageCallback, this));
  clientChannel_->setcloseCallback(std::bind(&Connection::closeCallback, this));
  clientChannel_->seterrorCallback(std::bind(&Connection::errorCallback, this));
  clientChannel_->setwriteCallback(std::bind(&Connection::writeCallback, this));
  clientChannel_->usset();
  clientChannel_->enableReading();
}
Connection::~Connection() {

}
int Connection::getCfd() const{
  return clientSock_->getfd();
}
std::string Connection::getCip() const{

  return clientSock_->getIP();
}
uint16_t Connection::getCport() const{
  return clientSock_->getPort();
}
void Connection::setOnmessageCallback(std::function<void(spConnection, std::string & )> cb) {
  onmessagecallback_ = cb;
}
void Connection::setCloseCallback(std::function<void(spConnection)> cb) {
  closecallback_ = cb;
}
void Connection::setErrorCallback(std::function<void(spConnection)> cb) {
  errorcallback_ = cb;
}
void Connection::setWriteCallback(std::function<void(spConnection)> cb) {
  sendcompletecallback_ = cb;
}
