#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

// socket地址协议类

class InetAddress {
private:
  sockaddr_in addr_;//表示地址协议的结构体
public:
  InetAddress();//缺省的构造函数
  InetAddress(const std::string &ip,uint16_t port); //如果是监听的 fd,用这个构造函数
  InetAddress(const sockaddr_in addr);  //如果是客户端连上来的fd,用这个构造函数
  
  ~InetAddress();

  const char*ip() const;  //返回字符串表示的地址
  uint16_t port() const;  //返回整数表示的端口
  const sockaddr *addr() const; //返回 addr_ 成员的地址，转换成了 sockaddr
  void setaddr(sockaddr_in clientaddr);   //设置 addr_成员的值
};