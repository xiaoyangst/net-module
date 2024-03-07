//
// Created by xy on 2024-02-16.
//

#include "TcpServer.h"
#include <sys/syscall.h>
#include <iostream>

using spConnection = std::shared_ptr<Connection>;

class EchoServer{

 private:
  TcpServer tcpserver_;
  ThreadPool threadpool_;
 public:
  EchoServer(const std::string &ip, const uint16_t port, int subthreadnum,size_t workthreadnum)
      : tcpserver_(ip,port,subthreadnum)
      , threadpool_(workthreadnum,"WORKS")
  {
    tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
    tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));

  }

// 启动服务。
  void Start()
  {
    tcpserver_.start();
  }

  // 停止服务。
  void Stop()
  {
    // 停止工作线程。
    threadpool_.stop();
    printf("工作线程已停止。\n");

    // 停止IO线程（事件循环）。
    tcpserver_.stop();
  }

// 处理新客户端连接请求，在TcpServer类中回调此函数。
  void HandleNewConnection(spConnection conn)
  {
    // std::cout << "New Connection Come in." << std::endl;
    // printf("EchoServer::HandleNewConnection() thread is %d.\n",syscall(SYS_gettid));
    printf ("%s new connection(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().toString().c_str(),conn->getCfd(),conn->getCip().c_str(),conn->getCport());

    // 根据业务的需求，在这里可以增加其它的代码。
  }

// 关闭客户端的连接，在TcpServer类中回调此函数。
  void HandleClose(spConnection conn)
  {
    printf("EchoServer::HandleClose thread is %ld.\n",syscall(SYS_gettid));
    printf ("%s connection closed(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().toString().c_str(),conn->getCfd(),conn->getCip().c_str(),conn->getCport());

    // std::cout << "EchoServer conn closed." << std::endl;
    // tcpserver_.removeconn(conn->getCfd());
    // 根据业务的需求，在这里可以增加其它的代码。
  }

// 客户端的连接错误，在TcpServer类中回调此函数。
  void HandleError(spConnection conn)
  {
     std::cout << "EchoServer conn error." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
  }

// 处理客户端的请求报文，在TcpServer类中回调此函数。
  void HandleMessage(spConnection conn,std::string& message)
  {
    // printf("EchoServer::HandleMessage() thread is %d.\n",syscall(SYS_gettid));

    if (threadpool_.size()==0)
    {
      // 如果没有工作线程，表示在IO线程中计算。
      OnMessage(conn,message);
    }
    else
    {
      // 把业务添加到线程池的任务队列中，交给工作线程去处理业务。
      threadpool_.addtask(std::bind(&EchoServer::OnMessage,this,conn,message));
    }
  }

  // 处理客户端的请求报文，用于添加给线程池。
  void OnMessage(spConnection conn,std::string& message)
  {
     //printf("%s message (fd=%d):%s\n",Timestamp::now().toString().c_str(),conn->getCfd(),message.c_str());

    // 在这里，将经过若干步骤的运算。
    // message="reply:"+message;          // 回显业务（压力测试时不用输出）
    //std::cout<<"OnMessage message = "<<message<<std::endl;
    if (!conn->getConStatue()){   // 发送数据之前检查连接状态，如果连接还在才允许发送
      conn->send(message.data(),message.size());// 把数据发送出去
    }
  }

// 数据发送完成后，在TcpServer类中回调此函数。
  void HandleSendComplete(spConnection conn)
  {
    // std::cout << "Message send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
  }
};



#include <signal.h>

EchoServer* Server;

void Stop(int sig)    // 信号2和15的处理函数，功能是停止服务程序。
{
  printf("sig=%d\n",sig);
  // 调用BankServer::Stop()停止服务。
  Server->Stop();
  printf("Server。\n");
  delete Server;
  printf("delete Server。\n");
  exit(0);
}

int main(int argc,char *argv[]){
  if (argc != 3)
  {
    printf("usage: ./Server ip port\n");
    printf("./Server 172.17.32.175 9090\n");
    return -1;
  }

  signal(SIGTERM,Stop);    // 信号15，系统kill或killall命令默认发送的信号。
  signal(SIGINT,Stop);        // 信号2，按Ctrl+C发送的信号。

  Server = new EchoServer(argv[1],atoi(argv[2]),3,5);
  Server->Start();

  return 0;
}