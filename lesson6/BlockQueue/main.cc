#include <time.h>
#include <unistd.h>
#include "BlockQueue.hpp"
#include "Task.hpp"

template<class C,class S>
class BlockQueues
{
public:
    BlockQueue<C>* c_bq;
    BlockQueue<S>* s_bq;
};

void* consume(void* args)
{
    BlockQueue<Task>* task_bq=(static_cast<BlockQueues<Task,SaveTask>*>(args))->c_bq;
    BlockQueue<SaveTask>* save_bq=(static_cast<BlockQueues<Task,SaveTask>*>(args))->s_bq;
    while(true)
    {
        Task ts;
        task_bq->pop(&ts);
        std::string result=ts();
        std::cout<<"完成任务->"<<result<<std::endl;
        SaveTask s(result,Save);
        save_bq->push(s);
        std::cout<<"推送存储任务完成。。。"<<std::endl;
        sleep(1);
    }
    return nullptr;
}
void* produce(void* args)
{
    BlockQueue<Task>* p=(static_cast<BlockQueues<Task,SaveTask>*>(args))->c_bq;
    while(true)
    {
       int x=rand()%10+1;
       int y=rand()%5+1;
       int pos=rand()%oper.size();
       Task t(mymath,x,y,oper[pos]);
       p->push(t);
       std::cout<<"生产任务->"<<t.Task_string()<<std::endl;
       //sleep(1);
    }
    return nullptr;
}
void* saver(void* args)
{
    BlockQueue<SaveTask>* p=(static_cast<BlockQueues<Task,SaveTask>*>(args))->s_bq;
    while(true)
    {
        SaveTask s;
        p->pop(&s);
        s();
        std::cout<<"保存成功。。。"<<std::endl;
    }
    return nullptr;
}

int main()
{
    srand((unsigned long)time(nullptr) ^ getpid());
    BlockQueues<Task,SaveTask> bqs;
    bqs.c_bq=new BlockQueue<Task>();
    bqs.s_bq=new BlockQueue<SaveTask>();

    pthread_t s,c,p;
    pthread_create(&c,nullptr,consume,&bqs);
    pthread_create(&p,nullptr,produce,&bqs);
    pthread_create(&s,nullptr,saver,&bqs);



    pthread_join(s,nullptr);
    pthread_join(c,nullptr);
    pthread_join(p,nullptr);

    delete bqs.c_bq;
    delete bqs.s_bq;




    //BlockQueue<Task>* bp=new BlockQueue<Task>();
    //pthread_t c,p;
    //pthread_create(&c,nullptr,consume,(void*)bp);
    //pthread_create(&p,nullptr,produce,(void*)bp);

    //pthread_join(c,nullptr);
    //pthread_join(p,nullptr);
    return 0;
}
