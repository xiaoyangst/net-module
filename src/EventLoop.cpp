//
// Created by xy on 2024-02-13.
//

#include <cstring>
#include <sys/syscall.h>
#include <bits/syscall.h>
#include "EventLoop.h"


//TODO 待了解的函数 timerfd_create
int createtimerfd(int sec=30)
{
  int tfd=timerfd_create(CLOCK_MONOTONIC,TFD_CLOEXEC|TFD_NONBLOCK);   // 创建timerfd。
  struct itimerspec timeout;                                // 定时时间的数据结构。
  memset(&timeout,0,sizeof(struct itimerspec));
  timeout.it_value.tv_sec = sec;                             // 定时时间，固定为5，方便测试。
  timeout.it_value.tv_nsec = 0;
  timerfd_settime(tfd,0,&timeout,0);
  return tfd;
}

EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
  : ep_(new Epoller)
  , mainLoop_(mainloop)
  , timeout_(timeout)
  , stop_(false)
  , wakeupfd_(eventfd(0,EFD_NONBLOCK))    // 创建非阻塞的 eventfd
  , wakeChannel_(new Channel(this,wakeupfd_))
  , timerfd_(createtimerfd(timeout_))
  , timeChannel_(new Channel(this,timerfd_))
{
  wakeChannel_->setreadCallback(std::bind(&EventLoop::handlewakeup, this));
  wakeChannel_->enableReading();

  timeChannel_->setreadCallback(std::bind(&EventLoop::handletimer, this));
  timeChannel_->enableReading();
}
EventLoop::~EventLoop() {

}

//唤醒
void EventLoop::wakeup() {
  uint64_t val = 1;
  write(wakeupfd_,&val, sizeof(val));
}

//被唤醒，然后去执行 任务队列taskQueue_中的回调函数
void EventLoop::handlewakeup() {
  uint64_t val = 1;
  read(wakeupfd_,&val,sizeof(val));

  std::lock_guard<std::mutex> lock_guard(taskmutex_);
  while (!taskQueue_.empty()){
    std::function<void()> fn = std::move(taskQueue_.front());
    taskQueue_.pop();
    fn();
  }
}

void EventLoop::newconnection(EventLoop::spConnection conn) {
  std::lock_guard<std::mutex> lock_guard(connmutex_);
  conns_[conn->getCfd()] = conn;
}

void EventLoop::setTimerCallback(std::function<void(int)> cb) {
  timerCallback_ = cb;
}

void EventLoop::setEpollTimeoutCallback(std::function<void(EventLoop *)> cb) {
  epollTimeoutCallback_ = cb;
}


void EventLoop::run() {
  threadId_ = syscall(SYS_gettid);  // 获取事件循环所在线程的id

  while (!stop_){
    std::vector<Channel *> channels = ep_->loop(10 * 1000);

    if (channels.empty()){  //没有事件发生
      epollTimeoutCallback_(this);
    }else{
      for (auto &ch : channels) {
        ch->handleEvent();    //执行事件绑定的回调
      }
    }
  }
}

//TODO 有待理解

// 闹钟响时执行的函数
void EventLoop::handletimer() {
  struct itimerspec timeout;
  memset(&timeout,0,sizeof(struct itimerspec));
  timeout.it_value.tv_sec = timetvl_;
  timerfd_settime(timerfd_,0,&timeout,0);

  if (mainLoop_){

  }else{
    time_t now = time(0);
    for(auto conn:conns_){
      if (conn.second->timeout(now,timeout_)){
        {
          std::lock_guard<std::mutex> lock_guard(connmutex_);
          conns_.erase(conn.first);   // 从EventLoop的map中删除超时的conn
        }
        timerCallback_(conn.first);   // 从TcpServer的map中删除超时的conn
      }
    }
  }
}

void EventLoop::stop() {
  stop_ = true;
  wakeup(); // 唤醒事件循环，如果没有这行代码，事件循环将在下次闹钟响时或epoll_wait()超时时才会停下来
}

void EventLoop::updateChannel(Channel *ch) {
  ep_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel *ch) {
  ep_->removeChannel(ch);
}

bool EventLoop::isInLoopThread() {
  return threadId_ == syscall(SYS_gettid);
}

void EventLoop::queueInLoop(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock_guard(taskmutex_);
    taskQueue_.push(fn);
  }

  wakeup(); // 唤醒事件循环
}



















