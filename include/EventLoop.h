
//
// Created by xy on 2024-02-13.
//

#ifndef NET__EVENTLOOP_H_
#define NET__EVENTLOOP_H_

#include <memory>
#include <atomic>
#include <functional>
#include "Channel.h"
#include "Epoller.h"

class EventLoop {
 public:
  EventLoop(bool mainloop, int timetvl = 10, int timeout = 20);
  ~EventLoop();

  void run();
  void stop();

  void wakeup();

  void updateChannel(Channel *ch);
  void removeChannel(Channel *ch);

  bool isInLoopThread();              //判断当前线程是否为事件循环线程，不是就去唤醒其他loop线程

  void queueInLoop(std::function<void()> fn);  //把任务添加到对列中

  std::function<void(EventLoop*)> epollTimeoutCallback_;

 private:
  int timetvl_;


  pid_t threadId_;                        // 事件循环 所在的线程 ID
  std::atomic_bool stop_;
  std::unique_ptr<Epoller> ep_;           // 每个事件循环对应 一个 epoll
};

#endif //NET__EVENTLOOP_H_
