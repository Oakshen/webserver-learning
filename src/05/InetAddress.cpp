#include "InetAddress.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// class InetAddress {
//     private:
//       sockaddr_in addr_;//表示地址协议的结构体
//     public:
//       InetAddress(const std::string &ip,uint16_t port);//如果是监听的
//       fd,用这个构造函数 InetAddress(const sockaddr_in
//       addr):addr_(addr){}//如果是客户端连上来的fd,用这个构造函数
//       ~InetAddress();

//       const char*ip() const;  //返回字符串表示的地址
//       uint16_t port() const;  //返回整数表示的端口
//       const sockaddr *addr() const; //返回 addr_ 成员的地址，转换成了
//       sockaddr
// };
InetAddress::InetAddress(){
  
}

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
  addr_.sin_family = AF_INET;                    // 设置协议族为 IPv4
  addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 服务端用于监听的 IP 地址
  addr_.sin_port = htons(port); // ! 注意这里是将服务端的 ip 地址转换为能在网络中传输的大端序
}

InetAddress::InetAddress(const sockaddr_in addr): addr_(addr) { // 如果是客户端连上来的fd,用这个构造函数


}

// 析构函数
InetAddress::~InetAddress() 
{

}

const char *InetAddress::ip() const {
  return inet_ntoa(addr_.sin_addr);
}
uint16_t InetAddress::port() const {
  return ntohs(addr_.sin_port);
}
const sockaddr *InetAddress::addr() const {
  return (sockaddr*)&addr_;
}

void InetAddress::setaddr(sockaddr_in clientaddr){
  addr_=clientaddr;
}