//
// Created by xy on 2024-02-13.
//

#include <sys/epoll.h>
#include "Channel.h"

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop)
  , fd_(fd)
{}

Channel::~Channel() {
  // 在析构函数中，不要销毁loop_，也不能关闭fd_，因为这两个东西不属于Channel类
  // loop_ 是管理者，管理着  Channel，不会被 Channel 删除
  // fd_ 和 Channel 一 一 对应，Channel 就是 fd 相关信息的记录者，删除fd_ 也不归 Channel管
}

uint32_t Channel::getevents() {
  return events_;
}
uint32_t Channel::getrevents() {
  return revents_;
}
bool Channel::getinepoll() {
  return inepoll_;
}
int Channel::getfd() {
  return fd_;
}
void Channel::setinepoll(bool inepoll) {
  inepoll_ = inepoll;
}
void Channel::setrevents(uint32_t ev) {
  revents_ = ev;
}
void Channel::setreadCallback(Channel::EventCallback cb) {
  readCallback_ = cb;
}
void Channel::setwriteCallback(Channel::EventCallback cb) {
  writeCallback_ = cb;
}
void Channel::setcloseCallback(Channel::EventCallback cb) {
  closeCallback_ = cb;
}
void Channel::seterrorCallback(Channel::EventCallback cb) {
  errorCallback_ = cb;
}
void Channel::usset() {
  events_ = events_ | EPOLLET;
}
void Channel::enableReading() {
  events_ |= EPOLLIN;
  loop_->updateChannel(this);
}
void Channel::disableReading() {
  events_ &= ~EPOLLIN;
  loop_->updateChannel(this);
}
void Channel::enableWriting() {
  events_ |= EPOLLOUT;
  loop_->updateChannel(this);
}
void Channel::disableWriting() {
  events_ &= ~EPOLLOUT;
  loop_->updateChannel(this);
}
void Channel::disableAll() {
  events_ = 0;  // 表示不关注任何事件
  loop_->updateChannel(this);
}
void Channel::remove() {
  //disableAll();
  loop_->removeChannel(this);
}
void Channel::handleEvent() {
  if (revents_ & EPOLLRDHUP){
    closeCallback_();
  } else if (revents_ & (EPOLLIN | EPOLLPRI)){
    readCallback_();
  }else if(revents_ & EPOLLOUT){
    writeCallback_();
  }else{  // 其它事件，都视为错误
    errorCallback_();
  }
}



