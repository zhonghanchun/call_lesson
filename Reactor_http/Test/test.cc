#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <functional>
#include <sys/timerfd.h>

int main()
{
    int timerfd=timerfd_create(CLOCK_MONOTONIC,0);
    if(timerfd<0)
    {
        perror("timerfd_create error");
    }
    struct itimerspec itim;
    itim.it_value.tv_sec=3;
    itim.it_value.tv_nsec=0;
    itim.it_interval.tv_sec=3;
    itim.it_interval.tv_nsec=0;
    timerfd_settime(timerfd,0,&itim,nullptr);
    
    while(true)
    {
        uint64_t time=0;
        ssize_t s=read(timerfd,&time,sizeof time);
        if(s<0)
        {
            perror("read error");
        }
        std::cout<<"超时了，距离上一次超时了:"<<time<<"次"<<std::endl;
    }
    close(timerfd);
    return 0;
}
