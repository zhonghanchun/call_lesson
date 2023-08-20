#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <functional>
#include <thread>
#include <typeinfo>
#include <assert.h>
#include <string.h>
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

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
    fprintf(stdout,"[%p %s:%s:%d] " format "\n",(void*)pthread_self(),tmp,__FILE__,__LINE__,##__VA_ARGS__);\
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
            if(len==0)return;
            assert(len<=TailIdleSize());
            _writer_idx+=len;
        }
        //确保可写空间足够（整体空间够了就移动数据，否则扩容）
        void EnsureWriteSpace(uint64_t len){
            if(len==0)return;
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
            if(len==0)return;
            EnsureWriteSpace(len);
            const char* d=(const char*)data;
            std::copy(d,d+len,WritePosition());

        }
        void WriteAndPush(const void* data,uint64_t len){
            if(len==0)return;
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
            if(len==0)return;
            assert(len<=ReadAbleSize());
            std::copy(ReadPosition(),ReadPosition()+len,(char*)buf);
        }
        void ReadAndPop(void* buf,uint64_t len)
        {
            if(len==0)return;
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

using EventCallback = std::function<void()>;
class Poller;
class EventLoop;
class Channel
{
    private:
        int _fd;
        EventLoop* _loop;
        //当前需要监控事件
        uint32_t _events;
        //当前连接触发事件
        uint32_t _revents;
        // 可读事件被触发的回调函数
        EventCallback _read_callback;
        // 可写事件被触发的回调函数
        EventCallback _write_callback;
        // 错误事件被触发的回调函数
        EventCallback _error_callback;
        // 连接关闭事件被触发的回调函数
        EventCallback _close_callback;
        // 任意事件被触发的回调函数
        EventCallback _event_callback;
    public:
        Channel(EventLoop* loop,int fd):_fd(fd),_events(0),_revents(0),_loop(loop) {}
        int Fd(){ return _fd; }
        uint32_t Event(){ return _events; }
        void SetRevents(uint32_t events){ _revents=events; }
        void SetReadCallback(const EventCallback& cb){ _read_callback=cb; }
        void SetWriteCallback(const EventCallback& cb){ _write_callback=cb; }
        void SetErrorCallback(const EventCallback& cb){ _error_callback=cb; }
        void SetCloseCallback(const EventCallback& cb){ _close_callback=cb; }
        void SetEventCallback(const EventCallback& cb){ _event_callback=cb; }
        // 当前是否监控了可读
        bool ReadAble(){ return (_events & EPOLLIN); }
        // 当前是否监控了可写
        bool WriteAble(){return (_events & EPOLLOUT); }
        // 启动读事件监控
        void EnableRead(){ _events|=EPOLLIN; Update();}
        // 启动写事件监控
        void EnableWrite(){ _events|=EPOLLOUT;Update(); }
        // 关闭读事件监控
        void DisableRead(){ _events &= (~EPOLLIN);Update(); }
        // 关闭写事件监控
        void DisableWrite(){ _events &= (~EPOLLOUT);Update(); }
        // 关闭所有事件监控
        void DisableAll(){ _events=0; Update(); }
        // 移除监控
        void Remove();
        void Update();
        //事件处理，一旦连接触发了事件，就调用这个函数，自己触发了什么事件如何处理自己决定
        void HandleEvent(){
            if((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI)){
                //不管任何事件，都调用的回调函数
                if(_event_callback) _event_callback();//放到事件处理完毕后调用，刷新活跃度
                if(_read_callback) _read_callback();
            }
            //有可能会释放连接的操作事件，一次只能处理一个
            if(_revents & EPOLLOUT){
                 //不管任何事件，都调用的回调函数
                if(_event_callback) _event_callback();//放到事件处理完毕后调用，刷新活跃度
                if(_write_callback) _write_callback();
            }else if(_revents & EPOLLERR){
                if(_event_callback) _event_callback();//一旦出错，就会释放连接，因此要放到前边调用任意回调
                if(_error_callback) _error_callback();
            }else if(_revents & EPOLLHUP){
                if(_event_callback) _event_callback();//一旦出错，就会释放连接，因此要放到前边调用任意回调
                if(_close_callback) _close_callback();
            }
        }
};

class Poller
{
    private:
        int _epfd;
        struct epoll_event _evs[1024];
        std::unordered_map<int,Channel*> _channels;
    private:
        //对epoll的直接操作
        void Update(Channel* channel,int op){
            int fd=channel->Fd();
            struct epoll_event ev;
            ev.data.fd=fd;
            ev.events=channel->Event();
            int ret=epoll_ctl(_epfd,op,fd,&ev);
            if(ret<0){
                ERR_LOG("epoll_ctl failed");
            }
            return;
        }
        //判断一个Channel是否已经添加了事件监控
        bool HasChannel(Channel* channel){
            auto it = _channels.find(channel->Fd());
            if(it == _channels.end()){
                return false;
            }
            return true;
        }
    public:
        Poller(){
            _epfd=epoll_create(1024);
            if(_epfd<0){
                ERR_LOG("epoll_create failed");
                abort();
            }
        }
        //添加或修改监控事件
        void UpdateEvent(Channel* channel){
            bool ret=HasChannel(channel);
            if(ret==false){
                _channels.insert(std::make_pair(channel->Fd(),channel));
                return Update(channel,EPOLL_CTL_ADD);
            }
            return Update(channel,EPOLL_CTL_MOD);
        }
        //移除监控
        void RemoveEvent(Channel* channel){
            auto it=_channels.find(channel->Fd());
            if(it!=_channels.end()){
                _channels.erase(it);
            }
            Update(channel,EPOLL_CTL_DEL);
        }
        //开始监控，返回活跃连接
        void Poll(std::vector<Channel*>* active){
            int nfds=epoll_wait(_epfd,_evs,1024,-1);
            if(nfds<0){
                if(errno==EINTR){
                    return;
                }
                ERR_LOG("epoll_wait error: %s",strerror(errno));
                abort();
            }
            for(int i=0;i<nfds;i++)
            {
                auto it = _channels.find(_evs[i].data.fd);
                assert(it != _channels.end());
                it->second->SetRevents(_evs[i].events);
                active->push_back(it->second);
            }
        }
};

using TaskFunc=std::function<void()>;
using ReleaseFunc=std::function<void()>;
class TimerTask
{
    private:
        uint64_t _id;//定时器任务对象ID
        uint32_t _timeout;//定时任务超时时间
        TaskFunc _task_cb;//定时器对象要执行的定时任务
        ReleaseFunc _release;//用于删除Timer Wheel中保存的定时器对象信息
        bool _cancel;
    public:
        TimerTask(uint64_t id,uint32_t timeout,const TaskFunc& cb)
            :_id(id),_timeout(timeout),_task_cb(cb),_cancel(false){}
       ~TimerTask(){
           if(_cancel==false)
                _task_cb();
           _release();
       } 
       void SetRelease(const ReleaseFunc& cb){_release=cb;}
       uint32_t DelayTime(){return _timeout;}
       void SetCancel(){_cancel=true;}
};

using PtrTask=std::shared_ptr<TimerTask>;
using WeakTask=std::weak_ptr<TimerTask>;
class TimerWheel
{
    private:
        int _tick;//走到哪里释放对应的执行任务
        int _capacity;
        std::vector<std::vector<PtrTask>> _wheel;
        std::unordered_map<uint64_t,WeakTask> _timers;

        EventLoop* _loop;
        int _timerfd;
        std::unique_ptr<Channel> _timer_channel;
    private:
        void Remove(uint64_t id)
        {
            auto it=_timers.find(id);
            if(it!=_timers.end())
                _timers.erase(it);
        }
        static int CreateTimerfd()
        {
            int timerfd=timerfd_create(CLOCK_MONOTONIC,0);
            if(timerfd<0)
            {
                ERR_LOG("timerfd create error");
                abort();
            }
            struct itimerspec itim;
            itim.it_value.tv_sec=1;
            itim.it_value.tv_nsec=0;
            itim.it_interval.tv_sec=1;
            itim.it_interval.tv_nsec=0;
            timerfd_settime(timerfd,0,&itim,nullptr);
            return timerfd;
        }
        int ReadTimerfd()
        {
            uint64_t times=0;
            ssize_t s=read(_timerfd,&times,sizeof times);
            if(s<0)
            {
                ERR_LOG("ReadTimerfd error");
                abort();
            }
            return times;
        }
        //这个函数应该每秒被执行一次，相当于秒钟向后走了一步
        void RunTimerTask()
        {
            _tick=(_tick+1)%_capacity;
            _wheel[_tick].clear();
        }
        void  OnTime()
        {
            ReadTimerfd();
            RunTimerTask(); 
        }
        //添加定时任务
        void TimerAddInLoop(uint64_t id,uint32_t timeout,const TaskFunc& cb)
        {
            PtrTask pt(new TimerTask(id,timeout,cb));
            pt->SetRelease(std::bind(&TimerWheel::Remove,this,id));
            int pos=(_tick+timeout)%_capacity;
            _wheel[pos].push_back(pt);
            _timers[id]=WeakTask(pt);
        }
        //刷新/延迟定时任务
        void TimerRefreshInLoop(uint64_t id)
        {
            auto it=_timers.find(id);
            if(it==_timers.end())
                return;
            PtrTask pt=it->second.lock();
            int delay=pt->DelayTime();
            int pos=(_tick+delay)%_capacity;
            _wheel[pos].push_back(pt);
        }
        void TimerCancelInLoop(uint64_t id)
        {
            auto it=_timers.find(id);
            if(it==_timers.end())
                return;
            PtrTask pt=it->second.lock();
            if(pt)pt->SetCancel();
        }
    public:
        TimerWheel(EventLoop* loop):_tick(0),_capacity(60),_wheel(_capacity),
                                    _loop(loop),_timerfd(TimerWheel::CreateTimerfd()),_timer_channel(new Channel(_loop,_timerfd))
        {
           _timer_channel->SetReadCallback(std::bind(&TimerWheel::OnTime,this));
           _timer_channel->EnableRead();
        }
        void TimerAdd(uint64_t id,uint32_t timeout,const TaskFunc& cb);
        void TimerRefresh(uint64_t id);
        void TimerCancel(uint64_t id);

        bool HasTimer(uint64_t id){
            auto it=_timers.find(id);
            if(it==_timers.end()){
                return false;
            }
            return true;
        } 
};

using Functor=std::function<void()>;
class EventLoop
{
    private:
        std::thread::id _thread_id;//线程ID
        int _event_fd;//eventfd唤醒IO事件监控有可能导致的阻塞
        std::unique_ptr<Channel> _eventfd_channel;
        Poller _poller;//进行所有描述符的监控
        std::vector<Functor> _tasks;//任务池
        std::mutex _mutex;//实现任务池的操作安全
        TimerWheel _timer_wheel;
    private:
        // 执行任务池中的所有任务
        void RunAllTask(){
            std::vector<Functor> functor;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _tasks.swap(functor);
            }
            for(auto& f : functor){
                f();
            }
            return;
        }
        static int CreateEventFd(){
            int efd=eventfd(0,EFD_CLOEXEC|EFD_NONBLOCK);
            if(efd<0){
                ERR_LOG("create eventfd failed!");
                abort();
            }
            return efd;
        }
        void ReadEventFd(){
            uint64_t res=0;
            int ret=read(_event_fd,&res,sizeof(res));
            if(ret<0){
                if(errno == EINTR || errno==EAGAIN )return;
                ERR_LOG("read eventfd failed");
                abort();
            }
            return ;
        }
        void WeakUpdateFd(){
            uint64_t res=0;
            int ret=write(_event_fd,&res,sizeof(res));
            if(ret<0){
                if(errno == EINTR || errno==EAGAIN )return;
                ERR_LOG("write eventfd failed");
                abort();
            }
            return ;
        }
    public:
        EventLoop():_thread_id(std::this_thread::get_id()),
                    _event_fd(EventLoop::CreateEventFd()),
                    _eventfd_channel(new Channel(this,_event_fd)),
                    _timer_wheel(this)
        {
            //给eventfd添加可读事件回调函数，读取eventfd事件通知次数
            _eventfd_channel->SetReadCallback(std::bind(&EventLoop::ReadEventFd,this));
            //启动eventfd的读事件监控
            _eventfd_channel->EnableRead();
        }
        // 判断将要执行的任务是否处于当前线程中，如果是则执行，不是则压入队列
        void RunInLoop(const Functor& cb){
            if(IsLoop()){
                return cb();
            }
            return QueueInLoop(cb);
        }
        // 将任务压入任务池
        void QueueInLoop(const Functor& cb){
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _tasks.push_back(cb);
            }
            WeakUpdateFd();
        }
        // 用于判断当前线程是否是EventLoop对应的线程
        bool IsLoop(){
            return _thread_id==std::this_thread::get_id();
        }
        void AsserInLoop()
        {
            assert(_thread_id==std::this_thread::get_id());
        }
        // 添加或修改描述符的事件监控
        void UpdateEvent(Channel* channel){ return _poller.UpdateEvent(channel); }
        // 移除描述符的监控
        void RemoveEvent(Channel* channel){ return _poller.RemoveEvent(channel); }
        // 三步走->事件监控->就绪事件处理->执行任务
        void Start(){
            while(1){
                std::vector<Channel*> actives;
                _poller.Poll(&actives);
                for(auto& channel : actives)
                {
                    channel->HandleEvent();
                }
                RunAllTask();
            }
        }
        void TimerAdd(uint64_t id,uint32_t timeout,const TaskFunc& cb){ return _timer_wheel.TimerAdd(id,timeout,cb); }
        void TimerRefresh(uint64_t id){ return _timer_wheel.TimerRefresh(id); }
        void TimerCancel(uint64_t id){ return _timer_wheel.TimerCancel(id); }
        bool HasTimer(uint64_t id){ return _timer_wheel.HasTimer(id); }
};

class LoopThread
{
    private:
        //用于实现_loop获取的同步关系，避免线程创建了，但是_loop还没实例化之前去获取_loop
        std::mutex _mutex;       //互斥锁
        std::condition_variable _cond;//条件变量
        EventLoop* _loop;        //EventLoop指针变量，这个对象需要在线程实例化
        std::thread _thread;     //EventLoop对应的线程
    private:
        //实例化EventLoop对象，唤醒_cond上有可能阻塞的线程，并且开始运行EventLoop模块的功能
        void ThreadEntry(){
            EventLoop loop;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _loop=&loop;
                _cond.notify_all();
            }
            loop.Start();
        }
    public:
        //创建线程，设定线程入口函数
        LoopThread()
            :_loop(nullptr),_thread(std::thread(&LoopThread::ThreadEntry,this))
        {}
        //返回当前线程关联的EventLoop对象指针
        EventLoop* GetLoop(){
            EventLoop* loop=nullptr;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _cond.wait(lock,[&](){return _loop!=nullptr; }); //loop为nullptr就一直阻塞
                loop=_loop;
            }
            return loop;
        }
};

class LoopThreadPool
{
    private:
        int _thread_count; //从属线程的数量
        int _next_loop_idx;
        EventLoop* _base_loop;//主EventLoop，运行在主线程，从属线程为0，则所有操作都在_base_loop中进行
        std::vector<LoopThread*> _threads;//保存所有的LoopThread对象
        std::vector<EventLoop*> _loops;//从属线程数量大于0则从_loops中进行线程EventLoop分配
    public:
        LoopThreadPool(EventLoop* base_loop)
            :_thread_count(0),_next_loop_idx(0),_base_loop(base_loop)
        {}
        //设置线程数量
        void SetThreadCount(int count){
            _thread_count=count;
        }
        //创建所有从属线程并启动
        void Create(){
            if(_thread_count>0){
                _threads.resize(_thread_count); 
                _loops.resize(_thread_count); 
                for(int i=0;i<_thread_count;i++){
                    _threads[i]=new LoopThread();
                    _loops[i]=_threads[i]->GetLoop();
                }
            }
            return;
        }
        EventLoop* NextLoop(){
            if(_thread_count==0){
                return _base_loop;
            }
            _next_loop_idx=(_next_loop_idx+1)%_thread_count;
            return _loops[_next_loop_idx];
        }
};

class Any
{
    private:
        class holder
        {
            public:
                virtual ~holder(){}
                virtual const std::type_info& type()=0;
                virtual holder* clone()=0;
        };
        template<class T>
        class placeholder : public holder
        {
            public:
                placeholder(const T&val):_val(val){}
                virtual const std::type_info& type(){return typeid(T);}
                virtual holder* clone(){
                    return new placeholder(_val);
                }
            public:
                T _val;
        };
        holder* _content;
    public:
        Any():_content(nullptr){}
        template<class T>
        Any(const T& val):_content(new placeholder<T>(val))
        {}
        Any(const Any& other):_content(other._content ? other._content->clone() : nullptr)
        {}
        ~Any(){delete _content;}
        
        Any& swap(Any& other)
        {
            std::swap(_content,other._content);
            return *this;
        }

        template<class T>
        T* get(){
            assert(typeid(T)==_content->type());
            return &((placeholder<T>*)_content)->_val;
        }

        template<class T>
        Any& operator=(const T& val){
            Any(val).swap(*this);
            return *this;
        }
        Any& operator=(const Any& other)
        {
            Any(other).swap(*this);
            return *this;
        }
};

class Connection;
// DISCONNECTED -- 连接关闭状态     CONNECTING -- 连接建立成功-待处理状态
// CONNECTED -- 连接建立完成，各种设置已完成，可以通信的状态；
// DISCONNECTING -- 待关闭状态
typedef enum { DISCONNECTED,CONNECTING,CONNECTED,DISCONNECTING }ConnStatu;
using PtrConnection=std::shared_ptr<Connection>;
class Connection : public std::enable_shared_from_this<Connection>
{
    private:
        uint64_t _conn_id;   //连接唯一ID，便于连接的管理和查找
        int _sockfd;         //连接关联的文件描述符
        bool _enable_inactive_release;  //连接是否启动非活跃销毁的判断标志，默认false
        EventLoop* _loop;
        ConnStatu _statu;     //状态
        Socket _socket;       //套接字管理
        Channel _channel;     //连接事件管理
        Buffer _in_buffer;    //输入缓冲区---存放从socket中读取数据
        Buffer _out_buffer;   //输出缓冲区---存放要发送给对端的数据
        Any _context;         //请求的接收处理上下文

        using ConnectedCallback=std::function<void(const PtrConnection&)>;
        using MessageCallback=std::function<void(const PtrConnection&,Buffer*)>;
        using ClosedCallback=std::function<void(const PtrConnection&)>;
        using AnyEventCallback=std::function<void(const PtrConnection&)>;

        ConnectedCallback _connected_callback;
        MessageCallback _message_callback;
        ClosedCallback _cloesd_callback;
        AnyEventCallback _event_callback;
        //组件内的连接关闭回调--组件内设置，因为服务器组件内会把所有的连接管理起来，一旦某个连接关闭就应该从管理的地方移除掉自己的信息
        ClosedCallback _server_closed_callbck;
    private:
        //描述符可读事件触发后调用的函数，接收socket数据放到接收缓冲区中，然后调用_message_callback
        void HandleRead(){
            char buf[65536];
            int ret=_socket.NonBlockRecv(buf,65535);
            if(ret<0){
                return ShutdownInLoop();
            }
            _in_buffer.WriteAndPush(buf,ret);
            if(_in_buffer.ReadAbleSize()>0){
                //shared_from_this -- 从当前对象自身获取自身的shared_ptr管理对象
                _message_callback(shared_from_this(),&_in_buffer);
            }
        }
        //描述符可写事件触发后调用的函数，将发送缓冲区的数据进行发送
        void HandleWrite(){
            int ret=_socket.NonBlockSend(_out_buffer.ReadPosition(),_out_buffer.ReadAbleSize());
            if(ret<0){
                if(_in_buffer.ReadAbleSize()>0){
                    _message_callback(shared_from_this(),&_in_buffer);
                }
                return ReleaseInLoop();
            }
            _out_buffer.MoveReadOffset(ret);
            //没有数据发送了，关闭写事件监控
            if(_out_buffer.ReadAbleSize()==0){
                _channel.DisableWrite();
                //如果当前是连接待关闭状态，则有数据，发送完数据释放连接，没有数据则直接释放
                if(_statu == DISCONNECTING){
                    return ReleaseInLoop();
                }
            }
            return;
        }
        //描述符触发挂断事件
        void HandleClose(){
            if(_in_buffer.ReadAbleSize()>0){
                _message_callback(shared_from_this(),&_in_buffer);
            }
            return ReleaseInLoop();
        }
        //描述符触发出错事件
        void HandleError(){
            HandleClose();
        }
        //描述符触发任意事件: 1.刷新连接的活跃度--延迟定时销毁任务 2. 调用组件使用者的任意事件回调
        void HandleEvent(){
            if(_enable_inactive_release == true){
                _loop->TimerRefresh(_conn_id);
            }
            if(_event_callback){
                _event_callback(shared_from_this());
            }
        }
        //获取连接之后，所处的状态下要进行各种设置（给channel设置事件回调，启动读监控）
        void EsTablishedInLoop(){
            //1. 修改连接状态  2. 启动读事件监控  3. 调用回调函数
            assert(_statu == CONNECTING);//当前连接状态必须一定是上层的半连接状态
            _statu=CONNECTED;//当前函数执行完毕，则连接进入已完成连接状态
            _channel.EnableRead();//一旦启动读事件监控就有可能立即触发读事件，如果这时候启动了非活跃连接销毁
            if(_connected_callback) _connected_callback(shared_from_this());
        }
        //这个接口才是实际的释放接口
        void ReleaseInLoop(){
            //修改连接状态，将置为DISCONNECTED
            _statu=DISCONNECTED;
            //移除连接事件监控
            _channel.Remove();
            //关闭描述符
            _socket.Close();
            //如果当前定时器队列中还有定时销毁任务，则取消任务
            if(_loop->HasTimer(_conn_id)) CancelInactiveRelease();
            //调用关闭回调函数，避免先移除服务器管理的连接信息导致Connection被释放，再去处理会出错，因此先调用用户关闭回调函数
            if(_cloesd_callback) _cloesd_callback(shared_from_this());
            //移除服务器内部管理的连接信息
            if(_server_closed_callbck) _server_closed_callbck(shared_from_this());
        }
        //这个接口并不实际的发送接口，而只是把数据放到了发送缓冲区，启动了可写事件监控
        void SendInLoop(Buffer buf){
            if(_statu == DISCONNECTED) return;
            _out_buffer.WriteBufferAndPush(buf);
            if(_channel.WriteAble()==false) _channel.EnableWrite();
        }
        //这个关闭操作并非实际的连接释放操作，需要判断还有没有数据待处理，待发送
        void ShutdownInLoop(){
            _statu=DISCONNECTING;
            if(_in_buffer.ReadAbleSize()>0){
                if(_message_callback) _message_callback(shared_from_this(),&_in_buffer);
            }
            //写入数据的时候出错关闭，要么就是没有待发送数据，直接关闭
            if(_out_buffer.ReadAbleSize()>0){
                if(_channel.WriteAble() == false){
                    _channel.EnableWrite();
                }
            }
            if(_out_buffer.ReadAbleSize() == 0){
                ReleaseInLoop();
            }
        }
        //启动非活跃连接超时释放规则
        void EnableInactiveReleaseInLoop(int sec){
            _enable_inactive_release = true;
            if(_loop->HasTimer(_conn_id)) 
                return _loop->TimerRefresh(_conn_id);
            _loop->TimerAdd(_conn_id,sec,std::bind(&Connection::ReleaseInLoop,this));
        }
        void CancelInactiveReleaseInLoop(){
            _enable_inactive_release = false;
            if(_loop->HasTimer(_conn_id))
                _loop->TimerCancel(_conn_id);
        }
        void UpgradeInLoop(const Any& context,
                        const ConnectedCallback& conn,
                        const MessageCallback& msg,
                        const ClosedCallback& closed,
                        const AnyEventCallback& event){
            _context=context;
            _connected_callback=conn;
            _message_callback=msg;
            _cloesd_callback=closed;
            _event_callback=event;
        }
    public:
        Connection(EventLoop* loop,uint64_t conn_id,int sockfd)
            :_conn_id(conn_id),_sockfd(sockfd),_enable_inactive_release(false),_loop(loop),_statu(CONNECTING)
             ,_socket(_sockfd),_channel(_loop,_sockfd)
        {
            _channel.SetCloseCallback(std::bind(&Connection::HandleClose,this));
            _channel.SetReadCallback(std::bind(&Connection::HandleRead,this));
            _channel.SetWriteCallback(std::bind(&Connection::HandleWrite,this));
            _channel.SetErrorCallback(std::bind(&Connection::HandleError,this));
            _channel.SetEventCallback(std::bind(&Connection::HandleEvent,this));
        }
        ~Connection(){
            DBG_LOG("release conection: %p",this);
        }
        //描述符Fd
        int Fd(){ return _sockfd; }
        //_conn_id
        int Id(){ return _conn_id; }
        //是否处于CONNECTED
        bool Connected(){ return (_statu==CONNECTED); }
        //设置上下文
        void SetContext(const Any& context){ _context=context; }
        //获取上下文
        Any* GetContext(){ return &_context; }
        void SetConnectedCallback(const ConnectedCallback& cb){ _connected_callback=cb; }
        void SetMessageCallback(const MessageCallback& cb){_message_callback=cb; }
        void SetClosedCallback(const ClosedCallback& cb){ _cloesd_callback=cb; }
        void SetAnyEventCallback(const AnyEventCallback& cb){ _event_callback=cb; }
        void SetSrcClosedCallback(const ClosedCallback& cb){ _server_closed_callbck=cb; }
        //建立连接就绪后，进行channel回调设置，启动读监控，调用_connected_callback
        void EsTablished(){
            _loop->RunInLoop(std::bind(&Connection::EsTablishedInLoop,this));
        }
        //发送数据，将数据放到发送缓冲区，启动写事件监控
        void Send(char* data,size_t len){
            //外界传入的data可能是个临时的空间，我们现在只是把发送操作压入了任务池，有可能并没有被立即执行
            //因此有可能执行的时候，data指向的空间有可能已经被释放了
            Buffer buf;
            buf.WriteAndPush(data,len);
            _loop->RunInLoop(std::bind(&Connection::SendInLoop,this,buf));
        }
        //提供给组件者使用者的关闭接口--并不实际关闭，需要判断有没有数据待处理
        void Shutdown(){
            _loop->RunInLoop(std::bind(&Connection::ShutdownInLoop,this));
        }
        //启动非活跃销毁--并定义多长时间无通信就是非活跃，添加定时任务
        void EnableInactiveRelease(int sec){
            _loop->RunInLoop(std::bind(&Connection::EnableInactiveReleaseInLoop,this,sec));
        }
        //取消非活跃销毁
        void CancelInactiveRelease(){
            _loop->RunInLoop(std::bind(&Connection::CancelInactiveReleaseInLoop,this));
        }
        //切换协议---重置上下文以及阶段性处理函数
        void Upgrade(const Any& context,const ConnectedCallback& conn,const MessageCallback& msg,const ClosedCallback& closed,
                const AnyEventCallback& event){
            //这个接口必须在EventLoop线程中立即执行
            //防备新的事件触发后，处理的时候，切换任务还没有被执行--会导致数据使用原协议处理
            _loop->AsserInLoop();
            _loop->RunInLoop(std::bind(&Connection::UpgradeInLoop,this,context,conn,msg,closed,event));
        }
};

using AcceptCallback=std::function<void(int)>;
class Acceptor
{
    private:
        Socket _socket; //用于创建监听套接字
        EventLoop* _loop; //用于监听套接字进行事件监控
        Channel _channel; //用于对监听套接字进行事件管理

        AcceptCallback _accept_callbak;
    private:
        //监听套接字的读事件回调处理函数--获取新连接，调用_accept_callback函数进行新连接处理
        void HandleRead(){
            int newfd = _socket.Accept();
            if(newfd<0)return;
            if(_accept_callbak) _accept_callbak(newfd);
        }
        int SocketCreate(int port)
        {
            bool ret=_socket.CreateServer(port);
            assert(ret==true);
            return _socket.Fd();
        }
    public:
        //不能将启动读事件监控，放到构造函数中，必须在设置回调函数后，再去启动
        //否则有可能造成启动监控后，立即有事件触发，处理的时候回调函数还没设置好：新连接得不到处理，且造成资源泄露
        Acceptor(EventLoop* loop,uint16_t port)
            :_socket(SocketCreate(port)),_loop(loop),_channel(_loop,_socket.Fd())
        {
            _channel.SetReadCallback(std::bind(&Acceptor::HandleRead,this));
        }
        void SetAcceptCallback(const AcceptCallback& cb){
            _accept_callbak=cb;
        }
        void Listen(){ _channel.EnableRead(); }
};

class TcpServer
{
    private:
        uint64_t _conn_id;//这是一个自动增长的连接ID，是唯一的ID，跟Connection关联
        int _port;
        int _timeout;//这是非活跃连接的统计时间---多长时间无通信就是非活跃连接
        bool _enable_inactive_release;//是否启动了非活跃连接超时销毁的判断标志
        Acceptor _acceptor;//只是监听套接字的管理对象
        EventLoop _base_loop;//这是主线程的EventLoop对象，负责监听事件的处理
        LoopThreadPool _pool;//这是从属EventLoop线程池
        std::unordered_map<uint64_t,PtrConnection> _conns;//保存管理所有连接对应的shared_ptrshared_ptr对象

        using ConnectedCallback=std::function<void(const PtrConnection&)>;
        using MessageCallback=std::function<void(const PtrConnection&,Buffer*)>;
        using ClosedCallback=std::function<void(const PtrConnection&)>;
        using AnyEventCallback=std::function<void(const PtrConnection&)>;

        ConnectedCallback _connected_callback;
        MessageCallback _message_callback;
        ClosedCallback _cloesd_callback;
        AnyEventCallback _event_callback;
    private:
        void RunAfterInLoop(const Functor& task,int delay){
            _conn_id++;
            _base_loop.TimerAdd(_conn_id,delay,task);
        }
        //为新连接构造一个Connection进行管理
        void NewConnection(int fd){
            _conn_id++;
            PtrConnection conn(new Connection(_pool.NextLoop(),_conn_id,fd));
            conn->SetMessageCallback(_message_callback);
            conn->SetClosedCallback(_cloesd_callback);
            conn->SetConnectedCallback(_connected_callback);
            conn->SetAnyEventCallback(_event_callback);
            conn->SetSrcClosedCallback(std::bind(&TcpServer::RemoveConnection,this,std::placeholders::_1));
            if(_enable_inactive_release)conn->EnableInactiveRelease(_timeout);
            conn->EsTablished();
            _conns.insert(std::make_pair(_conn_id,conn));
        }
        void RemoveConnectionInLoop(const PtrConnection& conn){
            int id=conn->Id();
            auto it=_conns.find(id);
            if(it!=_conns.end()){
                _conns.erase(it);
            }
        }
        //从管理Connection的_conns中移除
        void RemoveConnection(const PtrConnection& conn){
            _base_loop.RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop,this,conn));
        }
    public:
        TcpServer(int port)
            :_conn_id(0),_port(port),_enable_inactive_release(false),_acceptor(&_base_loop,_port),_pool(&_base_loop)
        {
            _acceptor.SetAcceptCallback(std::bind(&TcpServer::NewConnection,this,std::placeholders::_1));
            _acceptor.Listen();
        }
        void SetThreadCount(int count){
            _pool.SetThreadCount(count);
        }
        void SetConnectedCallback(const ConnectedCallback& cb){ _connected_callback=cb; }
        void SetMessageCallback(const MessageCallback& cb){_message_callback=cb; }
        void SetClosedCallback(const ClosedCallback& cb){ _cloesd_callback=cb; }
        void SetAnyEventCallback(const AnyEventCallback& cb){ _event_callback=cb; }
        void EnableInactiveRelease(int timeout){
            _enable_inactive_release=true;
            _timeout=timeout;
        }
        //用于添加一个定时任务
        void RunAfter(const Functor& task,int delay){
            _base_loop.RunInLoop(std::bind(&TcpServer::RunAfterInLoop,this,task,delay));
        }
        void Start(){
            _pool.Create();_base_loop.Start();
        }
};

void Channel::Remove(){ return _loop->RemoveEvent(this); }
void Channel::Update(){ return _loop->UpdateEvent(this); }

void TimerWheel::TimerAdd(uint64_t id,uint32_t timeout,const TaskFunc& cb){
    _loop->RunInLoop(std::bind(&TimerWheel::TimerAddInLoop,this,id,timeout,cb));
}
void TimerWheel::TimerRefresh(uint64_t id){
    _loop->RunInLoop(std::bind(&TimerWheel::TimerRefreshInLoop,this,id));
}
void TimerWheel::TimerCancel(uint64_t id){
    _loop->RunInLoop(std::bind(&TimerWheel::TimerCancelInLoop,this,id));
}

class NetWork
{
    public:
        NetWork(){
            DBG_LOG("signal init!");
            signal(SIGPIPE,SIG_IGN);
        }
};
static NetWork nt;