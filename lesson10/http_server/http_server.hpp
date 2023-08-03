#pragma once 

#include <iostream>
#include <string.h>
#include <string>
#include <functional>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "protol.hpp"


typedef std::function<void(Reques& q,Reponse& p)> fun_t;

void handler(int sock,fun_t f)
{
    char buffer[4096]={0};
    Reques q;
    Reponse p;
   // while(true)
   // {
        ssize_t s=recv(sock,buffer,sizeof(buffer)-1,0);
        if(s>0)
        {
            buffer[s]=0;
            q.req=buffer;
            q.parse();

            f(q,p);

            send(sock,p.rep.c_str(),p.rep.size(),0);  
        }
   // }
}

class Http_Server
{
public:
    Http_Server(const int port)
        :_port(port)
    {}
    void Init()
    {
        _sock=socket(AF_INET,SOCK_STREAM,0);
        if(_sock<0)
        {
            std::cout<<"socket fail!"<<std::endl;
            exit(-1);
        }
        std::cout<<"socket success!"<<std::endl;

        struct sockaddr_in local;
        memset(&local,0,sizeof(local));
        local.sin_family=AF_INET;
        local.sin_port=htons(_port);
        local.sin_addr.s_addr=INADDR_ANY;
        
        if(bind(_sock,(struct sockaddr*)&local,sizeof(local))<0)
        {
            std::cout<<"bind fail!"<<std::endl;
            exit(-2);
        }
        std::cout<<"bind success!"<<std::endl;

        if(listen(_sock,5)<0)
        {
            std::cout<<"listen fail!"<<std::endl;
            exit(-3);
        }
        std::cout<<"listen success!"<<std::endl;

    }
    void start(fun_t f)
    {   
        for(;;)
        {
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            int sock=accept(_sock,(struct sockaddr*)&peer,&len); 
            if(sock<0)
                continue;

            std::cout<<"get a new accept sock:"<<sock<<" success!"<<std::endl;
            pid_t id=fork();
            if(id==0)
            {
                close(_sock);
                if(fork()>0)exit(0);
                handler(sock,f);
                close(sock);
                exit(0);
            }
            close(0);

            pid_t ret=waitpid(id,nullptr,0);
            if(ret>0)
                std::cout<<"wait success! "<<ret<<" "<<std::endl;
        }
    
    }
    ~Http_Server(){}
private:
    int _sock;
    int _port;
};
