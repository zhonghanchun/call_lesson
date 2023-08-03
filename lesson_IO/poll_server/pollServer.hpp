#pragma once 

#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <poll.h>
#include "sock.hpp"
namespace poll_ns
{
    const int num=2048;
    using func_t = std::function<std::string(const std::string&)>;
    class pollServer{
    private:
        int _sock;
        uint16_t _port;
        struct pollfd* _rfds;
        func_t fun;
    public:
        pollServer(func_t f,uint16_t port=8080)
            :_sock(-1),_port(port),_rfds(nullptr),fun(f)
        {}
        ~pollServer(){
            if(_sock<0){
                close(_sock);
            }
            if(_rfds){
                delete []_rfds;
            }
        }
        void Init(){
            _sock=Sock::Socket();
            Sock::Bind(_sock,_port);
            Sock::Listen(_sock);
            _rfds=new struct pollfd[num];
            for(int i=0;i<num;i++){
                _rfds[i].fd=-1;
                _rfds[i].events=0;
                _rfds[i].revents=0;
            }
            _rfds[0].fd=_sock;
            _rfds[0].events=POLLIN;
        }
        void Accept(int Sock)
        {
            std::string client_ip;
            uint16_t client_port;
            int sock=Sock::Accept(Sock,client_ip,client_port);
            if(sock<0)return;   
            std::cout<<"accept success: "<<client_ip<<":"<<client_port<<std::endl;
            
            //sock我们能直接recv/read吗？不能，整个代码，只有select有资格检测事件是否就绪
            //将新的sock托管给select
            int i=0;
            for(;i<num;i++){
                if(_rfds[i].fd!=-1)continue;
                else break;
            }
            if(i==num){
                std::cout<<"server if full,please wait"<<std::endl;
            }
            else{ 
                _rfds[i].fd=sock;
                _rfds[i].events=POLLIN;
            }
            Print();
        }
        void Recver(int pos){
            char buffer[1024]={0};
            ssize_t s=recv(_rfds[pos].fd,buffer,sizeof(buffer),0);
            if(s>0){
                buffer[s]=0;
                std::cout<<"client# "<<buffer<<std::endl;
            }else if(s==0){
                close(_rfds[pos].fd);
                _rfds[pos].fd=-1;
                _rfds[pos].events=0;
                _rfds[pos].revents=0;
                std::cout<<"client quit!"<<std::endl;
                return;
            }else {
                close(_rfds[pos].fd);
                _rfds[pos].fd=-1;
                _rfds[pos].events=0;
                _rfds[pos].revents=0;
                std::cout<<"client error"<<std::endl;
                return;
            }
            std::string response=fun(buffer);
            write(_rfds[pos].fd,response.c_str(),response.size());
            
        }
        void handler(){
            for(int i=0;i<num;i++){
                if(_rfds[i].fd==-1)continue;
                if(!(_rfds[i].events==POLLIN))continue;
                if((_rfds[i].fd==_sock)&&(_rfds[i].revents&POLLIN)){
                    Accept(_rfds[i].fd);
                }else if(_rfds[i].revents&POLLIN){
                    Recver(i);                    
                }else {

                }
            }
        }
        void Print(){
            std::cout<<"fd list: ";
            for(int i=0;i<num;i++){
                if(_rfds[i].fd!=-1){
                    std::cout<<_rfds[i].fd<<" ";
                }
            }
            std::cout<<std::endl;
        }
        void Start(){
            for(;;){
                int n=poll(_rfds,num,-1);
                switch(n){
                    case 0:
                        std::cout<<"time out..."<<std::endl;
                        break;
                    case -1:
                        std::cout<<"select error,code:"<<errno<<",err string: "<<strerror(errno)<<std::endl;
                        break;
                    default:
                        //说明有事件就绪了
                        //std::cout<<"get a new link..."<<std::endl;
                        handler();
                        break;
                } 
               //开始进行服务器的处理逻辑 
            }
        }
    };
}

