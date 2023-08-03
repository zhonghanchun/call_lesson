#include <vector>
#include <queue>
#include <unistd.h>
#include "Thread.hpp"
namespace Thread_s
{
    template<class T>
    class ThreadPool
    {
        private:
            static void* handler_task(void* args)
            {
                sleep(1);
                ThreadPool<T>* tp=static_cast<ThreadPool<T>*>(args);
                while(true)
                {
                    tp->Lock();
                    while(tp->Empty())
                    {
                        tp->thread_wait();
                    }
                    T t=tp->Pop();
                    tp->Unlock();
                    std::cout<<"子线程id:"<<pthread_self()%10<<"->消费任务";
                    t();
                    //sleep(1);
                }
                return nullptr;
            }
    public:
        void Lock(){pthread_mutex_lock(&_mutex);}
        void Unlock(){pthread_mutex_unlock(&_mutex);}
        bool Empty(){return _task_queue.empty();}
        void thread_wait(){pthread_cond_wait(&_cond,&_mutex);}

        T Pop()
        {
            T t=_task_queue.front();
            //std::cout<<t.Task_string()<<std::endl; 
            _task_queue.pop();
            return t;
        }
        void Push(T& in)
        {
            _task_queue.push(in);
            pthread_cond_signal(&_cond);
        }
        ThreadPool(const int size=5)
            :_size(size)
        {
            pthread_mutex_init(&_mutex,nullptr);
            pthread_cond_init(&_cond,nullptr);
            for(int i=0;i<_size;i++)
            {
                _threads.push_back(new Thread);
            }
        }
        void start_run()
        {
            for(auto& iter:_threads)
            {
                iter->Start(handler_task,this);
                std::cout<<iter->thread_name()<<":"<<pthread_self()<<std::endl;
            }
        }
        
        ~ThreadPool()
        {
            for(auto& iter:_threads)
            {
                delete iter;
            }
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_cond);
        }
    private:
        int _size;
        std::vector<Thread*> _threads;
        std::queue<T> _task_queue; 
        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
    };
}
