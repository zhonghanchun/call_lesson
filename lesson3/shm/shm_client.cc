#include "comm.hpp"
#include <unistd.h>
int main()
{
    key_t k=get_Key();
    int shmid=getShm(k);
    printf("k : 0x%x\n",k);
    printf("shmid : %d\n",shmid);
    char* mem=(char*)attachShm(shmid);
    printf("attach success mem: %p\n",mem);
    int cnt=1;
    pid_t id=getpid();
    const char* s="我是另一个进程!";
    while(true)
    {
        sleep(1);
        snprintf(mem,SIZE,"消息编号[%d]:%s,进程id:[%d]",cnt++,s,id);
    }
    detachShm(mem);
}