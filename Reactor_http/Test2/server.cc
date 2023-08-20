//6
#include "../source/server.hpp"

void OnConnected(const PtrConnection& conn){
    DBG_LOG("new connection %p",conn.get());
}
void OnClose(const PtrConnection& conn){
    DBG_LOG("close connection %p",conn.get());
}
void OnMessage(const PtrConnection& conn,Buffer* buf){
    DBG_LOG("%s",buf->ReadPosition());
    buf->MoveReadOffset(buf->ReadAbleSize());
    std::string str="Hello World!";
    conn->Send(&str[0],str.size());
}


int main()
{
    TcpServer server(8085);
    server.SetThreadCount(2);
    server.SetClosedCallback(OnClose);
    server.SetMessageCallback(OnMessage);
    server.SetConnectedCallback(OnConnected);
    //server.EnableInactiveRelease(10);
    server.Start();
    return 0;
}

































//5
// #include "../source/server.hpp"

// EventLoop base_loop;
// LoopThreadPool* loop_pool;

// uint64_t conn_id=0;
// std::unordered_map<uint64_t,PtrConnection> _conns;

// void ConnectionDestroy(const PtrConnection& conn){
//     _conns.erase(conn->Id());
// }
// void OnConnected(const PtrConnection& conn){
//     DBG_LOG("new connection %p",conn.get());
// }
// void OnMessage(const PtrConnection& conn,Buffer* buf){
//     DBG_LOG("%s",buf->ReadPosition());
//     buf->MoveReadOffset(buf->ReadAbleSize());
//     std::string str="Hello World!";
//     conn->Send(&str[0],str.size());
// }

// void Accept(int newfd){
//     conn_id++;
//     PtrConnection conn(new Connection(loop_pool->NextLoop(),conn_id,newfd));
//     conn->SetMessageCallback(std::bind(OnMessage,std::placeholders::_1,std::placeholders::_2));
//     conn->SetConnectedCallback(std::bind(OnConnected,std::placeholders::_1));
//     conn->SetSrcClosedCallback(std::bind(ConnectionDestroy,std::placeholders::_1));
//     conn->EnableInactiveRelease(10);
//     conn->EsTablished();
//     _conns.insert(std::make_pair(conn_id,conn));
//     DBG_LOG("new --------------------------");
// }

// int main()
// {
//     loop_pool=new LoopThreadPool(&base_loop);
//     loop_pool->SetThreadCount(2);
//     loop_pool->Create();
//     Acceptor acceptor(&base_loop,8085);
//     acceptor.SetAcceptCallback(std::bind(Accept,std::placeholders::_1));
//     acceptor.Listen();
//     base_loop.Start();
//     return 0;
// }

























//4
// #include "../source/server.hpp"

// EventLoop base_loop;
// std::vector<LoopThread> threads(2);
// int thread_id=0;

// uint64_t conn_id=0;
// std::unordered_map<uint64_t,PtrConnection> _conns;

// void ConnectionDestroy(const PtrConnection& conn){
//     _conns.erase(conn->Id());
// }
// void OnConnected(const PtrConnection& conn){
//     DBG_LOG("new connection %p",conn.get());
// }
// void OnMessage(const PtrConnection& conn,Buffer* buf){
//     DBG_LOG("%s",buf->ReadPosition());
//     buf->MoveReadOffset(buf->ReadAbleSize());
//     std::string str="Hello World!";
//     conn->Send(&str[0],str.size());
// }

// void Accept(int newfd){
//     conn_id++;
//     thread_id=(thread_id+1)%2;
//     PtrConnection conn(new Connection(threads[thread_id].GetLoop(),conn_id,newfd));
//     conn->SetMessageCallback(std::bind(OnMessage,std::placeholders::_1,std::placeholders::_2));
//     conn->SetConnectedCallback(std::bind(OnConnected,std::placeholders::_1));
//     conn->SetSrcClosedCallback(std::bind(ConnectionDestroy,std::placeholders::_1));
//     conn->EnableInactiveRelease(10);
//     conn->EsTablished();
//     _conns.insert(std::make_pair(conn_id,conn));
//     DBG_LOG("new --------------------------")
// }

// int main()
// {
//     srand(time(nullptr));
//     Acceptor acceptor(&base_loop,8085);
//     acceptor.SetAcceptCallback(std::bind(Accept,std::placeholders::_1));
//     acceptor.Listen();
//     while(1){
//         base_loop.Start();
//     }
//     return 0;
//}

























//3
// #include "../source/server.hpp"

// EventLoop loop;

// uint64_t conn_id=0;
// std::unordered_map<uint64_t,PtrConnection> _conns;

// void ConnectionDestroy(const PtrConnection& conn){
//     _conns.erase(conn->Id());
// }
// void OnConnected(const PtrConnection& conn){
//     DBG_LOG("new connection %p",conn.get());
// }
// void OnMessage(const PtrConnection& conn,Buffer* buf){
//     DBG_LOG("%s",buf->ReadPosition());
//     buf->MoveReadOffset(buf->ReadAbleSize());
//     std::string str="Hello World!";
//     conn->Send(&str[0],str.size());
// }

// void Accept(int newfd){
//     conn_id++;
//     PtrConnection conn(new Connection(&loop,conn_id,newfd));
//     conn->SetMessageCallback(std::bind(OnMessage,std::placeholders::_1,std::placeholders::_2));
//     conn->SetConnectedCallback(std::bind(OnConnected,std::placeholders::_1));
//     conn->SetSrcClosedCallback(std::bind(ConnectionDestroy,std::placeholders::_1));
//     conn->EnableInactiveRelease(10);
//     conn->EsTablished();
//     _conns.insert(std::make_pair(conn_id,conn));
// }

// int main()
// {
//     srand(time(nullptr));
//     Acceptor acceptor(&loop,8085);
//     acceptor.SetAcceptCallback(std::bind(Accept,std::placeholders::_1));
//     acceptor.Listen();
//     while(1){
//         loop.Start();
//     }
//     return 0;
// }






























//2
// #include "../source/server.hpp"

// uint64_t conn_id=0;
// std::unordered_map<uint64_t,PtrConnection> _conns;

// void ConnectionDestroy(const PtrConnection& conn){
//     _conns.erase(conn->Id());
// }
// void OnConnected(const PtrConnection& conn){
//     DBG_LOG("new connection %p",conn.get());
// }
// void OnMessage(const PtrConnection& conn,Buffer* buf){
//     DBG_LOG("%s",buf->ReadPosition());
//     buf->MoveReadOffset(buf->ReadAbleSize());
//     std::string str="Hello World!";
//     conn->Send(&str[0],str.size());
// }

// void Accept(EventLoop* loop,Channel* lst_channel){
//     int fd=lst_channel->Fd();
//     int newfd=accept(fd,nullptr,nullptr);
//     if(newfd<0){return;}
//     conn_id++;
//     PtrConnection conn(new Connection(loop,conn_id,newfd));
//     conn->SetMessageCallback(std::bind(OnMessage,std::placeholders::_1,std::placeholders::_2));
//     conn->SetConnectedCallback(std::bind(OnConnected,std::placeholders::_1));
//     conn->SetSrcClosedCallback(std::bind(ConnectionDestroy,std::placeholders::_1));
//     conn->EnableInactiveRelease(10);
//     conn->EsTablished();
//     _conns.insert(std::make_pair(conn_id,conn));
// }

// int main()
// {
//     srand(time(nullptr));
//     EventLoop loop;
//     Socket sock;
//     sock.CreateServer(8085);
//     Channel channel(&loop,sock.Fd());
//     channel.SetReadCallback(std::bind(Accept,&loop,&channel));
//     channel.EnableRead();
//     while(1){
//         loop.Start();
//     }
//     sock.Close();
//     return 0;
// }
























//1
// #include "../source/server.hpp"

// void HandleClose(Channel* channel){
//     DBG_LOG("close :%d",channel->Fd());
//     channel->Remove();

//     delete channel;
// }

// void HandleRead(Channel* channel){
//     int fd=channel->Fd();
//     char buf[1024]={0};
//     int ret=recv(fd,buf,1023,0);
//     if(ret<=0){
//         return HandleClose(channel);
//     }
//     channel->EnableWrite();
//     DBG_LOG("%s",buf);
// }
// void HandleWrite(Channel* channel){
//     int fd=channel->Fd();
//     const char* data="今天天气不错!";
//     int ret=send(fd,data,strlen(data),0);
//     if(ret<0){
//         return HandleClose(channel);
//     }
//     channel->DisableWrite();
// }
// void HandleError(Channel* channel){
//     return HandleClose(channel);
// }
// void HandleEvent(EventLoop* loop,uint64_t timerid,Channel* channel){
//     loop->TimerRefresh(timerid);
// }

// void Accept(EventLoop* loop,Channel* lst_channel){
//     int fd=lst_channel->Fd();
//     int newfd=accept(fd,nullptr,nullptr);
//     uint64_t timerid=rand()%10000;
//     if(newfd<0){return;}
//     Channel* channel=new Channel(loop,newfd);
//     channel->SetReadCallback(std::bind(HandleRead,channel));
//     channel->SetWriteCallback(std::bind(HandleWrite,channel));
//     channel->SetCloseCallback(std::bind(HandleClose,channel));
//     channel->SetErrorCallback(std::bind(HandleError,channel));
//     channel->SetEventCallback(std::bind(HandleEvent,loop,timerid,channel));

//     loop->TimerAdd(timerid,10,std::bind(HandleClose,channel));
//     channel->EnableRead();

// }

// int main()
// {
//     srand(time(nullptr));
//     EventLoop loop;
//     Socket sock;
//     sock.CreateServer(8085);
//     Channel channel(&loop,sock.Fd());
//     channel.SetReadCallback(std::bind(Accept,&loop,&channel));
//     channel.EnableRead();
//     while(1){
//         loop.Start();
//     }
//     sock.Close();
//     return 0;
// }
