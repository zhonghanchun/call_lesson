#pragma once 
#include <iostream>
#include <queue>
#include <pthread.h>
#include "Task.hpp"
#include "Log.hpp"
class ThreadPool
{
private:
    ThreadPool(int _num=6)
        :num(_num)
         ,stop(false)
    {
        pthread_mutex_init(&lock,nullptr);
        pthread_cond_init(&cond,nullptr);
    }
    ThreadPool(const ThreadPool& s)=delete ;
    ThreadPool& operator=(const ThreadPool& s)=delete;
public:
    static ThreadPool* getinstance()
    {
        static pthread_mutex_t my_lock=PTHREAD_MUTEX_INITIALIZER; 
        if(ts==nullptr)
        {
            pthread_mutex_lock(&my_lock);
            if(ts==nullptr)
            {
                ts=new ThreadPool;
                ts->ThreadPool_Init();
            }
            pthread_mutex_unlock(&my_lock);
        }
        return ts;
    }
    bool is_stop()
    {
        return stop;
    }
    
    bool is_task_queue_empty()
    {
        return task_queue.size()==0 ? true:false;
    }

    bool ThreadPool_Init()
    {
         for(int i=0;i<num;i++)
         {
             pthread_t tid;
             if((pthread_create(&tid,nullptr,ThreadRoutine,this))!=0)
             {
                 LOG(FATAL,"create thread pool error!");
                return false;
             }
         }
         LOG(INFO,"create thread pool success!");
         return true;
    }

    void ThreadWait()
    {
        pthread_cond_wait(&cond,&lock);
    }

    void ThreadWakeup()
    {
        pthread_cond_signal(&cond);
    }
    void Lock()
    {
        pthread_mutex_lock(&lock);
    }
    void UnLock()
    {
        pthread_mutex_unlock(&lock);
    }
    static void* ThreadRoutine(void* args)
    {
        ThreadPool *tp=static_cast<ThreadPool*>(args);
        while(true)
        {
            Task t;
            tp->Lock();
            while(tp->is_task_queue_empty())
            {
                tp->ThreadWait();
            }
            tp->Pop_task(t);
            tp->UnLock();
            t.Process_on();
        }
        return nullptr;
    }

    void Push_task(const Task& task)
    {
        Lock();
        task_queue.push(task);
        UnLock();
        ThreadWakeup();
    }

    void Pop_task(Task& task)
    {
        task=task_queue.front();
        task_queue.pop();
    }

    ~ThreadPool()
    {
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
    }
private:
    int num;
    bool stop;
    std::queue<Task> task_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    static ThreadPool* ts;
};
ThreadPool* ThreadPool::ts=nullptr; 
