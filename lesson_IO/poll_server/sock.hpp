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
        Sock(){}
        ~Sock(){}
        static int Socket(){
            int listen_socket=socket(AF_INET,SOCK_STREAM,0);
            if(listen_socket<0){
                std::cout<<"socket create faild!"<<std::endl;
                exit(-1);
            }
            int opt=1;
            setsockopt(listen_socket,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt));
            std::cout<<"socket success: "<<listen_socket<<std::endl;
            return listen_socket;
        }
        static void Bind(int sock,uint16_t port){
            struct sockaddr_in local;
            memset(&local,0,sizeof(local));
            local.sin_family=AF_INET;
            local.sin_port=htons(port);
            local.sin_addr.s_addr=INADDR_ANY;
            if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0){
                std::cout<<"Bind failed!"<<std::endl;
                exit(-2);
            }    
            std::cout<<"bind success"<<std::endl;
        }
        static void Listen(int sock){
            if(listen(sock,32)<0){
                std::cout<<"listen failed!"<<std::endl;
                exit(-3);
            }
            std::cout<<"listen success"<<std::endl;
        }
        static int Accept(int sock,std::string& clientip,uint16_t& clientport){
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            int Sock=accept(sock,(struct sockaddr*)&peer,&len);
            if(Sock<0)
                std::cout<<"accept error , next"<<std::endl;
            else{
                std::cout<<"accept a new link success , get new sock : " <<Sock<<std::endl;
                clientip=inet_ntoa(peer.sin_addr);
                clientport=ntohs(peer.sin_port);
            }
            return Sock;
        }
};
