#include "RingQueue.hpp"
#include <pthread.h>
#include <unistd.h>
#include <ctime>
using namespace std;
void* productor(void* args)
{
    ring_queue<int>* rq=static_cast<ring_queue<int>*>(args);
    while(true)
    {
       int data=rand()%10+1;
       rq->Push(data);
       cout<<"生产数据:"<<data<<endl;
    }

    return nullptr;
}
void* consumer(void* args)
{
    ring_queue<int>* rq=static_cast<ring_queue<int>*>(args);
    while(true)
    {
        int data=0;
        rq->Pop(&data);
        cout<<"消费数据:"<<data<<endl;
        sleep(1);
    }
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr)^getpid());
    ring_queue<int>* rq=new ring_queue<int>();
    pthread_t p,c;
    pthread_create(&p,nullptr,productor,rq);
    pthread_create(&c,nullptr,consumer,rq);


    pthread_join(p,nullptr);
    pthread_join(c,nullptr);
    delete rq;
    return 0;
}
