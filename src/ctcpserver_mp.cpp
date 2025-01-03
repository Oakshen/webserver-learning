#include <arpa/inet.h>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

class ctcpserver {
private:
  int m_listenfd;
  int m_clientfd; // 支持多个客户端
  string m_clientip;
  unsigned short m_port;

public:
  ctcpserver() : m_listenfd(-1), m_clientfd(-1) {}

  // 初始化 socket（bind 函数和 listen 函数）
  bool initServer(const unsigned short in_port) {
    if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      return false;
    }
    m_port = in_port;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); // 将定义的结构体清零
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(in_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(m_listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) ==
        -1) {
      close(m_listenfd);
      m_listenfd = -1;
      return false;
    }
    if (listen(m_listenfd, 1024) == -1) {
      close(m_listenfd);
      m_listenfd = -1;
      return false;
    }
    return true;
  }
  // 处理请求函数
  bool isAccept() {
    struct sockaddr_in caddr;
    socklen_t addrlen = sizeof(caddr);
    if ((m_clientfd =
             accept(m_listenfd, (struct sockaddr *)&caddr, &addrlen)) == -1) {
      return false;
    }

    m_clientip = inet_ntoa(caddr.sin_addr); // 将在网络中传输的大端序的 ip
                                            // 地址转换为小端序的 ip 地址
    return true;
  }
  //发送数据函数
  bool isSend(const string &buffer) {
    if (m_clientfd == -1)
      return false;
    if (send(m_clientfd, buffer.data(), buffer.size(), 0) <= 0) {
      return false;
    }
    return true;
  }
  // 接收报文
  bool isRecv(string &buffer, const size_t maxlen) {
    buffer.clear();
    buffer.resize(maxlen);
    // 这里为什么要这么写？&buffer[0]
    cout << "m_clientfd:" << m_clientfd << endl;
    int readn = recv(m_clientfd, &buffer[0], buffer.size(), 0);
    if (readn <= 0) {
      buffer.clear();
      cout << "接收失败" << endl;
      return false;
    }
    return true;
  }
  // 重载接收结构体的接收函数
  bool isRecv(void *buffer, const size_t size) {
    int readn = recv(m_clientfd, buffer, size, 0);
    if (readn <= 0) {
      return false;
    }
    return true;
  }

  // 关闭监听的 socket
  bool closeListen() {
    if (m_listenfd == -1)
      return false;
    close(m_listenfd);
    m_listenfd = -1;
    return true;
  }
  // 关闭客户端连上来的 socket
  bool closeClient() {
    if (m_clientfd == -1)
      return false;
    close(m_clientfd);
    m_clientfd = -1;
    return true;
  }
  // 发送文件的函数
  bool recvFile(const string &filename, const size_t filesize) {
    ofstream fout;
    fout.open(filename, ios::binary);
    if (fout.is_open() == false) {
      cout << "打开文件失败" << endl;
      return false;
    }
    int onread = 0;     // 每次打算接收的字节数
    int totalBytes = 0; // 已经接收的字节总数
    char buffer[4096];     // 接收文件内容的缓冲区
    while (true) {
      if (filesize - totalBytes > 4096) {
        onread=4096;
      }else {
        onread=filesize-totalBytes;
      }
      //接收文件内容
      if(isRecv(buffer,onread)==false){
        return false;
      }
      //写入文件
      // !检查
      fout.write(buffer, onread);
      //计算已接收数据大小
      totalBytes=totalBytes+onread;
      //如果已经接收完，则跳出循环
       if(totalBytes==filesize) break;
    }
    return true;
  }

  string showClientIp() { return m_clientip; }

  ~ctcpserver() {
    closeClient();
    closeListen();
  }
};


ctcpserver tcpserver;

// 父进程的信号处理函数
void FathEXITz(int sig) {
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  cout << "父进程" << getpid() << "退出，sig=" << sig << endl;
  kill(0, SIGTERM); // 向所有的子进程发送 15 信号

  // 可以增加释放资源的代码
  tcpserver.closeListen();

  exit(0);
}
// 子进程的信号处理函数
void ChldEXIT(int sig) {
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  cout << "子进程" << getpid() << "退出，sig=" << sig << endl;
  tcpserver.closeClient();
  exit(0);
}




int main(int agrc, char *argv[]) {
  if (agrc != 3) {
    cout << "please enter correct parameter" << endl;
    cout << "Using ./ctcpservser port 存放目录" << endl;
    return -1;
  }

  // 忽略全部的信号，不希望被打扰，顺便解决僵尸进程的问题
  for (int ii = 1; ii <= 64; ii++) {
    signal(ii, SIG_IGN);
  }
  signal(SIGTERM, FathEXITz);
  signal(SIGINT, FathEXITz);

  if (tcpserver.initServer(atoi(argv[1])) == false) {
    perror("initserver");
    return -1;
  }
  // 每受理一个客户端连接就 fork 一个子进程出来，让子进程负责与客户端连接
  while (true) {
    // accept 函数要理解成受理客户端的请求，就和已读和未读的区别是一样的
    // 入股无客户端连接，父进程将在此阻塞等待
    if (tcpserver.isAccept() == false) {
      perror("accept");
      return -1;
    }

    // 创建子进程
    int pid = fork();
    if (pid == -1) {
      perror("fork()");
      return -1;
    }
    if (pid > 0) {
      tcpserver.closeClient();
      continue; // 父进程回到循环开始的位置，继续受理客户端的连接
    }
    tcpserver.closeListen(); // 子进程不负责受理客户端，只负责与客户端通讯

    // ! 作用是什么?
    signal(SIGTERM, ChldEXIT);
    signal(SIGINT, SIG_IGN);

    cout << "客户端已连接" << endl;
    //   string buffer;
    //   while (true) {
    //     // 接收对端的报文
    //     if (tcpserver.isRecv(buffer, 1024) == false) {
    //       perror("recv()");
    //       break;
    //     }
    //     cout << "来自：" << tcpserver.showClientIp() << ",消息为：" << buffer
    //          << endl;
    //     buffer = "ok";
    //     if (tcpserver.isSend(buffer) == false) {
    //       perror("send");
    //       break;
    //     }
    //     cout << "发送了" << buffer << endl;
    //   }

    // 以下是接收文件的流程
    // 1、接收 文件名 和 文件大小 信息
    struct st_fileinfo {
      char filename[256];
      int filesize;
    } fileinfo; // 先定义st_fileinfo结构体，在申明变量 fileinfo
    memset(&fileinfo, 0, sizeof(fileinfo)); // 初始化
    if (tcpserver.isRecv(&fileinfo, sizeof(fileinfo)) == false) {
      perror("recv");
      return -1;
    }
    cout << "已收到通知的文件名：" << fileinfo.filename << endl
         << "文件大小:" << fileinfo.filesize << endl;
    // 2、给客户端回复确认报文，表示客户端可以发送文件了
    if (tcpserver.isSend("ok") == false) {
      perror("send");
      return -1;
    }
    // 3、接收文件内容
    if(tcpserver.recvFile(string(argv[2]) + "/" + fileinfo.filename, fileinfo.filesize)==false){
      perror("recvFile");
      return -1;
    }
    cout<<"接收文件内容成功"<<endl;
    // 4、给客户端回复确认报文，表示文件已经接收成功
    tcpserver.isSend("ok");
    return 0;//子进程一定要退出，否则会回到 accept()函数位置

  }
}