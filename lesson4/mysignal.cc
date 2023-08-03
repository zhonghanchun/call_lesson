#include <iostream>
#include <signal.h>
#include <unistd.h> 
using namespace std;
void show(sigset_t& sig)
{
    for(int i=31;i>0;i--)
    {
        if(sigismember(&sig,i))
        {
            cout<<"1";
        }else cout<<"0";
    }
    cout<<endl;
}
void handler(int signo)
{
    cout<<signo<<"号信号已经抵达!!"<<endl<<endl;
}
int main()
{
    sigset_t block,oblock,pinding;
    signal(2,handler);
    signal(3,handler);
    //初始化
    sigemptyset(&block);
    sigemptyset(&oblock);
    sigemptyset(&pinding);
    //添加要屏蔽的信号
    sigaddset(&block,2);
    sigaddset(&block,3);
    sigprocmask(SIG_SETMASK,&block,&oblock);
    int cnt=10;
    while(true)
    {
        sigemptyset(&pinding);
        sigpending(&pinding);
        show(pinding);
        sleep(1);
        if(cnt--==0)
        {
            cnt=10;
            sigprocmask(SIG_SETMASK,&oblock,&block);
            cout<<"恢复屏蔽信号"<<endl;
            sigprocmask(SIG_SETMASK,&block,&oblock);
            
        }
    }
    return 0;
}
