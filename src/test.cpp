#include <iostream>

using namespace std;




int main(){
    int a=100;
    int *p=new int;
    cout<<"p="<<p<<endl;
    delete p;
    p=&a;
    cout<<"p="<<p<<endl;
}