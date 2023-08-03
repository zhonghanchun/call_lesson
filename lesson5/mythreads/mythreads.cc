#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
using namespace std;
class pthreads
{
public:
//private:
    char name[64];
    int number;
    pthread_t tid;
};
class pthreads_exit
{
public:
    int exit_code;
    int result_code;
};
void* handler(void* args)
{
    sleep(1);
    pthreads* tid=static_cast<pthreads* >(args);
    int cnt=10;
    while(cnt)
    {
        cout<<"new thread create success,name: "<<tid->name<<" cnt:"<<cnt--<<endl;
        sleep(1);
    }
    delete tid;
    pthreads_exit* p=new pthreads_exit();
    p->exit_code=1;
    p->result_code=106;
    return (void*)p;
}
int main()
{
    vector<pthreads*> threads;
    for(int i=0;i<10;i++)
    {
        pthreads* tid=new pthreads();
        tid->number=106;
        snprintf(tid->name,sizeof(tid->name),"%s:%d","thread",i);
        pthread_create(&tid->tid,nullptr,handler,tid);
        threads.push_back(tid);
    }
    for(auto& td: threads)
    {
        cout<<"create thread: "<<td->name<<" : "<<td->tid<<"  success!"<<endl;
    }
    for(auto& td: threads)
    {
        pthreads_exit* ret=nullptr;
        pthread_join(td->tid,(void**)&ret);
        cout<<"pthread_join sucess  exit_code:"<< ret->exit_code<<", result_code: "<<ret->result_code<<endl;
        delete td;
       // delete ret;
    }
    cout<<"main quit!"<<endl;
  /*  while(true)
    {
        cout<<"main thread!"<<endl;
        sleep(1);
    }*/
    return 0;
}
