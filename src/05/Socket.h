#pragma once
#include "InetAddress.h"
#include <errno.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int createnonblocking();


class Socket {
private:
    const int fd_;      //Socket 持有的fd，在构造函数中传入
public:
    Socket(int fd);     //构造函数
    ~Socket();          //析构函数

    int fd() const;     //返回 fd_ 成员
    void setreuseaddr(bool on);     //设置允许地址重用
    void setreuseport(bool on);     // 允许多个 socket 绑定到同一个 ip 地址或者端口
    void settcpnodelay(bool on);    // 关闭 Nagle 算法（可能会引用延迟）
    void setkeepalive(bool on);     // 启用 TCP 保活机制

    void bind(const InetAddress& servaddr);     //服务端的 socket将调用次函数
    void listen(int nn=128);
    int accept(InetAddress& clientaddr);

};

