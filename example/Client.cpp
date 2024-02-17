//
// Created by xy on 2024-02-16.
//

// 网络通讯的客户端程序。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage:./Client ip port\n");
    return -1;
  }

  int sockfd;
  struct sockaddr_in servaddr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket() failed.\n");
    return -1;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);

  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
    printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
    close(sockfd);
    return -1;
  }

  printf("connect ok.\n");
  printf("开始时间：%ld\n", time(0));

  char buf[1024];
  for (int ii = 0; ii < 10; ii++) {
    sleep(1);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "这是第%d个超级女生。", ii);

    char tmpbuf[1024];
    memset(tmpbuf, 0, sizeof(tmpbuf));
    int len = strlen(buf);
    memcpy(tmpbuf, &len, 4);
    memcpy(tmpbuf + 4, buf, len);

    ssize_t sent_bytes = send(sockfd, tmpbuf, len + 4, 0);
    if (sent_bytes < 0) {
      perror("Failed to send data");
      close(sockfd);
      return 1;
    }

    std::cout << "Sent data: " << buf << std::endl;
  }
//
//  for (int ii = 0; ii < 10; ii++) {
//    memset(buf, 0, sizeof(buf));
//    sprintf(buf, "这是第%d个超级女生。", ii);
//
//    char tmpbuf[1024];                 // 临时的buffer，报文头部+报文内容。
//    memset(tmpbuf, 0, sizeof(tmpbuf));
//    int len = strlen(buf);                 // 计算报文的大小。
//    memcpy(tmpbuf, &len, 4);       // 拼接报文头部。
//    memcpy(tmpbuf + 4, buf, len);  // 拼接报文内容。
//    send(sockfd, tmpbuf, len + 4, 0);  // 把请求报文发送给服务端。
//    printf("Sent data: %s\n", tmpbuf);  // 输出发送的数据
//
////    recv(sockfd,&len,4,0);            // 先读取4字节的报文头部。
////    printf("接收头部数据%s\n",len);
////    memset(buf,0,sizeof(buf));
////    recv(sockfd,buf,len,0);           // 读取报文内容。
////
////    printf("recv:%s\n",buf);
//  }
  printf("结束时间：%ld\n", time(0));

  /*
  for (int ii=0;ii<10;ii++)
  {
      memset(buf,0,sizeof(buf));
      sprintf(buf,"这是第%d个超级女生。",ii);

      send(sockfd,buf,strlen(buf),0);  // 把请求报文发送给服务端。

      memset(buf,0,sizeof(buf));
      recv(sockfd,buf,1024,0);           // 读取报文内容。

      printf("recv:%s\n",buf);
sleep(1);
  }
  */

}
