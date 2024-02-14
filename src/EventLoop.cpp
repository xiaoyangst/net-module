//
// Created by xy on 2024-02-13.
//

#include "EventLoop.h"

EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
  :ep_(new Epoller)
{

}
EventLoop::~EventLoop() {

}

void EventLoop::updateChannel(Channel *ch) {

}



void EventLoop::wakeup() {

}

void EventLoop::run() {
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

void EventLoop::stop() {
  stop_ = true;
  wakeup();
}

