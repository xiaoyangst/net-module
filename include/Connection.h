
//
// Created by xy on 2024-02-13.
//

#ifndef NET__CONNECTION_H_
#define NET__CONNECTION_H_

#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Timestamp.h"
#include "Buffer.h"
#include <memory>

class EventLoop;
class Channel;

class Connection : public std::enable_shared_from_this<Connection>{
 public:
  using spConnection = std::shared_ptr<Connection>;

  Connection(EventLoop *loop,std::unique_ptr<Socket> clientSock);
  ~Connection();

  int getCfd() const;             //获得客户端client fd
  std::string getCip() const;     //获得客户端client ip
  uint16_t getCport() const;      //获得客户端client port

  void onmessageCallback();   // 处理 对端发送过来的消息
  void closeCallback();       // TCP连接断开的回调函数
  void errorCallback();       // TCP连接错误的回调函数
  void writeCallback();       // 处理 写事件的回调函数

  //设置 回调
  void setOnmessageCallback(std::function<void(spConnection,std::string&)> cb);
  void setCloseCallback(std::function<void(spConnection)> cb);
  void setErrorCallback(std::function<void(spConnection)> cb);
  void setWriteCallback(std::function<void(spConnection)> cb);

  void send(const char* data,size_t size);    // 发送数据，不管在任何线程中，都是调用此函数发送数据。
  void sendInloop(const char* data,size_t size);    // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行

  bool timeout(time_t now,int val);   //判断连接是否超时（空闲太久）

 private:
  EventLoop *loop_;
  std::unique_ptr<Socket> clientSock_;    //与 客户端通信的socket
  std::unique_ptr<Channel> clientChannel_;

  //每个 连接Connection 都有属于自己的 输入缓冲区inputBuffer_ 和输出缓冲区outputBuffer_
  Buffer* inputBuffer_;
  Buffer* outputBuffer_;

  std::atomic_bool disConnect_;   //判断连接是否断开

  //用户自定义的回调函数
  std::function<void(spConnection)> closecallback_;                   // 关闭 fd_的回调函数
  std::function<void(spConnection)> errorcallback_;                   // fd_ 发生了错误的回调函数
  std::function<void(spConnection,std::string&)> onmessagecallback_;   // 处理报文的回调函数
  std::function<void(spConnection)> sendcompletecallback_;               // 发送数据完成后的回调函数

  Timestamp lastTime_;
};

#endif //NET__CONNECTION_H_
