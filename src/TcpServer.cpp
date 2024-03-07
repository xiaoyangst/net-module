//
// Created by xy on 2024-02-13.
//

#include "TcpServer.h"


TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum)
  : threadNum_(threadnum)
  , mainloop_(new EventLoop(true))    // mainloop
  , acceptor_(mainloop_.get(),ip,port)
  , threadpool_(threadNum_,"IO")
{
  // 设置epoll_wait()超时的回调函数
  mainloop_->setEpollTimeoutCallback(std::bind(&TcpServer::epolltimeout, this,std::placeholders::_1));
  // 设置处理新客户端连接请求的回调函数
  acceptor_.setNewConnCb(std::bind(&TcpServer::newconnection, this,std::placeholders::_1));

  // subloop
  for (int i = 0; i < threadNum_; i++) {
    subloops_.emplace_back(new EventLoop(false,5,10));    // 创建从事件循环，存入subloops_容器中
    subloops_[i]->setEpollTimeoutCallback(std::bind(&TcpServer::epolltimeout, this,std::placeholders::_1));
    subloops_[i]->setTimerCallback(std::bind(&TcpServer::removeconn, this,std::placeholders::_1));
    threadpool_.addtask(std::bind(&EventLoop::run,subloops_[i].get())); // 在线程池中运行从事件循环
  }
}
TcpServer::~TcpServer() {

}
void TcpServer::start() {
  mainloop_->run();
}
void TcpServer::stop() {
  mainloop_->stop();
  printf("MainLoop stop ...\n");
  for (int i = 0; i < threadNum_; i++) {
    subloops_[i]->stop();
  }
  printf("SubLoop stop ...\n");
  threadpool_.stop();
  printf("ThreadPool stop ...\n");
}

void TcpServer::setcloseconnectioncb(std::function<void(spConnection)> cb) {
  closecallback_ = cb;
}

void TcpServer::seterrorconnectioncb(std::function<void(spConnection)> cb) {
  errorcallback_ = cb;
}

void TcpServer::setonmessagecb(std::function<void(spConnection, std::string & )> cb) {
  onmessagecallback_ = cb;
}

void TcpServer::setnewconnectioncb(std::function<void(spConnection)> cb) {
  newconnectioncb_ = cb;
}

void TcpServer::setremoveconnectioncb(std::function<void(int)> cb) {
  removeconnectioncb_ = cb;
}

void TcpServer::settimeoutcb(std::function<void(EventLoop *)> cb) {
  timeoutcb_ = cb;
}

void TcpServer::setsendcompletecb(std::function<void(spConnection)> cb) {
  sendcompletecallback_ = cb;
}

void TcpServer::removeconn(int fd) {
  std::lock_guard<std::mutex> lock_guard(connmutex_);
  conns_.erase(fd);

  if (removeconnectioncb_){
    removeconnectioncb_(fd);
  }
}

void TcpServer::onmessage(TcpServer::spConnection conn, std::string &message) {
  if (onmessagecallback_){
    onmessagecallback_(conn,message);
  }
}

void TcpServer::sendcomplete(TcpServer::spConnection conn) {
  if (sendcompletecallback_){
    sendcompletecallback_(conn);
  }
}

void TcpServer::epolltimeout(EventLoop *loop) {
  if (timeoutcb_){
    timeoutcb_(loop);
  }
}

void TcpServer::errorconnection(TcpServer::spConnection conn) {
  if (errorcallback_){
    errorcallback_(conn);
  }
  {
    std::lock_guard<std::mutex> lock_guard(connmutex_);
    // 连接错误，移除即可
    conns_.erase(conn->getCfd());
  }
}

void TcpServer::closeconnection(TcpServer::spConnection conn) {
  if (closecallback_){
    closecallback_(conn);    // 回调EchoServer::HandleClose()
  }
  {
    std::lock_guard<std::mutex> lock_guard(connmutex_);
    // 连接关闭，移除即可
    conns_.erase(conn->getCfd());
  }
}

void TcpServer::newconnection(std::unique_ptr<Socket> clientsock) {
  spConnection conn(new Connection(subloops_[clientsock->getfd() % threadNum_].get(),std::move(clientsock)));
  conn->setCloseCallback(std::bind(&TcpServer::closeconnection,this,std::placeholders::_1));
  conn->setErrorCallback(std::bind(&TcpServer::errorconnection,this,std::placeholders::_1));
  conn->setOnmessageCallback(std::bind(&TcpServer::onmessage,this,std::placeholders::_1,std::placeholders::_2));
  conn->setWriteCallback(std::bind(&TcpServer::sendcomplete,this,std::placeholders::_1));

  {
    std::lock_guard<std::mutex> lock_guard(connmutex_);
    conns_[conn->getCfd()] = conn;
  }

  subloops_[conn->getCfd() % threadNum_]->newconnection(conn);

  if (newconnectioncb_){
    newconnectioncb_(conn);
  }
}