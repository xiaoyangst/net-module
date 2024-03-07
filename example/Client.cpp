#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("usage:./client ip port\n");
    return -1;
  }

  int sockfd;
  struct sockaddr_in servaddr;
  char buf[1024];

  if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }

  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_port=htons(atoi(argv[2]));
  servaddr.sin_addr.s_addr=inet_addr(argv[1]);

  if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
  {
    printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
  }

  printf("connect ok.\n");
  printf("开始时间：%ld\n",time(0));

  for (int ii=0;ii<50000;ii++)
  {
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%d个",ii);

    char tmpbuf[1024]; // 修改为足够容纳报文头部和报文内容的大小
    memset(tmpbuf,0,sizeof(tmpbuf));
    size_t len=strlen(buf);
    if (len > 1024) {
      printf("Message too long\n");
      close(sockfd);
      break;
    }
    memcpy(tmpbuf,&len,4);
    strcpy(tmpbuf+4,buf);

    send(sockfd,tmpbuf,len+4,0);

    int recv_len;
    recv(sockfd,&recv_len,4,0);
    if (recv_len > 1024) {
      printf("Received message too long\n");
      close(sockfd);
      break;
    }
    memset(buf,0,sizeof(buf));
    recv(sockfd,buf,recv_len,0);
  }
  printf("结束时间：%ld\n",time(0));
  close(sockfd);
  return 0;
}
