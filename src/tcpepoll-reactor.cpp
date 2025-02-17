#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <netinet/in.h>
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
#include <netinet/tcp.h>
#include <fcntl.h>
using namespace std;

//设置非阻塞的 IO
void setnonblocking(int fd){
    fcntl(fd, F_SETFL,fcntl(fd, F_GETFL)|O_NONBLOCK);//位操作符 | 将当前标志与 O_NONBLOCK 进行按位或操作。
}

int main(int argc,char *argv[]){
    if(argc!=3){
        cout<<"usage:./tcpepoll-reactor ip port"<<endl;
        return -1;
    }
    //创建服务端用于监听的 listenfd
    int listenfd =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listenfd<0){
        perror("listenfd failed:");
        return -1;
    }
    
    int opt=1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));//允许地址重用
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));//关闭 Nagle 算法（可能会引用延迟）
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));//允许多个 socket 绑定到同一个 ip 地址或者端口
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));//启用 TCP 保活机制

    setnonblocking(listenfd);//将服务端的 listenfd 设置成非阻塞

    struct sockaddr_in servaddr;    //服务端 ip 地址的结构体
    servaddr.sin_family=AF_INET;    //设置协议族为 IPv4
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);    //服务端用于监听的 IP 地址
    servaddr.sin_port=htons(atoi(argv[2]));     // ! 注意这里是将服务端的 ip 地址转换为能在网络中传输的大端序

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        perror("bind failed:");
        close(listenfd);
        return -1;
    }
    if(listen(listenfd,128)!=0){
        perror("bind() failed");
        close(listenfd);
        return -1;
    }

    int epollfd=epoll_create(1);    //创建 epoll 句柄
    epoll_event ev;     //声明事件的数据结构
    ev.data.fd=listenfd;    //指定事件的自定义数据，会随着 epoll_wait()返回的时间一并返回
    ev.events=EPOLLIN;      //让 epoll 监视 listenfd 的读事件，采用水平触发

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);//将 listenfd 添加到 epoll实例中
    epoll_event evs[10];    //存放 epoll_wait() 返回事件的数组

    while (true) {
        int infds=epoll_wait(epollfd, evs, 10, -1);//等待 epoll 实例中注册的文件描述符上发生的事件
        //infds 返回发生事件的 socket 数量
        if(infds<0){
            perror("epoll_wait() failed");
            break;
        }
        if(infds==0){
            perror("epoll_wait() timeout:");
            continue;
        }

        //如果 infds>0，遍历数组
        for(int ii=0;ii<infds;ii++){
            if(evs[ii].data.fd==listenfd){//listenfd 有事件，表示有新客户端连接上来
                struct sockaddr_in clientaddr;
                socklen_t len=sizeof(clientaddr);
                int clientfd=accept(listenfd, (struct sockaddr*)&clientaddr, &len);// ! 注意最后一个参数是 &len
                // ! 因为 accept(···, ···, socklen_t *addrlen);最后一个参数是指针
                setnonblocking(clientfd);   //客户端连接的 clientfd 必须设置为非阻塞

                //为新客户端连接准备读事件，并添加到 epoll 中
                ev.data.fd=clientfd;
                ev.events=EPOLLIN|EPOLLET;  //设置为边缘触发
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
            }else {     //如果是客户端连接的 fd 有事件
                if(evs[ii].events & EPOLLRDHUP){ //对方已关闭
                    cout<<"client disconnect"<<endl;
                    close(evs[ii].data.fd);//关闭客户端 fd
                }            
                else if(evs[ii].events & (EPOLLIN|EPOLLPRI)){ //接受缓冲区中有数据可以读
                    char buffer[1024];
                    while (true) {      //由于使用非阻塞 IO，一次读取 buffer 大小数据，直到全部数据读取完毕
                        bzero(&buffer, sizeof(buffer));//将指定内存区域置为 0
                        ssize_t nread=read(evs[ii].data.fd,buffer,sizeof(buffer));
                        if(nread>0){
                            send(evs[ii].data.fd, buffer, strlen(buffer), 0);
                        }else if(nread==-1 && errno==EINTR) {//读取数据时信号中断，继续读取
                            continue;
                        }else if (nread==-1 && ((errno==EAGAIN)|| (errno==EWOULDBLOCK))) {//全部数据读取完毕
                            break;
                        }else if (nread==0) {//客户端连接已断开
                            cout<<"client disconnect"<<endl;
                            
                        }
                    }
                }  
                else if(evs[ii].events & EPOLLOUT){//有数据需要写

                }          
                else{

                }
                
            }

        }
    }

    
    
}