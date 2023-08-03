#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define INF 0
#define DBG 1
#define ERR 2
#define LOG_LEVEL DBG 

#define LOG(level,format,...) do{\
    if(level<LOG_LEVEL)break;\
    time_t t=time(nullptr);\
    struct tm* ltm=localtime(&t);\
    char tmp[32]={0};\
    strftime(tmp,31,"%H:%M:%S",ltm);\
    fprintf(stdout,"[%s:%s:%d] " format "\n",tmp,__FILE__,__LINE__,##__VA_ARGS__);\
    }while(0)

#define INF_LOG(format,...) LOG(INF,format,##__VA_ARGS__)
#define DBG_LOG(format,...) LOG(DBG,format,##__VA_ARGS__)
#define ERR_LOG(format,...) LOG(ERR,format,##__VA_ARGS__)

class Buffer
{
    private:
        std::vector<char> _buffer;
        uint64_t _reader_idx;//读偏移
        uint64_t _writer_idx;//写偏移
    public:
        Buffer():_buffer(1024),_reader_idx(0),_writer_idx(0){}
        char* Begin()
        {
            return &*_buffer.begin();
        }
        //获取当前写入启始地址
        char* WritePosition(){return Begin()+_writer_idx;}
        //获取当前读取启始地址
        char* ReadPosition(){return Begin()+_reader_idx;}
        //获取后沿空闲空间大小--写偏移之后的空闲空间
        uint64_t TailIdleSize(){return _buffer.size()-_writer_idx;}
        //获取前沿空闲空间大小--读偏移之后的空闲空间
        uint64_t HeadIdleSize(){return _reader_idx;}
        //获取可读数据大小
        uint64_t ReadAbleSize(){return _writer_idx-_reader_idx;}
        //将读偏移向后移动
        void MoveReadOffset(uint64_t len){
            assert(len<=ReadAbleSize());
            _reader_idx+=len;
        }
        //将写偏移向后移动
        void MoveWriteOffset(uint64_t len){
            assert(len<=TailIdleSize());
            _writer_idx+=len;
        }
        //确保可写空间足够（整体空间够了就移动数据，否则扩容）
        void EnsureWriteSpace(uint64_t len){
            if(len<=TailIdleSize())return;
            else if(len<=HeadIdleSize()+TailIdleSize())
            {
                uint64_t rsz=ReadAbleSize();
                std::copy(ReadPosition(),ReadPosition()+rsz,Begin());
                _reader_idx=0;
                _writer_idx=rsz;
            }
            else {
                _buffer.resize(_writer_idx+len);
            }
        }
        //写入数据
        void Write(const void* data,uint64_t len){
            EnsureWriteSpace(len);
            const char* d=(const char*)data;
            std::copy(d,d+len,WritePosition());

        }
        void WriteAndPush(const void* data,uint64_t len){
            Write(data,len);
            MoveWriteOffset(len);
        }
        void WriteString(const std::string& data)
        {
            return Write(data.c_str(),data.size());
        }
        void WriteStringAndPop(const std::string& data)
        {
            WriteString(data);
            MoveWriteOffset(data.size());
        }
        void WriteBuffer(Buffer& data)
        {
            return Write(data.ReadPosition(),data.ReadAbleSize());
        }
        void WriteBufferAndPush(Buffer& data)
        {
            WriteBuffer(data);
            MoveWriteOffset(data.ReadAbleSize());
        }
        //读取数据
        void Read(void* buf,uint64_t len){
            assert(len<=ReadAbleSize());
            std::copy(ReadPosition(),ReadPosition()+len,(char*)buf);
        }
        void ReadAndPop(void* buf,uint64_t len)
        {
            Read(buf,len);
            MoveReadOffset(len);
        }
        std::string ReadString(uint64_t len){
            assert(len<=ReadAbleSize());
            std::string str;
            str.resize(len);
            Read(&str[0],str.size());
            return str;
        }
        std::string ReadStringAndPop(uint64_t len)
        {
            std::string str=ReadString(len);
            MoveReadOffset(len);
            return str;
        }
        void* FindCRLF()
        {
            void* res=memchr(ReadPosition(),'\n',ReadAbleSize());
            return res;
        }
        std::string GetLine()
        {
            char* pos=(char*)FindCRLF();
            if(pos==nullptr)return "";
            return ReadString(pos-ReadPosition()+1);
        }
        std::string GetLineAndPop()
        {
            std::string str=GetLine();
            MoveReadOffset(str.size());
            return str;
        }
        //情况缓冲区
        void Clear(){
            _reader_idx=0;_writer_idx=0;
        }
};

class Socket
{
    private:
        int _sockfd;
    public:
        Socket():_sockfd(-1){}
        Socket(int fd):_sockfd(fd){}
        ~Socket(){Close();}
        //创建套接字
        bool Create(){
            _sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            if(_sockfd<0){
                ERR_LOG("Create socket failed!");
                return false;
            }
            return true;
        }
        //绑定地址信息
        bool Bind(const std::string& ip,uint16_t port){
            struct sockaddr_in addr;
            addr.sin_family=AF_INET;
            addr.sin_port=htons(port);
            addr.sin_addr.s_addr=inet_addr(ip.c_str());
            if(bind(_sockfd,(struct sockaddr*)&addr,sizeof(addr))<0){
                ERR_LOG("Bind failed");
                return false;
            }
            return true;
        }
        //开始监听
        bool Listen(int backlog=1024){
            if(listen(_sockfd,backlog)<0){
                ERR_LOG("Listen failed");
                return false;
            }
            return true;
        }
        //向服务器发起连接
        bool Connect(const std::string& ip,uint16_t port){
            struct sockaddr_in addr;
            addr.sin_family=AF_INET;
            addr.sin_port=htons(port);
            addr.sin_addr.s_addr=inet_addr(ip.c_str());
            if(connect(_sockfd,(struct sockaddr*)&addr,sizeof(addr))<0){
                ERR_LOG("Connect server failed");
                return false;
            }
            return true;
        }
        //获取新连接
        int Accept(){
            int newfd=accept(_sockfd,nullptr,nullptr);
            if(newfd<0){
                ERR_LOG("socket Accept failed");
                return -1;
            }
            return newfd;
        }
        //接受数据
        ssize_t Recv(void *buf,size_t len,int flag=0){
            ssize_t ret=recv(_sockfd,buf,len,flag);
            if(ret<=0){
                //EAGAIN 当前socket的接收缓冲区没有数据了，在非阻塞情况下才会有这个错误
                //EINTR 表示当前socket的阻塞等待，被信号打断了
                if(errno==EAGAIN||errno==EINTR)
                    return 0;
                ERR_LOG("socket read failed");
                return -1;
            }
            return ret;
        }
        ssize_t NonBlockRecv(void* buf,size_t len){
            return Recv(buf,len,MSG_DONTWAIT);  //MSG_DONTWAIT 表示当前接收为非阻塞
        }
        //发送数据
        ssize_t Send(const void *buf,size_t len,int flag=0){
            ssize_t ret=send(_sockfd,buf,len,flag);
            if(ret<0){
                if(errno==EAGAIN||errno==EINTR)
                    return 0;
                ERR_LOG("socket send failed");
                return -1;
            }
            return ret;
        }
        ssize_t NonBlockSend(void* buf,size_t len){
            return Send(buf,len,MSG_DONTWAIT);//MSG_DONTWAIT 表示当前发送为非阻塞
        }
        //关闭套接字
        void Close(){
            if(_sockfd!=-1){
                close(_sockfd);
                _sockfd=-1;
            }
        }
        //创建一个服务端连接
        bool CreateServer(uint16_t port,const std::string& ip="0.0.0.0",bool flag=false){
            if(Create()==false)return false;
            if(flag)
                NonBlock();
            if(Bind(ip,port)==false)return false;
            if(Listen()==false)return false;
            ReuseAddress();
            return true;
        }
        //创建一个客户端连接
        bool CreateClient(uint16_t port,const std::string& ip){
            if(Create()==false)return false;
            if(Connect(ip,port)==false)return false;
            return true;
        }
        //设置套接字选项---开启地址端口重用
        void ReuseAddress(){
            int val=1;
            setsockopt(_sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&val,sizeof(int));
            val=1;
            setsockopt(_sockfd,SOL_SOCKET,SO_REUSEPORT,(void*)&val,sizeof(int));
        }
        //设置套接字阻塞属性---设置为非阻塞
        void NonBlock(){
            int flag=fcntl(_sockfd,F_GETFL,0);
            fcntl(_sockfd,F_SETFL,flag|O_NONBLOCK);
        }
        int Fd(){return _sockfd;}
};

