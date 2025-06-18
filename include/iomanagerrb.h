#ifndef __XTEN_IOMANAGERRB_H__
#define __XTEN_IOMANAGERRB_H__
#include "scheduler.h"
#include "timer.h"
namespace Xten
{
    // 基于 epoll_wait+红黑树定时器 封装的io协程调度器
    class IOManagerRB : public Scheduler, public TimerManager
    {
    public:
        typedef std::shared_ptr<IOManagerRB> prt;
        IOManagerRB(int threadNum = 1, bool userCaller = true, const std::string &name="");
        ~IOManagerRB();
        enum Event
        {
            // 无事件
            NONE = 0x00,
            // 读事件
            READ = 0x01,
            // 写事件
            WRITE = 0x04
        };
        // 添加io事件
        int AddEvent(int fd, Event ev, std::function<void()> func = nullptr);
        // 删除io事件
        bool DelEvent(int fd, Event ev);
        // 取消io事件
        bool CancelEvent(int fd, Event ev);
        // 取消fd上所有io事件
        bool CancelAll(int fd);
        // 获取当前调度器指针
        static IOManagerRB *GetThis();
        //fd上下文结构
        struct FdContext
        {
            //事件上下文结构
            struct EventContext
            {
                // 事件所属的调度器
                Scheduler *scheduler = nullptr;
                // 事件触发后执行协程
                Fiber::ptr fiber;
                // 事件触发后执行回调
                std::function<void()> cb;
            };
            //获取事件对应上下文
            EventContext& getEvContext(Event ev);
            //重置事件上下文
            void resetEvContext(EventContext& evctx);
            //触发事件上下文
            void triggerEvent(Event ev);
            EventContext read;  // 读上下文
            EventContext write; // 写上下文
            Event events;       // 当前fd的事件
            int fd;             // fd句柄
            SpinLock mutex;     // 自旋锁
        };

    protected:
        // 通知线程有任务
        virtual void Tickle() override;
        // 返回是否可以终止
        virtual bool IsStopping() override;
        // 线程无任务执行idle空闲协程
        virtual void Idle() override;
        //有更早过期任务
        virtual void onTimerInsertedAtFront() override;
        //_fdContexts扩容
        void FdContextsResize(int size);
        bool IsStopping(uint64_t& timeout);
    private:
        int _epfd;                            // eventpoll结构对应的fd
        int _pipeTicklefd[2];                 // 用于通知操作的管道读写fd
        std::atomic<int> _pendingEventNum{0}; // 要处理的任务数量
        RWMutex _mutex;                       // 读写锁
        std::vector<FdContext *> _fdContexts; // io事件上下文
    };
    // 基于 epoll_wait+多层级时间轮定时器 封装的io协程调度器
    class IOManagerTW : public Scheduler, public TimerWheelManager
    {
    public:
    private:
    };
}
#endif