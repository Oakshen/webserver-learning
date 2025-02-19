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

int main(int argc,char* argv[]){
    if(argc!=3){
        cout<<"usage:./tcpepoll-client ip port"<<endl;
        return -1;
    }
    char buf[1024];
    int sockfd=socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1){
        perror("sockfd");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);//转换 ip 地址

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))!=0){
        perror("connect failed:");
        return -1;
    }
    cout<<"connect ok"<<endl;

    for(int ii=0;ii<2000;ii++){
        memset(buf, 0, sizeof(buf));
        cout<<"please input:"<<endl;
        cin>>buf;
        if(send(sockfd, buf, strlen(buf), 0)<=0){
            perror("write failed:");
            close(sockfd);
            return -1;
        }
        memset(buf, 0, sizeof(buf));
        if(recv(sockfd, buf, sizeof(buf), 0)<=0){
            perror("recv failed:");
            close(sockfd);
            return -1;
        }
        cout<<"接收到:"<<buf<<endl;
    }
    

    

}