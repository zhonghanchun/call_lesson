#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "ThreadPool.hpp"
#include "Task.hpp"
using namespace std;
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
    void Start()
    {
        Thread_s::ThreadPool<Task_call::Tasks>::getInstance()->start_run();
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
            Task_call::Tasks t(Task_call::serviceIO,sock);
            Thread_s::ThreadPool<Task_call::Tasks>::getInstance()->Push(t);
            //serviceIO(sock);
        }
    }
    // void serviceIO(int sock)
    // {
    //     char buffer[1024]={0};
    //     while(true)
    //     {
    //        ssize_t s=read(sock,buffer,sizeof(buffer));
    //         if(s>0)
    //         {
    //            buffer[s]=0;
    //            cout<<"Client# "<<buffer<<endl;

    //             string res="Server# ";
    //             res+=buffer;
    //             write(sock,res.c_str(),res.size());
    //         }
    //         else if(s==0)
    //         {
    //             cout<<"Client quuit!"<<endl;
    //             break;
    //         } 
    //     }
    //     close(sock);
    // }
    ~TcpServer(){}
private:
    int _sock;
    uint16_t _serverport;
};
