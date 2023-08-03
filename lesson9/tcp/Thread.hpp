#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <string>
#include <functional>
using namespace std;


class Thread
{
public:
    typedef function<void*(void*)> fun_t;
private:
    static void* start_up(void* args)
    {
        Thread* p=static_cast<Thread*>(args);
        
        return p->run();
    }
public:
    Thread()
    {
        char buffer[1024];
        snprintf(buffer,sizeof buffer,"thread->%d",number++);
        _name=buffer;
    }
    void Start(fun_t fun,void* args=nullptr)
    {
        _fun=fun;
        _args=args;
        pthread_create(&_tid,nullptr,start_up,this);
    }
    void Join()
    {
        pthread_join(_tid,nullptr);
    }
    std::string thread_name()
    {
        return _name;
    }
    void* run()
    {
        return _fun(_args);
    }
private:
    string _name;
    fun_t _fun;
    void* _args;
    pthread_t _tid;

    static int number;
};
int Thread::number=1;
