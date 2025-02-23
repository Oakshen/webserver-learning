#include "Epoll.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

// class Epoll{
//     private:
//         static const int MaxEvents=100;    //epoll_wait()返回数组的大大小
//         int epollfd_=-1;            //创建 epoll 句柄
//         epoll_event events_[int MaxEvents];//
//     public:
//         Epoll();
//         ~Epoll();

//         void addfd(int fd,uint32_t op);
//         std::vector<epoll_event> loop(int timeout=-1);
//     };

Epoll::Epoll() {
  if ((epollfd_ = epoll_create(1)) == -1) {
    perror("epoll create failed");
    exit(-1);
  }
}

Epoll::~Epoll() { close(epollfd_); }

void Epoll::addfd(int fd, uint32_t op) {
  epoll_event ev; // 声明事件的数据结构
  ev.data.fd =fd; // 指定事件的自定义数据，会随着 epoll_wait()返回的时间一并返回
  ev.events =op; // 让 epoll 监视 listenfd 的读事件，采用水平触发

  if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd,&ev)==-1){
    perror("epoll_ctl error");
    exit(-1);
  } 

std::vector<epoll_event> loop(int timeout) {
    bzero(events_)
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
}