
//
// Created by xy on 2024-02-13.
//

#ifndef NET__EVENTLOOP_H_
#define NET__EVENTLOOP_H_

#include <memory>
#include <atomic>
#include <functional>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "Connection.h"
#include "Channel.h"
#include "Epoller.h"
#include <queue>
#include <mutex>
#include <map>

class Connection;
class Epoller;

class EventLoop {
 public:
  using spConnection = std::shared_ptr<Connection>;

  EventLoop(bool mainloop, int timetvl = 10, int timeout = 20);
  ~EventLoop();

  void run();
  void stop();

  void wakeup();
  void handlewakeup();

  void handletimer();
  void setTimerCallback(std::function<void(int)> cb);
  void setEpollTimeoutCallback(std::function<void(EventLoop*)> cb);

  void updateChannel(Channel *ch);
  void removeChannel(Channel *ch);

  bool isInLoopThread();              //判断当前线程是否为事件循环线程，不是就去唤醒其他loop线程

  void queueInLoop(std::function<void()> fn);  //把任务添加到对列中

  void newconnection(spConnection conn);            // 把Connection对象保存在conns_中
 private:
  int timetvl_;                            // 闹钟时间间隔
  int timeout_;                           // 超时时间

  std::function<void(EventLoop*)> epollTimeoutCallback_;
  std::function<void(int)> timerCallback_;

  std::mutex taskmutex_;            // 保护 taskQueue_
  std::queue<std::function<void()>> taskQueue_;   // 存放 执行的回调函数
  std::mutex connmutex_;                  // 保护conns_
  std::map<int,spConnection> conns_;      // 存放连接

  int wakeupfd_;
  std::unique_ptr<Channel> wakeChannel_;
  int timerfd_;
  std::unique_ptr<Channel> timeChannel_;

  bool mainLoop_;                         // 判断是不是 主loop


  pid_t threadId_;                        // 事件循环 所在的线程 ID
  std::atomic_bool stop_;                 // 用于停止事件循环
  std::unique_ptr<Epoller> ep_;           // 每个事件循环对应 一个 epoll
};

#endif //NET__EVENTLOOP_H_
