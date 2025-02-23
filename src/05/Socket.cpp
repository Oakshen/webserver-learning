#include "Socket.h"
#include "InetAddress.h"
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

// class Socket {
// private:
//   const int fd_; // Socket 持有的fd，在构造函数中传入
// public:
//   Socket(int fd); // 构造函数
//   ~Socket();      // 析构函数

//   int fd() const;             // 返回 fd_ 成员
//   void setreuseaddr(bool on); // 设置允许地址重用
//   void setreuseport(bool on); // 允许多个 socket 绑定到同一个 ip 地址或者端口
//   void settcpnodelay(bool on); // 关闭 Nagle 算法（可能会引用延迟）
//   void setkeepalive(bool on);  // 启用 TCP 保活机制

//   void bind(const InetAddress &servaddr); // 服务端的 socket将调用次函数
//   void listen(int nn = 128);
//   void accept(InetAddress &clientaddr);
// };

int createnonblocking(){
    int listenfd =socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK,IPPROTO_TCP); // SOCK_NONBLOCK表示将 listenfd 设置成非阻塞
    if (listenfd < 0) {
    perror("listenfd failed:");
    exit(-1);
    }
    return listenfd;
}

Socket::Socket(int fd):fd_(fd){

}


Socket::~Socket(){
    ::close(fd_);
}

int Socket::fd() const{
    return fd_;//作用就是从类中返回 fd 成员
}

void Socket::settcpnodelay(bool on){
    int optval=on?1:0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setreuseaddr(bool on){
    int optval=on?1:0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval,static_cast<socklen_t>(sizeof optval)); // 允许地址重用
}

void Socket::setreuseport(bool on){
    int optval=on?1:0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval,static_cast<socklen_t>(sizeof optval));
}

void Socket::setkeepalive(bool on){
    int optval=on?1:0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval,static_cast<socklen_t>(sizeof optval));
}


void Socket::bind(const InetAddress &servaddr){
    if (::bind(fd_, servaddr.addr(), sizeof(servaddr)) < 0) {
        perror("bind failed:");
        close(fd_);
        exit(-1);
      }
}

void Socket::listen(int nn){
    // ::表示要调用的是系统提供的全局全局 listen() 函数（来自 <sys/socket.h>），
    // 而不是其他命名空间或类中可能存在的 listen() 函数
    if (::listen(fd_, 128) != 0) {
        perror("bind() failed");
        close(fd_);
        exit(-1);
      }
}

int Socket::accept(InetAddress& clientaddr){
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK); // ! 注意最后一个参数是 &len
    // ! 因为 accept(···, ···, socklen_t *addrlen);最后一个参数是指针
    // setnonblocking(clientfd);   //客户端连接的 clientfd
    // 必须设置为非阻塞
    clientaddr.setaddr(peeraddr);       //客户端的地址和协议
    return clientfd;
}