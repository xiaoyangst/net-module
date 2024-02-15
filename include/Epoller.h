
//
// Created by xy on 2024-02-13.
//

#ifndef NET__EPOLLER_H_
#define NET__EPOLLER_H_

#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"

class Channel;

class Epoller {
 public:
  Epoller();
  ~Epoller();

  void updateChannel(Channel *ch);  // 封装 EPOLL_CTL_MOD EPOLL_CTL_ADD
  void removeChannel(Channel *ch);  // 封装 EPOLL_CTL_DEL

  std::vector<Channel*> loop(int timeout = -1); // 封装 epoll_wait 监听fd上发生的事件 并返回
 private:
  static const int MaxEvents = 100;   // epoll_wait 返回事件数组大小
  int epollfd_ = -1;
  epoll_event events_[MaxEvents];     // epoll_wait 检测到发生的事件总和
};

#endif //NET__EPOLLER_H_
