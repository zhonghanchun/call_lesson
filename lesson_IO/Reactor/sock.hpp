#pragma once 
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Sock{
    private:
        int _sock;
    public:
        Sock():_sock(-1)
        {}
        ~Sock(){}
        void Socket(){
            _sock=socket(AF_INET,SOCK_STREAM,0);
            if(_sock<0){
                std::cout<<"socket create faild!"<<std::endl;
                exit(-1);
            }
            int opt=1;
            setsockopt(_sock,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt));
            std::cout<<"socket success: "<<_sock<<std::endl;
        }
        void Bind(uint16_t port){
            struct sockaddr_in local;
            memset(&local,0,sizeof(local));
            local.sin_family=AF_INET;
            local.sin_port=htons(port);
            local.sin_addr.s_addr=INADDR_ANY;
            if(bind(_sock,(struct sockaddr*)&local,sizeof(local))<0){
                std::cout<<"Bind failed!"<<std::endl;
                exit(-2);
            }    
            std::cout<<"bind success"<<std::endl;
        }
        void Listen(){
            if(listen(_sock,32)<0){
                std::cout<<"listen failed!"<<std::endl;
                exit(-3);
            }
            std::cout<<"listen success"<<std::endl;
        }
        int Accept(std::string& clientip,uint16_t& clientport,int &err){
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            int sock=accept(_sock,(struct sockaddr*)&peer,&len);
            err=errno;
            if(sock<0)
                std::cout<<"accept error , next"<<std::endl;
            else{
                std::cout<<"accept a new link success , get new sock : " <<sock<<std::endl;
                clientip=inet_ntoa(peer.sin_addr);
                clientport=ntohs(peer.sin_port);
            }
            return sock;
        }
        int fd()
        {
            return _sock;
        }
        void Close()
        {
            if(_sock!=-1)
                close(_sock);
        }
};
