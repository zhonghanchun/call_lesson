#include <iostream>
#include <signal.h>
#include "TcpServer.hpp"
#include "Task.hpp"
#include "ThreadPool.hpp"
#include "Log.hpp"
class HttpServer
{
public:
    HttpServer(int port=8081)
        :_port(port)
         ,tcp_server(nullptr)
         ,quit(false)
    {}
    void InitServer()
    {
        //信号SIGPIPE需要进行忽略，如果不忽略，在写入时候，可能直接崩溃server
        signal(SIGPIPE,SIG_IGN);
        tcp_server=TcpServer::getinstance(_port);
    }
    void Loop()
    {
        LOG(INFO,"loop begin");
        int listen_sock=tcp_server->get_listensock();
        while(!false)
        {
            struct sockaddr_in peer;
//            memset(&peer,0,sizeof(peer));
            socklen_t len=sizeof(peer);
            int sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
            if(sock<0)
            {
                continue;
            }
            LOG(INFO,"Get a new link!");
            Task task(sock);
            ThreadPool* tp=ThreadPool::getinstance();
            tp->Push_task(task);
            
           /*int* socks=new int(sock);
            pthread_t tid;
            pthread_create(&tid,nullptr,Entrance::HandlerRequest,socks);
            pthread_detach(tid);*/
        }
    }
private:
    int _port;
    TcpServer* tcp_server;
    bool quit;
};
