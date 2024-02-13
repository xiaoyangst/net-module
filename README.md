## 万丈高楼平地起

```c++
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

// 设置非阻塞的IO。
void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("用法: ./tcpepoll IP 地址 端口\n");
        printf("示例: ./tcpepoll 192.168.150.128 5085\n\n");
        return -1;
    }

    // 创建服务端用于监听的listenfd。
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd < 0) {
        perror("socket() 失败");
        return -1;
    }

    int opt = 1;
    // 设置socket选项
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, sizeof(opt));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    setnonblocking(listenfd);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind() 失败");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 128) != 0) {
        perror("listen() 失败");
        close(listenfd);
        return -1;
    }

    int epollfd = epoll_create(1);

    struct epoll_event ev;
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

    struct epoll_event evs[10];

    while (1) {
        int infds = epoll_wait(epollfd, evs, 10, -1);

        if (infds < 0) {
            perror("epoll_wait() 失败");
            break;
        }

        if (infds == 0) {
            printf("epoll_wait() 超时.\n");
            continue;
        }

        for (int ii = 0; ii < infds; ii++) {
            if (evs[ii].data.fd == listenfd) {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
                setnonblocking(clientfd);

                printf("接受客户端连接(fd=%d,ip=%s,port=%d) 成功.\n", clientfd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                ev.data.fd = clientfd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
            } else {
                if (evs[ii].events & EPOLLRDHUP) {
                    printf("1客户端(eventfd=%d) 断开连接.\n", evs[ii].data.fd);
                    close(evs[ii].data.fd);
                } else if (evs[ii].events & (EPOLLIN | EPOLLPRI)) {
                    char buffer[1024];
                    while (1) {
                        bzero(&buffer, sizeof(buffer));
                        ssize_t nread = read(evs[ii].data.fd, buffer, sizeof(buffer));
                        if (nread > 0) {
                            printf("接收(eventfd=%d):%s\n", evs[ii].data.fd, buffer);
                            send(evs[ii].data.fd, buffer, strlen(buffer), 0);
                        } else if (nread == -1 && errno == EINTR) {
                            continue;
                        } else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
                            break;
                        } else if (nread == 0) {
                            printf("2客户端(eventfd=%d) 断开连接.\n", evs[ii].data.fd);
                            close(evs[ii].data.fd);
                            break;
                        }
                    }
                } else if (evs[ii].events & EPOLLOUT) {
                    // 处理输出事件
                } else {
                    printf("3客户端(eventfd=%d) 发生错误.\n", evs[ii].data.fd);
                    close(evs[ii].data.fd);
                }
            }
        }
    }

    return 0;
}
```

