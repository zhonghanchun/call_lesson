#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
using namespace std;
int main()
{
    int ans[2];
    pipe(ans); 
    pid_t id=fork();
    if(id==0)
    {
        close(ans[0]);
        while(true)
        {
            char buffer[1024];
            snprintf(buffer,sizeof buffer,"我是子进程:[%d]",getpid());
            write(ans[1],buffer,strlen(buffer));
        //    sleep(1);
        }

        close(ans[1]);
        exit(0);
    }
    close(ans[1]);
    while(true)
    {
        sleep(1);
        char buffer[1024];
        ssize_t s=read(ans[0],buffer,sizeof(buffer));
        if(s>0) buffer[s]=0;
        {
            cout<<"Get Message# "<<buffer<<" | "<<getpid()<<endl;
            break;
        }   
    }
    close(ans[0]);
    int status=0;
    waitpid(id,&status,0);
    cout<<(status & 0x7f)<<endl;

    return 0;
}
