#include <iostream>
#include <queue>
#include <pthread.h>

template<class T>
class BlockQueue
{
public:
    BlockQueue(const int size=10)
        :_cap(size)
    {
        pthread_mutex_init(&_mutex,nullptr);
        pthread_cond_init(&_pcond,nullptr);
        pthread_cond_init(&_ccond,nullptr);
    }
    void push(const T& val)
    {
        pthread_mutex_lock(&_mutex);
        while(is_full())
        {
            pthread_cond_wait(&_pcond,&_mutex);
        }
        _q.push(val);
        pthread_cond_signal(&_ccond);
        pthread_mutex_unlock(&_mutex);
    }
    T& pop(T* out)
    {
        pthread_mutex_lock(&_mutex);
        while(is_empty())
        {
            pthread_cond_wait(&_ccond,&_mutex);
        }
        *out=_q.front();
        _q.pop();
        pthread_cond_signal(&_pcond);
        pthread_mutex_unlock(&_mutex);
    }
private:
    bool is_full()
    {
        return _q.size()==_cap;
    }
    bool is_empty()
    {
        return _q.empty();
    }
private:
    std::queue<T> _q;
    int _cap;
    pthread_mutex_t _mutex;
    pthread_cond_t _pcond;
    pthread_cond_t _ccond;
};
