#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <fstream>
#include <iterator>
#include <mysql/mysql.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <rpc/types.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// struct sockaddr_in{
// 		unsigned short sin_family;//协议族
// 		unsigned short sin_port;//16 位端口号
// 		struct in_addr sin_addr;//32 位地址
// 		unsigned char sin_zero[8];//未使用，为保持和 struct sockaddr
// 一样的长度而添
// }
// struct in_addr{//IP 地址的结构体
// 		unsigned int s_addr;//32 位的IP地址，大端序
// }

using namespace std;

class ctcpclient {
public:
  int m_clientfd;
  string m_ip;
  unsigned short m_port;

  // 构造函数
  ctcpclient() : m_clientfd(-1) {}

  // 创建 socket 请求
  bool isConnect(const string &in_ip, unsigned short in_port) {
    if (m_clientfd != -1)
      return false;
    m_ip = in_ip;
    m_port = in_port;
    // 1、创建 socket 请求并存储在 h 结构体中
    if ((m_clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      return false;
    }
    struct sockaddr_in servaddr; // 存放协议、端口和 IP 地址的结构体
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(m_port);

    struct hostent *h;
    if ((h = gethostbyname(m_ip.c_str())) == nullptr) {
      close(m_clientfd);
      m_clientfd = -1;
      return false;
    }

    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
    // 2、向服务器发起连接
    if (connect(m_clientfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) ==
        -1) {
      cout << "connect failed: " << strerror(errno) << endl;
      close(m_clientfd);
      m_clientfd = -1;
      return false;
    }
    return true;
  }

  bool isSend(const string &buffer) {
    if (m_clientfd == -1)
      return false;
    if ((send(m_clientfd, buffer.data(), buffer.size(), 0)) <= 0) {
      return false;
    }
    return true;
  }
  // 发送结构体的 isSend 函数重载版
  bool isSend(void *buffer, const size_t size) {
    if (m_clientfd == -1)
      return false;
    if ((send(m_clientfd, buffer, size, 0)) <= 0) {
      return false;
    }
    return true;
  }

  bool isRecv(string &buffer, const size_t maxlen) {
    buffer.clear();
    buffer.resize(maxlen);
    int readn = recv(m_clientfd, &buffer[0], buffer.size(), 0);
    if (readn <= 0) {
      buffer.clear();
      return false;
    }
    buffer.resize(readn); // 只包含接收到的有效数据，避免浪费内存
    return true;
  }

  bool isClose() {
    if (m_clientfd == -1)
      return false;
    close(m_clientfd);
    m_clientfd = -1;
    return true;
  }

  bool sendFile(const string &filename, const size_t filesize) {
    ifstream fin(filename, ios::binary);
    if (fin.is_open() == false) {
      cout << "打开文件失败" << endl;
      return false;
    }
    int onread = 0;     // 每次调用 fin.read()时打算读取的字节数
    int totalBytes = 0; // 从文件中已经读取的字节总数
    char buffer[4096];     // 存放数据的 buffer
    while (true) {
      memset(&buffer, 0, sizeof(buffer));
      if(filesize-totalBytes>4096){
        onread=4096;
      }else {
        onread=filesize-totalBytes;
      }
      //从文件中读取数据
      fin.read(buffer, onread);
      //发送数据
      if(isSend(&buffer,onread)==false) return false;
      //计算已读取的字节总数
      totalBytes=totalBytes+onread;
      if(totalBytes==filesize) break;
    }
    return true;
  }

  // 析构函数
  ~ctcpclient() { isClose(); }
};

int main(int argc, char *argv[]) {
  if (argc != 5) {
    cout << "Using: ./ctcpclient ip port docName docSize" << endl;
    return -1;
  }
  ctcpclient tcpClient;
  if (tcpClient.isConnect(argv[1], atoi(argv[2])) == true) {
    // perror("isConnect");
    // return -1;
    cout << "连接成功" << endl;
  } else {
    cout << "连接失败" << endl;
  }

  // string buffer;//定义缓冲区
  // for(int ii=0;ii<10;ii++){

  //     buffer="这是第"+to_string(ii+1)+"个报文";
  //     //向服务端发送请求报文
  //     if(tcpClient.isSend(buffer)==false){
  //         perror("send");
  //         break;
  //     }
  //     cout<<"已发送："<<buffer<<endl;

  //     if((tcpClient.isRecv(buffer,1024))==false){
  //         perror("recv");
  //         break;
  //     }
  //     cout<<"接收到："<<buffer<<endl;
  //     sleep(1);
  // }
  
  // 以下是发送文件的流程
  // 1、将服务端要发送的 文件名 和 文件大小 告诉服务端
  struct st_fileinfo {
    char filename[256];
    int filesize;
  } fileinfo; // 先定义st_fileinfo结构体，在申明变量 fileinfo
  // ! 注意：在 socket 中推荐使用 char，不使用 string ，因为string 常常包含指向堆的指针
  memset(&fileinfo, 0, sizeof(fileinfo)); // 初始化
  strcpy(fileinfo.filename, argv[3]);
  fileinfo.filesize = atoi(argv[4]);
  if (tcpClient.isSend(&fileinfo, sizeof(fileinfo)) == false) {
    perror("send");
    return -1;
  }
  cout << "已发送文件名和文件大小" << endl;
  // 2、等待服务端的确认报文
  string buffer;
  if (tcpClient.isRecv(buffer, 2) == false) {
    perror("recv");
    return -1;
  }
  if (buffer != "ok") {
    cout << "服务端未回复" << endl;
    return -1;
  }
  // 3、发送文件内容
  if(tcpClient.sendFile(fileinfo.filename,fileinfo.filesize)==false){
    perror("sendFile");
    return -1;
  }
  // 4、等待服务端确认接收报文
  if(tcpClient.isRecv(buffer, 2)==false){
    perror("recv()");
    return -1;
  }
  if(buffer!="ok"){
    cout<<"发送文件失败"<<endl;
    return -1;
  }
  cout<<"发送成功"<<endl;
}