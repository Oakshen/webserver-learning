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
#include <signal.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
using namespace std;

// 先定义下initserver,然后编译器知道有这个函数
int initserver(int port);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "./tcpselect.cpp port " << endl;
    return -1;
  }
  // 初始化用于监听的 sock
  int listensock = initserver(atoi(argv[1]));
  cout << "listensock=" << listensock << endl;
  if (listensock < 0) {
    perror("listensock:");
    return -1;
  }
  // fd_set 是一个位图结构 int[32]
  fd_set readfds; // 创建一个专门监听 读事件 的文件描述符集合
  FD_ZERO(&readfds);            // 初始化 readfs
  FD_SET(listensock, &readfds); // 将服务端用于监听的 socket加入 readfds
  int maxfd = listensock;

  while (true) {
    // 定义超时结构体
    struct timeval timeout;
    timeout.tv_sec = 10; // 秒
    timeout.tv_usec = 0; // 微秒

    fd_set tmpfds = readfds; // 定义一个临时 fds

    // select() 等待事件的发生（监视哪些 socket 发生了事件）
    int infds = select(maxfd + 1, &tmpfds, NULL, NULL, &timeout);
    // 表示调用失败
    if (infds < 0) {
      perror("select() failed");
      break;
    }
    // 返回 0，表示 select 超时
    if (infds == 0) {
      perror("select timeout");
      continue;
      ;
    }

    // 大于 0，表示有事件发生
    for (int eventfd = 0; eventfd <= maxfd; eventfd++) {
      if (FD_ISSET(eventfd, &tmpfds) == 0)
        continue;
      //如果发生事件的是监听的 socket 说明已连接队列中已经有准备好的 socket
      if (eventfd == listensock) {
        struct sockaddr_in client;
        socklen_t len=sizeof(client);
        int clientsock=accept(listensock, (struct sockaddr*)&client,&len);
        if(clientsock<0){
            perror("accept:");
            continue;
        }
        cout<<"client socket fd="<<clientsock<<endl;
        FD_SET(clientsock, &readfds);//把新连上的客户端标志位设置为 1
        if(maxfd<clientsock) maxfd=clientsock;//更新 maxfd 的值
      }else {
        //代表 有数据发送过来 和 客户端已断开 两种情况
        //如果发生事件的是客户端连接的 socket，说明接收缓存中有数据可以读出（对端发送的报文已到达）
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int readn=recv(eventfd, buffer, sizeof(buffer), 0);
        if(readn<=0){
            cout<<"client(eventfd="<<eventfd<<") disconnect"<<endl;
            FD_CLR(eventfd, &readfds);
            // 当断开的客户端 socket 文件描述符恰好等于 maxfd
            /*举个例子：
            假设当前有文件描述符 3,4,7 在监听,maxfd = 7
            如果文件描述符 7 断开连接,那么需要将 maxfd 更新为 4（下一个最大的文件描述符）
            这样可以保证 select() 函数不会遍历多余的文件描述符，提高效率。*/
            if(eventfd==maxfd){
                for(int ii=maxfd;ii>0;ii--){
                    if(FD_ISSET(ii,&readfds)){
                        maxfd=ii;
                        break;
                    }
                }
            }
        }else {
            //如果收到数据
            cout<<"recv="<<buffer<<endl;
            //发送回去
            send(eventfd, buffer, sizeof(buffer), 0);
        }

      }
    }
  }
  return 0;
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