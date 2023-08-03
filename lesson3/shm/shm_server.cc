#include "comm.hpp"
#include <unistd.h>

int main()
{
    key_t k=get_Key();
    int shmid=createShm(k);
    printf("k : 0x%x\n",k);
    printf("shmid : %d\n",shmid);
    char* mem=(char*)attachShm(shmid);
    printf("attach success mem: %p\n",mem);
    while(true)
    {
        printf("client# %s\n",mem);
        sleep(1);
    }
    detachShm(mem);
    sleep(5);
    delShm(shmid);
}