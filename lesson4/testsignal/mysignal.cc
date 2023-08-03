#include <iostream>
#include <unistd.h>
#include <signal.h>
using namespace std;
int flag=0;
void handler(int signo)
{
    cout<<"捕捉："<<signo<<"信号"<<endl;
    cout<<"flag:"<<flag;
    flag=1;
    cout<<"->"<<flag<<endl;
}
int main()
{
    signal(2,handler);
    while(!flag)sleep(1);
    cout<<"正常退出!"<<endl;
    return 0;
}
