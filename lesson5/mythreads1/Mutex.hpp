#include <iostream>
#include <pthread.h>

using namespace std;
class Mutex
{
public:
    Mutex(pthread_mutex_t* mutex):_mutex(mutex)
    {
        //pthread_mutex_lock(_mutex);
    }
    void lock()
    {
        pthread_mutex_lock(_mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(_mutex);
    }
    ~Mutex(){}
private:
    pthread_mutex_t* _mutex;
};


class MutexGroud
{
public:
    MutexGroud(pthread_mutex_t* mutex):_mt(mutex)
    {
        _mt.lock();
    }
    ~MutexGroud()
    {
        _mt.unlock();
    }
private:
    Mutex _mt;
};
