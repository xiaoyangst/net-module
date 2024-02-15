
//
// Created by xy on 2024-02-13.
//

#ifndef NET__TCPSERVER_H_
#define NET__TCPSERVER_H_

#include <memory>
#include <vector>
#include "EventLoop.h"
#include "Acceptor.h"
#include "ThreadPool.h"
#include "Connection.h"


class TcpServer {
 public:
  using spConnection = std::shared_ptr<Connection>;

  TcpServer(const std::string &ip,const uint16_t port,int threadnum=3);
  ~TcpServer();

  void start();
  void stop();

  void newconnection(std::unique_ptr<Socket> clientsock);
  void closeconnection(spConnection conn);
  void errorconnection(spConnection conn);
  void onmessage(spConnection conn,std::string& message);
  void sendcomplete(spConnection conn);
  void epolltimeout(EventLoop *loop);
  void removeconn(int fd);

  void setnewconnectioncb(std::function<void(spConnection)> cb);
  void setcloseconnectioncb(std::function<void(spConnection)> cb);
  void seterrorconnectioncb(std::function<void(spConnection)> cb);
  void setonmessagecb(std::function<void(spConnection,std::string &message)> cb);
  void setsendcompletecb(std::function<void(spConnection)> cb);
  void settimeoutcb(std::function<void(EventLoop*)> cb);
  void setremoveconnectioncb(std::function<void(int)> cb);
 private:
  // 主从事件循环
  std::unique_ptr<EventLoop> mainloop_;
  std::vector<std::unique_ptr<EventLoop>> subloops_;

  // 监听客户端的新连接（一个TcpServer 只有一个 Acceptor）
  Acceptor acceptor_;

  int threadNum_; //线程数量，其实就是subloop的数量
  ThreadPool threadpool_; // 线程池

  std::mutex connmutex_;
  std::map<int,spConnection>  conns_;

  std::function<void(spConnection)> closecallback_;                   // 关闭 fd_的回调函数
  std::function<void(spConnection)> errorcallback_;                   // fd_ 发生了错误的回调函数
  std::function<void(spConnection,std::string& message)> onmessagecallback_;   // 处理报文的回调函数
  std::function<void(spConnection)> sendcompletecallback_;               // 发送数据完成后的回调函数
  std::function<void(spConnection)> newconnectioncb_;                 // 新连接的回调
  std::function<void(int)> removeconnectioncb_;                       // 移除连接的回调
  std::function<void(EventLoop*)>  timeoutcb_;                        // 超时回调


};

#endif //NET__TCPSERVER_H_
