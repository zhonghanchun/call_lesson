#pragma once 
#include <iostream>
#include <sys/epoll.h>
#include <string>
#include <cstring>
#include <unistd.h>
class Epoller
{
    private:
        int _epfd;
    public:
        Epoller():_epfd(-1)
        {}
        ~Epoller(){
            if(_epfd!=-1){
                close(_epfd);
            }
        }
        void Create()
        {
            _epfd=epoll_create(128);
    
            if(_epfd<0){
                std::cout<<"epoll_create failed!"<<std::endl;
                exit(-1);
            }        
        }
        bool AddEvent(int sock,uint32_t events){
            struct epoll_event ev;
            ev.data.fd=sock;
            ev.events=events;
            int n=epoll_ctl(_epfd,EPOLL_CTL_ADD,sock,&ev);
            return n==0;
        }
        int Wait(struct epoll_event* evens,int num,int timeout)
        {
            int n=epoll_wait(_epfd,evens,num,timeout);
            return n;
        }
        void Control(int sock,uint32_t events,int flag)
        {
            if(flag==EPOLL_CTL_MOD)
            {
                struct epoll_event ev;
                ev.data.fd=sock;
                ev.events=events;
                epoll_ctl(_epfd,flag,sock,&ev);
            }else if(flag==EPOLL_CTL_DEL)
            {
                epoll_ctl(_epfd,flag,sock,nullptr);
            }else{}
        }
        void Close()
        {
            if(_epfd!=-1)
                close(_epfd);
        }
};
