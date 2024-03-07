//
// Created by xy on 2024-02-13.
//

#include <cstring>
#include <iostream>
#include "Connection.h"

Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientSock)
    : loop_(loop),
      clientSock_(std::move(clientSock)),
      disConnect_(false),
      clientChannel_(new Channel(loop_, clientSock_->getfd()))
      {
  // 为新客户端连接准备读事件，并添加到epoll中
  clientChannel_->setreadCallback(std::bind(&Connection::onmessageCallback, this));
  clientChannel_->setcloseCallback(std::bind(&Connection::closeCallback, this));
  clientChannel_->seterrorCallback(std::bind(&Connection::errorCallback, this));
  clientChannel_->setwriteCallback(std::bind(&Connection::writeCallback, this));
  clientChannel_->usset();  //边缘触发(只通知一次)
  clientChannel_->enableReading();  // 让epoll_wait()监视clientchannel的读事件
}
Connection::~Connection() {

}
int Connection::getCfd() const {
  return clientSock_->getfd();
}
std::string Connection::getCip() const {

  return clientSock_->getIP();
}
uint16_t Connection::getCport() const {
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
bool Connection::timeout(time_t now, int val) {
  return now - lastTime_.toInt() > val;
}

//处理对端发来的消息
void Connection::onmessageCallback() {
  char buffer[1024];

  while (true) { // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕
    bzero(&buffer, sizeof(buffer));

    ssize_t nread = read(getCfd(), buffer, sizeof(buffer));    //读取客户端发送的数据

    if (nread > 0) { //成功读取到数据
      inputBuffer_.append(buffer, nread);
    } else if (nread == -1 && errno == EINTR) { // 读取数据的时候被信号中断，继续读取
      continue;
    } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 全部的数据已读取完毕
      std::string message;
      while (true) { // 从接收缓冲区中拆分出客户端的请求消息
        if (!inputBuffer_.pickMessage(message)) break;
        lastTime_ = Timestamp::now(); //更新时间，表明连接非空闲状态

        onmessagecallback_(shared_from_this(), message);
      }
      break;
    } else if (nread == 0) { //客户端断开连接
      closeCallback();
      break;
    }
  }
}
void Connection::closeCallback() {
  disConnect_ = true;
  clientChannel_->remove();
  closecallback_(shared_from_this());
}
void Connection::errorCallback() {
  disConnect_ = true;
  clientChannel_->remove();
  errorcallback_(shared_from_this());
}

// 处理写事件的回调函数
void Connection::writeCallback() {
  int writen = ::send(getCfd(),outputBuffer_.data(),outputBuffer_.size(),0);
  if (writen < 0){
    outputBuffer_.erase(0,writen);
  }
  if (outputBuffer_.size() == 0){  //缓冲区中没有数据了
    clientChannel_->disableWriting();
    sendcompletecallback_(shared_from_this());
  }
}

//发送数据皆调用此方法
void Connection::send(const char *data, size_t size) {
  if (disConnect_){
    printf("客户端已经断开\n");
    return;
  }

  if (loop_->isInLoopThread()){ //当前线程为事件循环线程
    sendInloop(data,size);
  }else{  // 交给其他 loop去处理
    loop_->queueInLoop(std::bind(&Connection::sendInloop, this,data,size));
  }
}

// 当前loop线程可用的发送数据的方法（用send方法间接调用，而不是直接调用）
void Connection::sendInloop(const char *data, size_t size) {
  outputBuffer_.appendSep(data,size);  // 把需要发送的数据保存到Connection的发送缓冲区中
  clientChannel_->enableWriting();  //注册写事件
}
