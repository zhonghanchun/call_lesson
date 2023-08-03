#include <iostream>
#include <functional>
#include <pthread.h>
#include <assert.h>
using namespace std;

class Thread;

class Context
{
public:
    Thread* _th;
    void* _args;
    Context():_th(nullptr),_args(nullptr){}
};

class Thread
{
public:
    typedef function<void*(void*)> fun;
public:
    Thread(fun f,void* args=nullptr,int number=0):_fun(f),_args(args)
    {
        char buffer[1024];
        snprintf(buffer,sizeof buffer,"thread-%d",number);
        _name=buffer;

        Context* con=new Context();
        con->_th=this;
        con->_args=args;
        int n=pthread_create(&_tid,nullptr,start_up,con);
        assert(n==0);
        (void)n;
    }
    static void* start_up(void* args)
    {
        Context* con=static_cast<Context*>(args);
        void* ret=con->_th->run(con->_args);
        delete con;
        return ret;
    }
    void* run(void* args)
    {
        return _fun(args);
    }
    string getname()
    {
        return _name;
    }
    void join()
    {
        int n=pthread_join(_tid,nullptr);
        assert(n==0);
        (void)n;
    }
    ~Thread(){}
private:
    string _name;
    fun _fun;
    void* _args;
    pthread_t _tid;
};
