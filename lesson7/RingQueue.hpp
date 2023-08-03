#pragma once 
#include <iostream>
#include <vector>
#include <semaphore.h>
template<class T>
class ring_queue
{
    void P(sem_t& sem)
    {
        sem_wait(&sem);
    }
    void V(sem_t& sem)
    {
        sem_post(&sem);
    }
    

public:
    ring_queue(const int& cap=5)
        :_queue(cap)
         ,_cap(cap)
    {
        sem_init(&_space_sem,0,_cap);
        sem_init(&_data_sem,0,0);
    }
    void Push(const T& val)
    {
        P(_space_sem);
        _queue[productor_step++]=val;
        productor_step%=_cap;
        V(_data_sem);
    }
    void Pop(T* out)
    {
        P(_data_sem);
        *out=_queue[consumer_step++];
        consumer_step%=_cap;
        V(_space_sem);
    }
    ~ring_queue()
    {
        sem_destroy(&_space_sem);
        sem_destroy(&_data_sem);
    }
private:
    std::vector<int> _queue;
    int _cap;
    sem_t _space_sem;
    sem_t _data_sem;
    int productor_step=0;
    int consumer_step=0;
};
