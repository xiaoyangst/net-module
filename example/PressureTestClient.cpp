#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string>
#include <cstring>

#define SERVER_ADDRESS "172.17.32.175"
#define SERVER_PORT 9090
#define SEND_MESSAGE "Client：压力测试中，Server！"
#define MAX_EVENTS 10
#define HEAD_SIZE 4

char SendBuffer[1024];
char RecvBuffer[1024];

void SendMessage(const std::string& message){   //封包
  memset(SendBuffer,0, sizeof(SendBuffer));   // 清空操作
  int len = message.size();              // 得到消息的长度
  memcpy(SendBuffer,&len,HEAD_SIZE);          // 头部
  memcpy(SendBuffer+HEAD_SIZE,&message,len);      // 尾部
}

bool RecvMessage(char * Buffer,std::string& data){ //拆包
  int AllLen = strlen(Buffer);   //整个包的大小
  if (AllLen < HEAD_SIZE){
    // 完全不足以解析头部，直接返回
    return false;
  }
  int BodyLen = 0;
  memcpy(&BodyLen,Buffer,HEAD_SIZE);  // 得到消息体的长度(记录在头部中)
  if (BodyLen + HEAD_SIZE < AllLen){  // 包不完整
    // 把消息临时存储在缓冲区中，等下一次读完整之后拼接完整在完整接收并输出
    return false;
  }
  std::string message(Buffer+HEAD_SIZE,BodyLen);    //得到消息体
  data = message;
  return true;
}


int main() {
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    std::cerr << "Error: Unable to create socket. " << strerror(errno) << std::endl;
    return 1;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  if (inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr) <= 0) {
    std::cerr << "Error: Invalid address. " << strerror(errno) << std::endl;
    close(clientSocket);
    return 1;
  }

  if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
    std::cerr << "Error: Connection failed. " << strerror(errno) << std::endl;
    close(clientSocket);
    return 1;
  }

  int epollFd = epoll_create1(0);
  if (epollFd == -1) {
    std::cerr << "Error: Failed to create epoll. " << strerror(errno) << std::endl;
    close(clientSocket);
    return 1;
  }

  epoll_event event{};
  event.events = EPOLLOUT | EPOLLIN;
  event.data.fd = clientSocket;
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
    std::cerr << "Error: Failed to add socket to epoll. " << strerror(errno) << std::endl;
    close(clientSocket);
    close(epollFd);
    return 1;
  }

  while (true) {
    epoll_event events[MAX_EVENTS];
    int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
    if (numEvents == -1) {
      std::cerr << "Error: epoll_wait failed. " << strerror(errno) << std::endl;
      break;
    }

    for (int i = 0; i < numEvents; ++i) {   // 写事件（回复数据给服务端）
      if (events[i].events & EPOLLOUT) {
        int len = 0;
        SendMessage(SEND_MESSAGE);    // 把要发送的数据封装成 包头+包体
        ssize_t sent = send(clientSocket, SendBuffer, strlen(SendBuffer), 0);
        if (sent == -1) {
          std::cerr << "Error: Failed to send message. " << strerror(errno) << std::endl;
          close(clientSocket);
          close(epollFd);
          return 1;
        }
        event.events = EPOLLIN;
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, clientSocket, &event) == -1) {
          std::cerr << "Error: Failed to modify socket events. " << strerror(errno) << std::endl;
          close(clientSocket);
          close(epollFd);
          return 1;
        }
      } else if (events[i].events & EPOLLIN) {    //读事件（接收服务端的数据）

        ssize_t received = recv(clientSocket, RecvBuffer, sizeof(RecvBuffer), 0);

        if (received == -1) {
          std::cerr << "Error: Failed to receive message. " << strerror(errno) << std::endl;
          close(clientSocket);
          close(epollFd);
          return 1;
        } else if (received == 0) {
          std::cerr << "Server closed connection." << std::endl;
          close(clientSocket);
          close(epollFd);
          return 0;
        }
        std::string data;
        if (RecvMessage(RecvBuffer,data)){
          std::cout << "Received: " << data << std::endl;
        }
      }
    }
  }

  close(clientSocket);
  close(epollFd);
  return 0;
}
