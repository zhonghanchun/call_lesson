#pragma once 
	
#include <iostream>
#include <string>
#include <strings.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <functional>
using namespace std;

class udpServer
{
public:
    typedef function<void(int,string,uint16_t,string)> fun_t;
    udpServer(const fun_t &f, const uint16_t& port,const string& ip="0.0.0.0")
        :_f(f)
        ,_port(port)
        ,_ip(ip)
        ,_sock(-1)
    {}
    void Init_Server()
    {
        this->Sock();
        this->Bind();
    }
    void Start()
    {
        cout<<"start..."<<endl;
        char buffer[1024]={0};
        while(true)
        {
            struct sockaddr_in peer;
            memset(&peer,0,sizeof(peer));
            socklen_t len=sizeof(peer);
           ssize_t s=recvfrom(_sock,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);
           if(s>0)
           {
               buffer[s]=0;
               string client_ip=inet_ntoa(peer.sin_addr);
               uint16_t client_port=ntohs(peer.sin_port);
               string res=buffer;
               //cout<<client_ip<<":"<<client_port<<"# "<<res<<endl;
               _f(_sock,client_ip,client_port,res);     
           }
        }
    }
    void Sock()
    {
        _sock=socket(AF_INET,SOCK_DGRAM,0);
        if(_sock == -1)
        {
            cout<<"socket failed!"<<endl;
            exit(2);
        }
        cout<<"socket Success: "<<_sock<<endl;
    }
    void Bind()
    {
        struct sockaddr_in local;
        memset(&local,0,sizeof(local));
        local.sin_family=AF_INET;
        local.sin_port=htons(_port);
        local.sin_addr.s_addr=inet_addr(_ip.c_str());
        //local.sin_addr.s_addr=INADDR_ANY;
        if(bind(_sock,(struct sockaddr*)&local,sizeof(local))<0)
        {
            cout<<"bind failed!"<<endl;
            exit(3);
        } 
        cout<<"Bind Success: "<<endl;
    }
    ~udpServer(){}
private:
    fun_t _f;
    int _sock;
    uint16_t _port;
    string _ip;
};
