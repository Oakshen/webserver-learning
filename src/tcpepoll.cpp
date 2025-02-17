#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <netinet/in.h>
#include <ostream>
#include <poll.h>
#include <signal.h>
#include <string>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
#include <netinet/tcp.h>
#include <fcntl.h>
using namespace std;

// struct epoll_event {
//     uint32_t     events;    /* Epoll events */
//     epoll_data_t data;      /* User data variable */
// };

// typedef union epoll_data {
//     void        *ptr;
//     int          fd;
//     uint32_t     u32;
//     uint64_t     u64;
// } epoll_data_t;

// 初始化服务器(先定义)
int initserver(int port);

void setnonblocking(int fd){
    fcntl(fd, F_SETFL,fcntl(fd, F_GETFL)|O_NONBLOCK);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "./tcpepoll port" << endl;
  }
  int listensock = initserver(atoi(argv[2]));
  if (listensock < 0) {
    perror("initserver:");
    return -1;
  }
  // 创建 epoll 句柄
  int epollfd = epoll_create(1);

  epoll_event ev;
  ev.data.fd = listensock;
  ev.events = EPOLLIN; // 让 epoll 监听 listensock 事件
  epoll_ctl(epollfd, EPOLL_CTL_ADD, listensock, &ev);

  epoll_event evs[10]; // 存放 epoll 返回的事件

  while (true) {//事件循环
    int infds = epoll_wait(epollfd, evs, 10, -1);
    // 如果 infds 小于 0，说明失败
    if (infds < 0) {
      perror("epoll() failed");
      return -1;
    }
    // 如果 infds 等于 0，说明超时
    if (infds == 0) {
      cout << "epoll 等待超时" << endl;
      continue;
    }
    // 成功时 epoll_wait 返回发生事件的文件描述符数量
    // ! epoll_wait 会将检测到的事件填充到 evs 数组的前 infs 个元素中
    if (infds > 0) {
      // 遍历 epll 返回的数组
      for (int ii = 0; ii < infds; ii++) {
        if (evs[ii].data.fd == listensock) {
          struct sockaddr_in client;
          socklen_t len = sizeof(client);
          int clientsock = accept(listensock, (struct sockaddr *)&client, &len);

          // 为新连接上来的客户端准备可读事件，并添加到 epoll中继续监控
          ev.data.fd = clientsock;
          ev.events = EPOLLIN;
          epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev);
        } else {
          // 如果是客户端连接的 socket
          // 有事件发生，表示报文可读或者是客户端连接已断开
          char buffer[1024];
          memset(buffer, 0, sizeof(buffer));
          if (recv(evs[ii].data.fd, buffer, sizeof(buffer), 0) <= 0) {
            // 表示客户端的连接已断开
            cout << "客户端(eventfd=" << evs[ii].data.fd << ") 已断开" << endl;
            close(evs[ii].data.fd);
          } else {
            cout << "buffer" << endl;
            //recv(evs[ii].data.fd, buffer, sizeof(buffer), 0);
            send(evs[ii].data.fd, buffer, sizeof(buffer), 0);
          }
        }
      }
    }
  }
  return 0;
}