#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

// 定义一个模板类
// T 有可能是 string，char，int 等多种类型
template <typename T> class Box {
private:
    T data; // 存储的数据
public:
    //构造函数
    Box(T value):data(value){}

    //获取函数
    T getData(){
        return data;
    }
    //设置数据
    void setData(T value){
        data=value;
    }
};

template <class TT ,int MaxLength>class squeue{
private:
    bool m_inited;//队列是否被初始化
    TT m_data[MaxLength];//使用数组来存储循环队列中的元素
    int m_head;//队列头指针
    int m_tail;//队列尾指针
    int m_length;//队列实际长度
    squeue(const squeue &)=delete;//禁用拷贝构造函数
    squeue &operator=(const squeue &)=delete;//禁用赋值函数

public:
    squeue(){ init(); }//构造函数
    void init(){
        
    }

};



int main(){
    Box< vector<int> > inBox({1,2});
    vector<int> get=inBox.getData();
    for(int i=0;i<get.size();i++){
        cout<<get[i]<<" ";
        
    }
    cout<<endl;

}
