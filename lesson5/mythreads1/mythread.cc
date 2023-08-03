#include <memory>
#include <unistd.h>
#include "Thread.hpp"
#include "Mutex.hpp"
int tickets=100;
pthread_mutex_t mt=PTHREAD_MUTEX_INITIALIZER;
void* start_up(void* args)
{
    string name=static_cast<const char*>(args);
    while(true)
    {
        {
        MutexGroud mu(&mt);
        if(tickets>0)
        {
            cout<<name<<" tickets: "<<tickets--<<endl;
            usleep(10000);
        }
        else 
        {
            break;
        }
        }
        usleep(1000);
    }
    return nullptr;
}

int main()
{
    unique_ptr<Thread> thread1(new Thread(start_up,(void*)"user1",1));
    unique_ptr<Thread> thread2(new Thread(start_up,(void*)"user2",2));
    unique_ptr<Thread> thread3(new Thread(start_up,(void*)"user3",3));
    unique_ptr<Thread> thread4(new Thread(start_up,(void*)"user4",4));

    thread1->join();
    thread2->join();
    thread3->join();
    thread4->join();
    return 0;
}
