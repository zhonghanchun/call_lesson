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
#include <pthread.h>
using namespace std;

class udpClient
{
public:
    udpClient(const string& serverip,const uint16_t serverport)
        :_serverip(serverip)
         ,_serverport(serverport)
         ,_sock(-1)
    {}
    void Init_Client()
    {
        _sock=socket(AF_INET,SOCK_DGRAM,0);
        if(_sock == -1)
        {
            cout<<"socket failed!"<<endl;
            exit(2);
        }
        cout<<"socket Success!"<<_sock<<endl;
    }
    static void* readMessage(void* args)
    {
        int socket=*(static_cast<int*>(args));
        pthread_detach(pthread_self());
        while(true)
        {
            char buffer[1024]={0};
            struct sockaddr_in temp;
            socklen_t len=sizeof(temp);
            size_t s=recvfrom(socket,buffer,sizeof(buffer),0,(struct sockaddr*)&temp,&len);
            if(s>=0)
            {
                buffer[s]=0;
                cout<<buffer<<endl;
            }
        }
        return nullptr;
    }
    void Run()
    {
        pthread_create(&_reader,nullptr,readMessage,&_sock); 
        cout<<"Run..."<<endl;
        struct sockaddr_in server;
        memset(&server,0,sizeof(server));
        server.sin_family=AF_INET;
        server.sin_addr.s_addr=inet_addr(_serverip.c_str());;
        server.sin_port=htons(_serverport);
        char cmdline[1024]={0};
        string res;
        while(true)
        {
            //cout<<"Please Enter#";
            //cin>>res;
            fprintf(stderr,"Enter# ");
            fflush(stderr);
            fgets(cmdline,sizeof(cmdline),stdin);
            cmdline[strlen(cmdline)-1]=0;
            res=cmdline;
            sendto(_sock,res.c_str(),res.size(),0,(struct sockaddr*)&server,sizeof(server));   
            
            // char buffer[1024]={0};
            // struct sockaddr_in tmp;
            // socklen_t len=sizeof(tmp);
            // size_t s=recvfrom(_sock,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&tmp,&len);
            // if(s>=0)
            // {
            //     buffer[s]=0;
            // }
            // cout<<buffer<<endl;
        }
    }
    ~udpClient(){}
private:
    int _sock;
    string _serverip;
    uint16_t _serverport;
    pthread_t _reader;
};
