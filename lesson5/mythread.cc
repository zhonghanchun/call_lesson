#include <iostream>
#include <pthread.h>
#include <unistd.h>
using namespace std;
void* handler(void* args)
{
    const char* name=(const char*)args;
    while(true)
    {
        cout<<"我是新线程，name : "<<name<<endl;
        sleep(1);
    }
}
int main()
{
    pthread_t tid;
    pthread_create(&tid,nullptr,handler,(void*)"one thread");
    while(true)
    {
        cout<<"我是主线程"<<endl;
        sleep(1);
    }
    return 0;
}
