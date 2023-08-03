#ifndef _COMM_HPP_
#define _COMM_HPP_

#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define PATHNAME "."
#define PROJ_ID 0x66
#define SIZE 2048
key_t get_Key()
{
    key_t k=ftok(PATHNAME,PROJ_ID);
    if(k<0)
    {
        std::cout<<"get_Key() fail"<<std::endl;
        exit(1);
    }
    return k;
}
int getShmHelper(key_t k,int flags)
{
    int shmid=shmget(k,SIZE,flags);
    if(shmid<0)
    {
        std::cout<<"getShmHelper() fail"<<std::endl;
        exit(2);
    }
    return shmid;
}
void* attachShm(int shmid)
{
    void* mem=shmat(shmid,nullptr,0);
    if((long long)mem==-1)
    {
        std::cout<<"attachShm() fail"<<std::endl;
        exit(4);
    }
    return mem;
}
void detachShm(void* start)
{
    if(shmdt(start)==-1)
    {
        std::cout<<"detachShm() fail"<<std::endl;
        exit(5);
    }
}
void delShm(int shmid)
{
    if(shmctl(shmid,IPC_RMID,nullptr)==-1)
    {
        std::cout<<"delShm() fail"<<std::endl;
        exit(3);
    }
}
int getShm(key_t k)
{
    return getShmHelper(k,IPC_CREAT); 
}
int createShm(key_t k)
{
    return getShmHelper(k,IPC_CREAT|IPC_EXCL|0600);
}
#endif