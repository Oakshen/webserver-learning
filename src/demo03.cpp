#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

int main(){
    int fd;
    fd=open("data.txt",O_TRUNC|O_RDWR);
    if(fd==-1){
        perror("open(data.txt)");
        return -1;
    }
    cout<<"文件描述符 fd= "<< fd <<endl; 
}
