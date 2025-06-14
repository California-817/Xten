#include"../include/scheduler.h"
namespace  Xten
{      //默认让创建线程参与协程调度
        Scheduler::Scheduler(int threadNum=-1,bool use_caller=true,const std::string& name){}
         Scheduler::~Scheduler(){}
        //启动
        void  Scheduler::Start(){}
        //停止
        void  Scheduler::Stop(){}
        //放任务
        void  Scheduler::Schedule(){}
        void  Scheduler::Schedule(){}
        //无锁放任务
        void  Scheduler::ScheduleNoblock(){}
        //获取name
        std::string  Scheduler::GetName() const{}
        //输出调度器状态信息
        std::ostream&  Scheduler::dump(std::ostream& os) const{}
        //切换执行线程
        void  Scheduler::SwitchTo(int threadId=-1){}

        //返回线程的当前协程调度器
         Scheduler*  Scheduler::GetThis(){}
        //返回协程调度器的root_fiber
         Fiber*  Scheduler::GetRootFiber(){}


        //通知线程有任务
         void  Scheduler::Tickle(){}
        //运行函数
        void  Scheduler::Run(){}
        //返回是否可以终止
         bool  Scheduler::IsStopping(){}
        //线程无任务执行idle空闲协程
         void  Scheduler::Idle(){}
        //设置线程当前调度器
        void  Scheduler::SetThis(){}
        //返回是否有空闲线程
        bool  Scheduler::HasIdleThread(){}
    
} 