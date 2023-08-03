#include <iostream>
#include <functional>
#include <string>
#include <sys/epoll.h>
#include "sock.hpp"

namespace epoll_ns
{
    using fun_t=std::function<std::string(const std::string&)>; 
    class epollServer
    {
    private:
        uint16_t _port;
        int _sock;
        int _epfd;
        struct epoll_event* _revs;
        int _num;
        fun_t _fun;
    public:
        epollServer(fun_t fun,uint16_t port=8080,int num=64)
            :_port(port),_sock(-1),_epfd(-1),_revs(nullptr),_num(num),_fun(fun)
        {}
        ~epollServer()
        {
            if(_sock!=-1)close(_sock);
            if(_epfd!=-1)close(_epfd);
            if(_revs)delete[] _revs;
        }
        void Init()
        {
            //创建socket
            _sock=Sock::Socket();
            Sock::Bind(_sock,_port);
            Sock::Listen(_sock);
            //创建epoll模型
            _epfd=epoll_create(128);
            if(_epfd<0){
                std::cout<<"epoll_create failed!"<<std::endl;
                exit(-1);
            }
            //添加_sock到_epfd中
            struct epoll_event ev;
            ev.events=EPOLLIN;
            ev.data.fd=_sock;
            epoll_ctl(_epfd,EPOLL_CTL_ADD,_sock,&ev);

            _revs=new struct epoll_event[_num];
        }
        void Accept(int sock)
        {
            std::string client_ip;
            uint16_t client_port;
            int fd=Sock::Accept(sock,client_ip,client_port);
            if(fd<0)
            {
                std::cout<<"accept error"<<std::endl;
                return;
            }
            struct epoll_event ev;
            ev.events=EPOLLIN;
            ev.data.fd=fd;
            epoll_ctl(_epfd,EPOLL_CTL_ADD,fd,&ev);
        }
        void reader(int sock){
            char buffer[1024]={0};
            ssize_t s=recv(sock,buffer,sizeof(buffer),0);
            if(s>0){
                buffer[s]=0;
                std::cout<<"client# "<<buffer<<std::endl;
                std::string respone=_fun(buffer);
                send(sock,respone.c_str(),respone.size(),0);
            }else if(s==0){
                epoll_ctl(_epfd,EPOLL_CTL_DEL,sock,nullptr);
                close(sock);
                std::cout<<"client quit!"<<std::endl;
            }else {
                epoll_ctl(_epfd,EPOLL_CTL_DEL,sock,nullptr);
                close(sock);
                std::cout<<"reader error!"<<std::endl;
            }
        }
        void handler(int n){
            for(int i=0;i<n;i++)
            {
                int sock=_revs[i].data.fd;
                int events=_revs[i].events;
                if(sock==_sock&&(events&EPOLLIN)){
                    Accept(sock);
                }else if(events&EPOLLIN){
                    reader(sock);
                }else {}
            }
        }
        void Start()
        {
            for(;;){
                int n=epoll_wait(_epfd,_revs,_num,-1);
                switch(n){
                    case 0:
                        std::cout<<"timeout..."<<std::endl;
                        break;
                    case -1:
                        std::cout<<"epoll_wait error"<<std::endl;
                        break;
                    default:
                        std::cout<<"have event ready"<<std::endl;
                        handler(n);
                        break;
                } 
            }
        }
    };
}
