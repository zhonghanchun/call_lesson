#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <unistd.h>

using TaskFunc=std::function<void()>;
using ReleaseFunc=std::function<void()>;
class TimerTask
{
    private:
        uint64_t _id;//定时器任务对象ID
        uint32_t _timeout;//定时任务超时时间
        TaskFunc _task_cb;//定时器对象要执行的定时任务
        ReleaseFunc _release;//用于删除Timer Wheel中保存的定时器对象信息
        bool _cancel;
    public:
        TimerTask(uint64_t id,uint32_t timeout,const TaskFunc& cb)
            :_id(id),_timeout(timeout),_task_cb(cb),_cancel(false){}
       ~TimerTask(){
           if(_cancel==false)
                _task_cb();
           _release();
       } 
       void SetRelease(const ReleaseFunc& cb){_release=cb;}
       uint32_t DelayTime(){return _timeout;}
       void SetCancel(){_cancel=true;}
};

using PtrTask=std::shared_ptr<TimerTask>;
using WeakTask=std::weak_ptr<TimerTask>;
class TimerWheel
{
    private:
        int _tick;//走到哪里释放对应的执行任务
        int _capacity;
        std::vector<std::vector<PtrTask>> _wheel;
        std::unordered_map<uint64_t,WeakTask> _timers;
    private:
        void Remove(uint64_t id)
        {
            auto it=_timers.find(id);
            if(it!=_timers.end())
                _timers.erase(it);
        }
    public:
        TimerWheel():_tick(0),_capacity(60),_wheel(_capacity){}
        //添加定时任务
        void TimerAdd(uint64_t id,uint32_t timeout,const TaskFunc& cb)
        {
            PtrTask pt(new TimerTask(id,timeout,cb));
            pt->SetRelease(std::bind(&TimerWheel::Remove,this,id));
            int pos=(_tick+timeout)%_capacity;
            _wheel[pos].push_back(pt);
            _timers[id]=WeakTask(pt);
        }
        //刷新/延迟定时任务
        void TimerRefresh(uint64_t id)
        {
            auto it=_timers.find(id);
            if(it==_timers.end())
                return;
            PtrTask pt=it->second.lock();
            int delay=pt->DelayTime();
            int pos=(_tick+delay)%_capacity;
            _wheel[pos].push_back(pt);
        }
        void RunTimerTask()
        {
            _tick=(_tick+1)%_capacity;
            _wheel[_tick].clear();
        }
        void TimerCancel(uint64_t id)
        {
            auto it=_timers.find(id);
            if(it==_timers.end())
                return;
            PtrTask pt=it->second.lock();
            if(pt)pt->SetCancel();
        }
};



class Test
{
    public:
        Test(){std::cout<<"构造函数"<<std::endl;}
        ~Test(){std::cout<<"析构函数"<<std::endl;}
};


void Delete(Test* t)
{
    delete t;
}

int main()
{
    TimerWheel tw;
    Test* t=new Test();
    tw.TimerAdd(1,3,std::bind(Delete,t));
    for(int i=1;i<=3;i++)
    {
        sleep(1);
        tw.TimerRefresh(1);
        tw.RunTimerTask();
        std::cout<<"刷新了一下定时任务"<<std::endl;
    }
    tw.TimerCancel(1);
    while(1)
    {
        sleep(1);
        tw.RunTimerTask();
        std::cout<<"----------------------------"<<std::endl;
    }
    return 0;
}








