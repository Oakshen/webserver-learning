#include "Socket.h"
#include "InetAddress.h"
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>


class Epoll{
private:
    static const int MaxEvents=100;     //epoll_wait()返回数组的大大小
    int epollfd_=-1;                    //创建 epoll 句柄，我们可以把它理解为一个容器，用来存放和管理所有我们关注的socket文件描述符
    epoll_event events_[int MaxEvents]; //
public:
    Epoll();
    ~Epoll();

    void addfd(int fd,uint32_t op);
    std::vector<epoll_event> loop(int timeout=-1);
};
