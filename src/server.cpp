#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// struct sockaddr_in{
// 		unsigned short sin_family;//协议族
// 		unsigned short sin_port;//16 位端口号
// 		struct in_addr sin_addr;//32 位地址
// 		unsigned char sin_zero[8];//未使用，为保持和 struct sockaddr 一样的长度而添
// }
// struct in_addr{//IP 地址的结构体
// 		unsigned int s_addr;//32 位的IP地址，大端序
// }

using namespace std;

int main(int argc,char *argv[]){
    if(argc!=2){
        cout<<"输入错误"<<endl;
        return 0;
    }
    //1、创建 socket
    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd==-1){
        perror("socket");return -1;
    }
    //2、绑定对应通信的 ip和端口到 socket 上
    struct sockaddr_in servaddr;
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[1]));
    //注意：这里使用 htonl 是因为 INADDR_ANY的值为 0，不是字符串
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    // if(inet_aton(argv[2], &servaddr.sin_addr)==0){
    //     perror("inet_aton");
    //     return -1;
    // }

    //(struct sockaddr*)&servaddr的含义是强制类型转换
    //sockaddr_in 是 sockaddr 结构体的 IPv4 特别版
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1){
        perror("bind");
        close(listenfd);
        return -1;
    }

    //3、将 socket 设置为可监听状态
    if(listen(listenfd, 5)==-1){
        perror("listen");
        close(listenfd);
        return -1;
    }


}
