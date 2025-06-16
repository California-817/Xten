#ifndef __XTEN_IOMANAGER_H__
#define __XTEN_IOMANAGER_H__
#include "scheduler.h"
#include "timer.h"
namespace Xten
{
    //基于 epoll_wait+红黑树定时器 封装的io协程调度器
    class IOManagerRB : public Scheduler, public TimerManager
    {
        public:
            IOManagerRB();
            ~IOManagerRB();
        private:
            std::string _name; //调度器name
            
    };
    //基于 epoll_wait+多层级时间轮定时器 封装的io协程调度器
    class IOManagerTW : public Scheduler,public TimerWheelManager
    {
        public:
        private:
    };
}
#endif