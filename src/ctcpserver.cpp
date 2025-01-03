#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

class ctcpserver {
private:
    int m_listenfd;
    int m_clientfd;//支持多个客户端
    string m_clientip;
    unsigned short m_port;
public:
    ctcpserver():m_listenfd(-1),m_clientfd(-1){}
    bool initServer(const unsigned short in_port){
        if((m_listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){
            return false;
        }
        m_port=in_port;
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));//将定义的结构体清零
        servaddr.sin_family=AF_INET;
        servaddr.sin_port=htons(in_port);
        servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
        if(bind(m_listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))==-1){
            close(m_listenfd);
            m_listenfd=-1;
            return false;
        }
        if(listen(m_listenfd,1024)==-1){
            close(m_listenfd);
            m_listenfd=-1;
            return false;
        }
        return true;
    }
    //处理请求
    bool isAccept(){
        struct sockaddr_in caddr;
        socklen_t addrlen=sizeof(caddr);
        if((m_clientfd=accept(m_listenfd, (struct sockaddr*)&caddr, &addrlen)) == -1){
            return false;
        }

        m_clientip=inet_ntoa(caddr.sin_addr);//将在网络中传输的大端序的 ip 地址转换为小端序的 ip 地址
        return true;
    }
    bool isSend(const string & buffer){
        if(m_clientfd==-1) return false;
        if(send(m_clientfd, buffer.data(), buffer.size(), 0)<=0){
            return false;
        }
        return true;
    }
    //接收报文
    bool isRecv(string &buffer ,const size_t maxlen){
        buffer.clear();
        buffer.resize(maxlen);
        //这里为什么要这么写？&buffer[0]
        cout<<"m_clientfd:"<<m_clientfd<<endl;
        int readn=recv(m_clientfd, &buffer[0], buffer.size(), 0);
        if(readn<=0){
            buffer.clear();
            cout<<"接收失败"<<endl;
            return false;
        }
        return true;
    }
    

    //关闭监听的 socket
    bool closeListen(){
        if(m_listenfd==-1) return false;
        close(m_listenfd);
        m_listenfd=-1;
        return true;
    }
    //关闭客户端连上来的 socket
    bool closeClient(){
        if(m_clientfd==-1) return false;
        close(m_clientfd);
        m_clientfd=-1;
        return true;
    }

    string showClientIp(){
        return m_clientip;
    }

    ~ctcpserver(){closeClient();closeListen();}
};


int main(int agrc,char *argv[]){
    if(agrc!=3 ){
        cout<<"please enter correct parameter"<<endl;
        cout<<"Using ./ctcpservser port pwd"<<endl;
        return -1;
    }

    ctcpserver tcpserver;
    if(tcpserver.initServer(atoi(argv[1]))==false){
        perror("initserver");
        return -1;
    }
    //accept 函数要理解出受理客户端的请求，就和已读和未读的区别是一样的
    if(tcpserver.isAccept()==false){
        perror("accept");
        return -1;
    }
    cout<<"客户端已连接"<<endl;
    string buffer;
    while (true) {
        //接收对端的报文
        if(tcpserver.isRecv(buffer, 1024)==false){
            perror("recv()");
            break;
        }
        cout<<"来自："<<tcpserver.showClientIp()<<",消息为："<<buffer<<endl;
        buffer="ok";
        if(tcpserver.isSend(buffer)==false){
            perror("send");
            break;
        }
        cout<<"发送了"<<buffer<<endl;
    }


}