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
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
#include <poll.h>
using namespace std;

int initserver(int port);

// struct pollfd{
//     int fd;//文件描述符
//     short events;//需要监视的事件
//     short revents;//poll 返回的事件
// }//有事件发生时 poll 只会修改 revents

int main(int argc,char *argv[]){
    if(argc!=2){
        cout<<"please enter the correct parameter"<<endl;
        cout<<"./tcppoll port"<<endl;
        return -1;
    }
    int listensock=initserver(atoi(argv[1]));
    if(listensock<0){
        perror("initserver");
        return -1;
    }
    pollfd fds[1024];//fds用于存放需要监视的 socket
    // ! 初始化不能使用 memset
    //初始化
    for(int ii=0;ii<1024;ii++){
        fds[ii].fd=-1;
    }
    //让 poll 监视 listensock 读事件
    fds[listensock].fd=listensock;
    fds[listensock].events=POLLIN;//POLLIN表示读事件，POLLOUT 表示写事件

    int maxfd=listensock;


}