#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <string>
#include <functional>
using namespace std;
class Thread;
class Context
{
public:
    Thread* _ths;
    void* _args;
    Context():_ths(nullptr),_args(nullptr){}
};


class Thread
{
public:
    typedef function<void*(void*)> fun_t;
public:
    Thread(fun_t f,void* args=nullptr,int number=0)
        :_fun(f)
         ,_args(args)
    {
        char buffer[64];
        snprintf(buffer,sizeof buffer,"thread->%d",number);
        _name=buffer;
        Context* con=new Context;
        con->_args=args;
        con->_ths=this;
        pthread_create(&_tid,nullptr,start_up,con);
    }
    static void* start_up(void* args)
    {
        Context* con=static_cast<Context*>(args);
        void* ret=con->_ths->run(con->_args);
        delete con;
        return ret;
    }
    void* run(void* args)
    {
        return _fun(args);
    }
private:
    string _name;
    fun_t _fun;
    void* _args;
    pthread_t _tid;
};
