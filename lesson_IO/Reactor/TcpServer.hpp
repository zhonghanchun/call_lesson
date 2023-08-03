#pragma once 
#include <iostream>
#include <unordered_map>
#include <assert.h>
#include <functional>
#include "sock.hpp"
#include "Epoller.hpp"
#include "Util.hpp"
#include "Protocol.hpp"

namespace tcpserver
{
    class Connection;
    class TcpServer;

    using func_t = std::function<void (Connection*)>;
    using handler_t = std::function<void (const std::string&)>;
    class Connection
    {
        public:
            void Register(func_t recver,func_t sender,func_t excepter)
            {
                _recver=recver;_sender=sender;_excepter=excepter;
            }
        public:
            Connection(int sock,TcpServer* tsp)
                :_sock(sock),_tsp(tsp)
            {}
            ~Connection(){}
            void Close()
            {
                close(_sock);
            }
        public:
            int _sock;
            std::string inbuffer;// 输入缓冲区
            std::string outbuffer;// 输出缓冲区
            
            func_t _recver;// 从sock_读
            func_t _sender;// 向sock_写
            func_t _excepter;// 处理sock_ IO的时候上面的异常事件

            TcpServer* _tsp;
    };
    class TcpServer
    {
        private:
            uint16_t _port;
            Sock _sock;
            Epoller _epfd;
            std::unordered_map<int,Connection*> _connections;
            struct epoll_event* _evens;
            func_t _service;
        public:
            TcpServer(func_t func,uint16_t port=8080):_port(port),_evens(nullptr),_service(func)
            {}
            ~TcpServer(){
                _sock.Close();
                _epfd.Close();
                if(_evens)
                    delete []_evens;
            }
            void Init()
            {
                //创建socket
                _sock.Socket();
                _sock.Bind(_port);
                _sock.Listen();
                //创建epoll
                _epfd.Create();
                // 3. 将目前唯一的一个sock，添加到epoller中， 之前需要先将对应的fd设置成为非阻塞
                //listensock_也是一个socket啊，也要看做成为一个Connection
                AddConnection(_sock.fd(),EPOLLIN|EPOLLET,std::bind(&TcpServer::Accepter,this,std::placeholders::_1),nullptr,nullptr);
                _evens=new struct epoll_event[64];
            }
        private:
            void Recver(Connection* conn)
            {
                char buffer[1024]={0};
                while(true){
                    ssize_t s=recv(conn->_sock,buffer,sizeof(buffer),0);
                    if(s>0){
                        buffer[s]=0;
                        conn->inbuffer+=buffer;
                        _service(conn);
                    }else if(s==0){
                        if(conn->_excepter)
                            conn->_excepter(conn);
                        break;
                    }
                    else {
                        if(errno==EAGAIN || errno==EWOULDBLOCK)break;
                        else if(errno==EINTR)continue;
                        else {
                            if(conn->_excepter)
                            {
                                conn->_excepter(conn);
                                break;
                            }
                        }
                    }
                }
            }
            void Sender(Connection* conn)
            {
                while(true)
                {
                    ssize_t s=send(conn->_sock,conn->outbuffer.c_str(),conn->outbuffer.size(),0);
                    if(s>0){
                        if(conn->outbuffer.empty()){
                            conn->_tsp->EnableEeadWrite(conn,true,false);
                            break;
                        }
                        else conn->outbuffer.erase(0,s);
                    }else{
                        if(errno==EAGAIN || errno==EWOULDBLOCK)break;
                        else if(errno==EINTR)continue;
                        else {
                            if(conn->_excepter){
                                conn->_excepter(conn);
                                break;
                            }
                        }       
                    }
                }
            }
            void Excepter(Connection* conn)
            {
                std::cout<<"Excepter begin"<<std::endl;
                int sock=conn->_sock;
                _epfd.Control(conn->_sock,0,EPOLL_CTL_DEL);
                _connections.erase(conn->_sock);
                conn->Close();
                delete conn;
                std::cout<<"关闭:"<<sock<<"文件描述符，已经清除所有资源"<<std::endl;
            }
            void Accepter(Connection* conn){
                for(;;)
                {
                    std::string client_ip;
                    uint16_t client_port;
                    int err=0;
                    int sock=_sock.Accept(client_ip,client_port,err);
                    
                    if(sock>0)
                    {
                        AddConnection(sock,EPOLLIN|EPOLLET,std::bind(&TcpServer::Recver,this,std::placeholders::_1)
                                ,std::bind(&TcpServer::Sender,this,std::placeholders::_1)
                                ,std::bind(&TcpServer::Excepter,this,std::placeholders::_1));
                        std::cout<<"get a new link info: "<<client_ip<<":"<<client_port<<std::endl;
                    }else {
                        if(err==EAGAIN || err==EWOULDBLOCK)break;
                        else if(err==EINTR)continue;
                        else break;
                    }
                }
            }
            void AddConnection(int sock,uint32_t events,func_t recver,func_t sender,func_t excepter)
            {
                // 1. 首先要为该sock创建Connection，并初始化，并添加到connections_
                if(events&EPOLLET){
                    Util::SetNonBlock(sock);
                }    
                Connection* conn=new Connection(sock,this);
                conn->Register(recver,sender,excepter);

                bool ret=_epfd.AddEvent(sock,events);
                assert(ret);

                _connections.insert(std::make_pair(sock,conn));
                std::cout<<"add new sock: "<<sock<<", in epoll and unordered_map"<<std::endl;
            }
            void Loop(int timeout)
            {
                int n=_epfd.Wait(_evens,64,timeout);  
                for(int i=0;i<n;i++)
                {
                    int sock=_evens[i].data.fd;
                    uint32_t events=_evens[i].events;

                    //将所有异常问题全部转化为读写问题
                    if(events&EPOLLERR) events|=(EPOLLIN|EPOLLOUT);
                    if(events&EPOLLHUP) events|=(EPOLLIN|EPOLLOUT);
                    //事件就绪
                    if((events & EPOLLIN) && IsConnectionExists(sock) && _connections[sock]->_recver)
                        _connections[sock]->_recver(_connections[sock]);
                    if((events & EPOLLIN) && IsConnectionExists(sock) && _connections[sock]->_sender)
                        _connections[sock]->_sender(_connections[sock]);
                }
            }
            bool IsConnectionExists(int sock)
            {
                auto iter=_connections.find(sock);
                return iter!=_connections.end();
            }
        public:
            void EnableEeadWrite(Connection* conn,bool Read,bool Write)
            {
                uint32_t events=(Read ? EPOLLIN:0)|(Write ? EPOLLOUT:0)|EPOLLET;
                _epfd.Control(conn->_sock,events,EPOLL_CTL_MOD);
            }
            //事件派发器
            void Dispatcher()
            {
                int timeout=-1;
                while(true)
                {
                    Loop(timeout);
                    std::cout<<"timeout..."<<std::endl;
                }
            }

    };
}
