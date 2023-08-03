#pragma once 

#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include "sock.hpp"
namespace select_ns
{
    const int fdnum=sizeof(fd_set)*8;
    using func_t = std::function<std::string(const std::string&)>;
    class SelectServer{
    private:
        int _sock;
        uint16_t _port;
        int* fdarray;
        func_t fun;
    public:
        SelectServer(func_t f,uint16_t port=8080)
            :_sock(-1),_port(port),fdarray(nullptr),fun(f)
        {}
        ~SelectServer(){
            if(_sock<0){
                close(_sock);
            }
            if(fdarray){
                delete []fdarray;
            }
        }
        void Init(){
            _sock=Sock::Socket();
            Sock::Bind(_sock,_port);
            Sock::Listen(_sock);
            fdarray=new int[fdnum];
            for(int i=0;i<fdnum;i++){
                fdarray[i]=-1;
            }
            fdarray[0]=_sock;
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
            for(;i<fdnum;i++){
                if(fdarray[i]!=-1)continue;
                else break;
            }
            if(i==fdnum){
                std::cout<<"server if full,please wait"<<std::endl;
            }
            else 
                fdarray[i]=sock;
            Print();
        }
        void Recver(int sock,int pos){
            char buffer[1024]={0};
            ssize_t s=recv(sock,buffer,sizeof(buffer),0);
            if(s>0){
                buffer[s]=0;
                std::cout<<"client# "<<buffer<<std::endl;
            }else if(s==0){
                fdarray[pos]=-1;
                close(sock);
                std::cout<<"client quit!"<<std::endl;
                return;
            }else {
                fdarray[pos]=-1;
                close(sock);
                std::cout<<"client error"<<std::endl;
                return;
            }
            std::string response=fun(buffer);
            write(sock,response.c_str(),response.size());
            
        }
        void handler(fd_set &rfds){
            for(int i=0;i<fdnum;i++){
                if(fdarray[i]==-1)continue;
                if(FD_ISSET(fdarray[i],&rfds)&&fdarray[i]==_sock){
                    Accept(fdarray[i]);
                }else if(FD_ISSET(fdarray[i],&rfds)){
                    Recver(fdarray[i],i);                    
                }else {

                }
            }
        }
        void Print(){
            std::cout<<"fd list: ";
            for(int i=0;i<fdnum;i++){
                if(fdarray[i]!=-1){
                    std::cout<<fdarray[i]<<" ";
                }
            }
            std::cout<<std::endl;
        }
        void Start(){
            for(;;){
                fd_set rfds;
                FD_ZERO(&rfds);
                int fdmax=fdarray[0];
                for(int i=0;i<fdnum;i++){
                    if(fdarray[i]==-1)continue;
                    FD_SET(fdarray[i],&rfds);
                    if(fdarray[i]>fdmax){
                            fdmax=fdarray[i];
                    }
                }
                std::cout<<"max fd: "<<fdmax<<std::endl; 
                //_sock添加到读文件描述符集中
                //FD_SET(_sock,&rfds);
                //struct timeval time={1,0};
                //int n=select(_sock,&rfds,nullptr,nullptr,&time);
                int n=select(fdmax+1,&rfds,nullptr,nullptr,nullptr);
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
                        handler(rfds);
                        break;
                } 
               //开始进行服务器的处理逻辑 
            }
        }
    };
}

