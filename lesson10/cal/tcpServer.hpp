#include <iostream>
#include <string>
#include <cstring>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "protol.hpp"
using namespace std;

typedef std::function<bool(const Requse& req,Repose& rep)> fun_t;

void handlerEntery(int sock,fun_t f)
{
    std::string inbuffer;
    while(true)
    {
        std::string req_text,req_str;
        if(!RecvPackage(sock,inbuffer,&req_text))return;
        if(!deLength(req_text,&req_str))return;

        Requse q;
        q.deserialize(req_str);
        
        Repose p;
        f(q,p);
        std::string resp_str;
        p.serialize(&resp_str);

        std::string send_string=enLength(resp_str);
        send(sock,send_string.c_str(),send_string.size(),0);

    }
}

class TcpServer
{
public:
    TcpServer(const uint16_t serverport)
        :_serverport(serverport)
    {}
    void Init()
    {
        _sock=socket(AF_INET,SOCK_STREAM,0);
        if(_sock<0)
        {
            cout<<"socket failed!"<<endl;
            exit(1);
        }
        cout<<"socket success!"<<endl;
        struct sockaddr_in local;
        memset(&local,0,sizeof(local));
        local.sin_family=AF_INET;
        local.sin_port=htons(_serverport);
        local.sin_addr.s_addr=INADDR_ANY;

        if(bind(_sock,(struct sockaddr*)&local,sizeof(local))<0)
        {
            cout<<"bind failed!"<<endl;
            exit(2);
        }
        cout<<"bind success!"<<endl;
        if(listen(_sock,5)<0)
        {
            cout<<"listen failed!"<<endl;
            exit(3);
        }
        cout<<"listen success!"<<endl;
    }
    void Start(fun_t f)
    {
        while(true)
        {
            struct sockaddr_in peer;
            socklen_t len=sizeof(peer);
            int sock=accept(_sock,(struct sockaddr*)&peer,&len);
            if(sock<0)
            {
                continue;
            }
            cout<<"get a new accept sock: "<<sock<<" success!"<<endl;
            pid_t id=fork();
            if(id==0)
            {
                close(_sock);
                if(fork()>0)exit(0);
                handlerEntery(sock,f);
                close(sock);
                exit(0);
            }
            close(sock);    


            pid_t ret=waitpid(id,nullptr,0);
            if(ret>0)
            {
                std::cout<<"wait success!"<<ret<<std::endl;
            }
        }
    }
    ~TcpServer(){}
private:
    int _sock;
    uint16_t _serverport;
};
