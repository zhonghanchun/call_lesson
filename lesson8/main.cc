#include <memory>
#include <ctime>
#include <unistd.h>
#include "ThreadPool.hpp"
#include "Task.hpp"
using namespace Thread_s;

int main()
{
    srand((unsigned int)time(nullptr)^getpid());
    std::unique_ptr<ThreadPool<Task_call::Tasks>> tp(new ThreadPool<Task_call::Tasks>());
    tp->start_run();
    while(1)
    {
        int x=rand()%10+1;
        int y=rand()%5+1;
        size_t pos=rand()%Task_call::oper.size();
        Task_call::Tasks t(Task_call::mymath,x,y,Task_call::oper[pos]);
        tp->Push(t);
        std::cout<<"main_id:"<<pthread_self()%10<<"->生产任务:";
        t.Task_string();
        sleep(1);
    }
    return 0;
}
