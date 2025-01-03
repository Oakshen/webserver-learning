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
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
using namespace std;

int initserver(int port);

// struct pollfd{
//     int fd;//文件描述符
//     short events;//需要监视的事件
//     short revents;//poll 返回的事件
// }//有事件发生时 poll 只会修改 revents

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "please enter the correct parameter" << endl;
    cout << "./tcppoll port" << endl;
    return -1;
  }
  int listensock = initserver(atoi(argv[1]));
  if (listensock < 0) {
    perror("initserver");
    return -1;
  }
  pollfd fds[2048]; // fds用于存放需要监视的 socket
  // ! 初始化不能使用 memset
  // 初始化
  for (int ii = 0; ii < 2048; ii++) {
    fds[ii].fd = -1;
  }
  // 让 poll 监视 listensock 读事件
  fds[listensock].fd = listensock;
  fds[listensock].events = POLLIN; // POLLIN表示读事件，POLLOUT 表示写事件

  int maxfd = listensock;
  while (true) {
    // 调用 poll() 事件等待事件的发生（监听哪些 socket 发生了事件）
    int infds = poll(fds, maxfd + 1, 10000);
    if (infds == 0) {
      cout << "poll() timeout" << endl;
      continue;
    }
    if (infds < 0) {
      cout << "poll() fail" << endl;
      break;
      ;
    }
    // 如果infds>0,表示有事件发生，infds 存放了已发生事件的个数
    for (int eventfd = 0; eventfd < maxfd; eventfd++) {
      if (fds[eventfd].fd<0) continue;
      if((fds[eventfd].revents & POLLIN)==0) continue;
      //如果eventfd==listensock，说明有新的客户端连接上来
      if(eventfd==listensock){
        struct sockaddr_in client;
        socklen_t len=sizeof(client);
        int clientsock=accept(eventfd, (struct sockaddr*)&client, &len);
        if(clientsock<0){
            perror("clientsock error:");
            continue;
        }
        fds[clientsock].fd=clientsock;
        fds[clientsock].events=POLLIN;

        if(maxfd<clientsock) maxfd=clientsock;//更新 macfd 的值
      } else {
        //如果不是有客户端连上来了，说明是有读事件或者是客户端已经断开连接
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int readn=recv(eventfd, buffer, sizeof(buffer), 0);
        if(readn<0){
            //客户端连接已断开的情况
            close(eventfd);
            fds[eventfd].fd=-1;

            //重新计算 maxfd 的值
            if(eventfd==maxfd){
                for(int ii=maxfd;ii>0;ii--){
                    if(fds[ii].fd!=-1){
                        maxfd=ii;
                        break;
                    }
                }
            }
        }else {
            //如果收到报文
            cout<<"接收到数据："<<buffer<<endl;
            send(eventfd, buffer, sizeof(buffer), 0);
        }

      }
    }
  }
}


int initserver(int port) {
  int listenfd = -1;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    return -1;
  }
  // !配置 socket
  int opt = 1;
  unsigned int len = sizeof(opt);
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  // 绑定本机的所有 ip 地址
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    close(listenfd);
    listenfd = -1;
    return -1;
  }
  if (listen(listenfd, 1024) == -1) {
    close(listenfd);
    listenfd = -1;
    return -1;
  }
  return listenfd;
}