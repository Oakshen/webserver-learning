#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ostream>
#include <poll.h>
#include <signal.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <type_traits>
#include <unistd.h>
#include "InetAddress.h"
#include "Socket.h"
using namespace std;

// 设置非阻塞的 IO
void setnonblocking(int fd) {
  fcntl(fd, F_SETFL,
        fcntl(fd, F_GETFL) |
            O_NONBLOCK); // 位操作符 | 将当前标志与 O_NONBLOCK 进行按位或操作。
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "usage:./tcpepoll-reactor ip port" << endl;
    return -1;
  }
  
  Socket servsock(createnonblocking());
  InetAddress servaddr(argv[1],atoi(argv[2]));
  servsock.setkeepalive(true);
  servsock.setreuseaddr(true);
  servsock.setreuseport(true);
  servsock.settcpnodelay(true);
  servsock.bind(servaddr);
  servsock.listen(); 

  int epollfd = epoll_create(1); // 创建 epoll 句柄
  epoll_event ev;                // 声明事件的数据结构
  ev.data.fd =servsock.fd(); // 指定事件的自定义数据，会随着 epoll_wait()返回的时间一并返回
  ev.events = EPOLLIN; // 让 epoll 监视 listenfd 的读事件，采用水平触发
  
  epoll_ctl(epollfd, EPOLL_CTL_ADD, servsock.fd(),&ev);      // 将 listenfd 添加到 epoll实例中
  epoll_event evs[10]; // 存放 epoll_wait() 返回事件的数组

  while (true) {
    int infds = epoll_wait(epollfd, evs, 10,-1); // 等待 epoll 实例中注册的文件描述符上发生的事件
    // infds 返回发生事件的 socket 数量
    if (infds < 0) {
      perror("epoll_wait() failed");
      break;
    }
    if (infds == 0) {
      perror("epoll_wait() timeout:");
      continue;
    }

    // 如果 infds>0，遍历数组
    for (int ii = 0; ii < infds; ii++) {
      // 如果是客户端连接的 fd 有事件
      if (evs[ii].events & EPOLLRDHUP) { // 对方已关闭
        cout << "client disconnect" << endl;
        close(evs[ii].data.fd); // 关闭客户端 fd
      } else if (evs[ii].events &(EPOLLIN | EPOLLPRI)) { // 接受缓冲区中有数据可以读
        if (evs[ii].data.fd ==servsock.fd()) { // listenfd 有事件，表示有新客户端连接上来

          InetAddress clientaddr;
          Socket *clientsock=new Socket(servsock.accept(clientaddr));
          //创建一个指向socket的指针

          cout<<"来自"<<clientaddr.ip()<<"的连接"<<"端口为："<<clientaddr.port()<<endl;

          // 为新客户端连接准备读事件，并添加到 epoll 中
          ev.data.fd = clientsock->fd();
          ev.events = EPOLLIN | EPOLLET; // 设置为边缘触发
          epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock->fd(), &ev);
        } else {
          char buffer[1024];
          while (true) { // 由于使用非阻塞 IO，一次读取 buffer
                         // 大小数据，直到全部数据读取完毕
            bzero(&buffer, sizeof(buffer)); // 将指定内存区域置为 0
            ssize_t nread = read(evs[ii].data.fd, buffer, sizeof(buffer));
            if (nread > 0) {
              cout << "接收到：" << buffer << endl;
              send(evs[ii].data.fd, buffer, strlen(buffer), 0);
            } else if (nread == -1 &&
                       errno == EINTR) { // 读取数据时信号中断，继续读取
              continue;
            } else if (nread == -1 &&
                       ((errno == EAGAIN) ||
                        (errno == EWOULDBLOCK))) { // 全部数据读取完毕
              break;
            } else if (nread == 0) { // 客户端连接已断开
              cout << "client disconnect" << endl;
              close(evs[ii].data.fd);
              break;
            }
          }
        }
      } else if (evs[ii].events & EPOLLOUT) { // 有数据需要写

      } else { // 其他事件，都视为错误
        cout << "client error" << endl;
        close(evs[ii].data.fd);
      }
    }
  }
}