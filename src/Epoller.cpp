//
// Created by xy on 2024-02-13.
//

#include "Epoller.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Epoller::Epoller() {
  epollfd_ = epoll_create(1);
  if (epollfd_ == -1) {
    perror("epoll_create failed \n");
    exit(-1);
  }
}

Epoller::~Epoller() {
  ::close(epollfd_);
}

//把 channel 添加到 红黑树上
//如果已经添加 就更新
void Epoller::updateChannel(Channel *ch) {
  epoll_event event{};
  event.data.ptr = ch;
  event.events = ch->getevents();

  if (ch->getinepoll()) {  //已经被添加
    int re = epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->getfd(), &event);
    if (re == -1) {
      perror("epoll_ctl --> EPOLL_CTL_MOD failed \n");
      exit(-1);
    }
  } else {
    int re = epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->getfd(), &event);
    if (re == -1) {
      perror("epoll_ctl --> EPOLL_CTL_ADD failed \n");
      exit(-1);
    }
    ch->setinepoll(true);
  }
}

void Epoller::removeChannel(Channel *ch) {
  if (ch->getinepoll()) {  // 在树上才考虑删除动作
    int re = epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->getfd(), nullptr);
    if (re == -1) {
      perror("epoll_ctl --> EPOLL_CTL_DEL failed \n");
      exit(-1);
    }
  }
}
std::vector<Channel *> Epoller::loop(int timeout) {
  std::vector<Channel *> channels;

  bzero(events_, sizeof(events_));
  int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
  if (infds < 0) { //出错
    if (errno != EINTR){
      printf("检测\n");

    }
    perror("epoll_wait failed \n");
    exit(-1);
  } else if (infds == 0) { // 超时
    return channels;  //返回空的channels
  } else {  //有事件发生
    for (int i = 0; i < infds; ++i) {
      Channel *ch = (Channel *) events_[i].data.ptr;
      ch->setrevents(events_[i].events);
      channels.push_back(ch);
    }
  }
  return channels;
}
