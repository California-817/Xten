#ifndef __XTEN_SCHEDULER_H__
#define __XTEN_SCHEDULER_H__
#include <string>
#include <memory>
#include "thread.h"
#include "mutex.h"
#include"fiber.h"
#include <list>
#include<atomic>
#include <vector>
namespace Xten
{
    /// @brief  基类协程调度器
    class Scheduler
    {
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        //默认让创建线程参与协程调度
        Scheduler(int threadNum=-1,bool use_caller=true,const std::string& name);
        ~Scheduler();
        //启动
        void Start();
        //停止
        void Stop();
        //放任务
        void Schedule();
        void Schedule();
        //无锁放任务
        void ScheduleNoblock();
        //获取name
        std::string GetName() const;
        //输出调度器状态信息
        std::ostream& dump(std::ostream& os) const;
        //切换执行线程
        void SwitchTo(int threadId=-1);

        //返回线程的当前协程调度器
        static Scheduler* GetThis();
        //返回协程调度器的root_fiber
        static Fiber* GetRootFiber();
    protected:
        //通知线程有任务
        virtual void Tickle(); 
        //运行函数
        void Run();
        //返回是否可以终止
        virtual bool IsStopping();
        //线程无任务执行idle空闲协程
        virtual void Idle();
        //设置线程当前调度器
        void SetThis();
        //返回是否有空闲线程
        bool HasIdleThread();
    private:
        /// @brief 任务队列的事件实体 支持两种方式放入事件  1.回调函数  2.协程
        struct FuncOrFiber
        {
            FuncOrFiber():threadId(-1){}
            FuncOrFiber(std::function<void()>& fc,int id=-1)  //左值引用
            {
                fiber.reset();
                func=fc;
                threadId=id;
            }
            FuncOrFiber(std::function<void()>&& fc,int id=-1) //右值引用
            {
                fiber.reset();
                func.swap(fc); //外部为nullptr
                threadId=id;
            }
            FuncOrFiber(Xten::Fiber::ptr& fb,int id=-1)  //左值引用
            {
                fiber=fb;
                func=nullptr;
                threadId=id;
            }
            FuncOrFiber(Xten::Fiber::ptr&& fb,int id=-1) //右值引用
            {
                fiber.swap(fb); //外部为nullptr
                func=nullptr; 
                threadId=id;
            }
            void Reset()
            {
                func=nullptr;
                fiber.reset();
                threadId=-1;
            }
        public:
            std::function<void()> func; //回调函数
            Xten::Fiber::ptr fiber; //协程
            int threadId=-1; //任务指定的线程id
        };

    private:
        Xten::SpinLock _mutex;                   // 任务队列互斥锁
        std::string _name;                       // 调度器name
        std::vector<Xten::Thread::ptr> _threads; // 工作线程
        std::list<FuncOrFiber> _fun_fibers;      // 任务队列
        Xten::Fiber::ptr _root_fiber;       //创建线程的调度协程
    protected:
        std::vector<int> _thread_ids; //所有线程id
        int _threads_count ; //总线程数
        std::atomic<int> _active_threadNum={0}; //工作线程数
        std::atomic<int> _idle_threadNum={0};  //空闲线程数
        std::atomic<bool> _stopping=true; //是否终止
        std::atomic<bool> _auto_stopping=false; //是否自动终止
        int _root_threadId=-1; //创建线程参与调度的线程id
    };
}
#endif