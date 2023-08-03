#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "Log.hpp"

class TcpServer
{
public:
    TcpServer(const int port=8081)
        :_port(port)
        ,listen_sock(-1)
    {}
    void TcpServer_init()
    {
        Socket();
        Bind();
        Listen();
        LOG(INFO,"tcp_server init ... success!");
    }
    void Socket()
    {
        listen_sock=socket(AF_INET,SOCK_STREAM,0);
        if(listen_sock<0)
        {
            LOG(FATAL,"socket error!");
            exit(1);
        }
        int opt=1;
        setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
        LOG(INFO,"create socket ... success!");
    }
    void Bind()
    {
        struct sockaddr_in local;
        memset(&local,0,sizeof local);
        local.sin_family=AF_INET;
        local.sin_port=htons(_port);
        local.sin_addr.s_addr=INADDR_ANY;
        if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local))<0)
        {
            LOG(FATAL,"bind error!");
            exit(2);
        }
        LOG(INFO,"bind socket ... success!");
    }
    void Listen()
    {
        if(listen(listen_sock,5)<0)
        {
            LOG(FATAL,"listen socket error!");
            exit(3);
        }
        LOG(INFO,"listen socket ... success!");
    }

    static TcpServer* getinstance(int port)
    {
        if(svr==nullptr)
        {
            static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER; 
            pthread_mutex_lock(&lock);
            if(svr==nullptr)
            {
                svr=new TcpServer(port);
                svr->TcpServer_init();
            }
            pthread_mutex_unlock(&lock);
        }
        return svr;
    }
    int get_listensock()
    {
        return listen_sock;
    }
    ~TcpServer()
    {
        if(listen_sock>=0)
        {
            close(listen_sock);
        }
    }
private:
    int _port;
    int listen_sock;
    static TcpServer* svr;
};
TcpServer* TcpServer::svr=nullptr;
