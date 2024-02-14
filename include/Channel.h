
//
// Created by xy on 2024-02-13.
//

#ifndef NET__CHANNEL_H_
#define NET__CHANNEL_H_

#include <cstdint>
#include <functional>
#include "EventLoop.h"

class EventLoop;

class Channel {
 public:
  using EventCallback = std::function<void()>;

  Channel(EventLoop *loop, int fd);
  ~Channel();

  void usset();               //采用边缘触发
  void enableReading();       //注册读事件到epoll树上
  void disableReading();      //从epoll树上取消读事件
  void enableWriting();       //注册写事件epoll树上
  void disableWriting();     //从epoll树上取消写事件
  void disableAll();        //从epoll树上取消全部的事件

  void remove();  //从事件循环中删除Channel

  uint32_t getevents();
  int getfd();
  uint32_t getrevents();
  void setrevents(uint32_t ev);
  bool getinepoll();
  void setinepoll(bool inepoll);

  void handleEvent();   // 处理发生的事件（即执行设置的回调函数）

  //设置 事件发生后执行的回调函数
  void setreadCallback(EventCallback cb);
  void setwriteCallback(EventCallback cb);
  void setcloseCallback(EventCallback cb);
  void seterrorCallback(EventCallback cb);
 private:
  int fd_ = -1;             // 一个 Channel 对应一个  fd
  EventLoop *loop_;         // 事件循环 （一个 EventLoop 可以对应多个 Channel ；一个 Channel 只属于 一个 EventLoop）
  bool inepoll_ = false;     // 判断Channel 是否已经 添加到 epoll红黑树上
  uint32_t events_ = 0;     // fd 感兴趣的事件（即需要监视的事件）
  uint32_t revents_ = 0;    // fd 已经发生的事件

  //事件 对应的回调函数
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

#endif //NET__CHANNEL_H_
