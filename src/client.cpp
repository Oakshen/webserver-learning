#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(int argc ,char *argv[]){

    if(argc!=3){
        cout<<"./client.cpp ip port"<<endl;
    }

    //1、创建 socket 请求并存储在 h 结构体中
    int sockfd=socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1){
        perror("sockfd");
    }
    struct sockaddr_in servaddr;//存放协议、端口和 IP 地址的结构体
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));

    struct hostent *h;
    if((h=gethostbyname(argv[1]))==nullptr){
        perror("gethostbyname");
        cout<<"gethostbyname failed"<<endl;
        close(sockfd);
        return -1;
    }
    
    memcpy(&servaddr.sin_addr,h->h_addr_list,h->h_length);
    //2、向服务器发起连接
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))==-1){
        perror("connect");
        close(sockfd);
        return -1;
    }
    //3、与服务器通讯
    char buffer[1024];//定义缓冲区
    for(int ii=0;ii<10;ii++){
        int iret;
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "这是第 %d个报文",ii+1);
        //向服务端发送请求报文
        if((iret=send(sockfd, buffer, sizeof(buffer), 0))<=0){
            perror("send");
            break;
        }
        cout<<"已发送："<<buffer<<endl;

        memset(buffer, 0, sizeof(buffer));
        if((iret=recv(sockfd, buffer, sizeof(buffer), 0))<=0){
            cout<<"iret="<<iret<<endl;
        }
        cout<<"接收到："<<buffer<<endl;
    }
    

}