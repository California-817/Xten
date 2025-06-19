#ifndef __XTEN_SCHEDULER_H__
#define __XTEN_SCHEDULER_H__
#include <string>
#include <memory>
#include "thread.h"
#include "mutex.h"
#include "fiber.h"
#include <list>
#include <atomic>
#include "macro.h"
#include <vector>
namespace Xten
{
    /// @brief  基类协程调度器
    class Scheduler
    {
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        // 默认让创建线程参与协程调度
        Scheduler(int threadNum = 1, bool use_caller = true, const std::string &name = "");
        virtual ~Scheduler();
        // 启动
        void Start();
        // 停止
        void Stop();
        // 放任务  Task表示任务类型  1.fiber::ptr  2.std::function
        template <class Task>
        // Task &&task 这里的 T&&是万能引用 ---只有在模板类型推导的情况下才是万能引用（否则是普通右值引用）
        // 引用折叠规则 传入左值--左值引用   传入右值--右值引用
        void Schedule(Task &&task, int threadId = -1)
        {
            bool tickle_me = false;
            FuncOrFiber fcb(std::forward<Task>(task), threadId); // 这里的forward完美转发是必须的 保持原始语义
            {
                RWMutex::WriteLock lock(_mutex);
                if (_fun_fibers.empty())
                {
                    tickle_me = true;
                }
                XTEN_ASSERT((fcb.fiber != nullptr || fcb.func != nullptr));
                _fun_fibers.push_back(fcb);
            }
            if (tickle_me)
            {
                Tickle();
            }
        }
        template <class InputIterator>
        void Schedule(InputIterator begin, InputIterator end, int threadId = -1)
        {
            bool tickle_me = false;
            {
                RWMutex::WriteLock lock(_mutex);
                if (_fun_fibers.empty())
                {
                    tickle_me = true;
                }
                while (begin != end)
                {
                    FuncOrFiber fcb(std::move(*begin), threadId);
                    XTEN_ASSERT((fcb.fiber != nullptr || fcb.func != nullptr));
                    _fun_fibers.push_back(fcb);
                    begin++;
                }
            }
            if (tickle_me)
            {
                Tickle();
            }
        }
        // 获取name
        std::string GetName() const;
        // 输出调度器状态信息
        std::ostream &dump(std::ostream &os) const;
        // 切换执行线程
        void SwitchTo(int threadId = -1);

        // 返回线程的当前协程调度器
        static Scheduler *GetThis();
        // 返回当前线程的调度协程
        static Fiber *GetScheduleFiber();

    protected:
        // 通知线程有任务
        virtual void Tickle();
        // 线程运行函数
        void Run();
        // 返回是否可以终止
        virtual bool IsStopping();
        // 线程无任务执行idle空闲协程
        virtual void Idle();
        // 设置线程当前调度器
        void SetThis();
        // 返回是否有空闲线程
        bool HasIdleThread();

    private:
        /// @brief 任务队列的事件实体 支持两种方式放入事件  1.回调函数  2.协程
        struct FuncOrFiber
        {
            FuncOrFiber() : threadId(-1) {}
            FuncOrFiber(const FuncOrFiber &target) // 拷贝构造
            {
                if (this != &target)
                {
                    func = target.func;
                    fiber = target.fiber;
                    threadId = target.threadId;
                }
            }
            FuncOrFiber(FuncOrFiber &&target) // 移动构造
            {
                if (this != &target)
                {
                    func.swap(target.func);
                    fiber.swap(target.fiber);
                    threadId = target.threadId;
                }
            }
            FuncOrFiber &operator=(const FuncOrFiber &target)
            {
                if (this != &target)
                {
                    func = target.func;
                    fiber = target.fiber;
                    threadId = target.threadId;
                }
                return *this;
            }
            // 移动赋值运算符
            FuncOrFiber &operator=(FuncOrFiber &&target)
            {
                if (this != &target)
                {
                    func.swap(target.func);
                    fiber.swap(target.fiber);
                    threadId = target.threadId;
                }
                return *this;
            }
            FuncOrFiber(std::function<void()> &fc, int id = -1) // 左值引用
            {
                fiber.reset();
                func = fc;
                threadId = id;
            }
            FuncOrFiber(std::function<void()> &&fc, int id = -1) // 右值引用
            {
                fiber.reset();
                func.swap(fc); // 外部为nullptr
                threadId = id;
            }
            FuncOrFiber(Xten::Fiber::ptr &fb, int id = -1) // 左值引用
            {
                fiber = fb;
                func = nullptr;
                threadId = id;
            }
            FuncOrFiber(Xten::Fiber::ptr &&fb, int id = -1) // 右值引用
            {
                fiber.swap(fb); // 外部为nullptr
                func = nullptr;
                threadId = id;
            }
            void Reset()
            {
                func = nullptr;
                fiber.reset();
                threadId = -1;
            }

        public:
            std::function<void()> func; // 回调函数
            Xten::Fiber::ptr fiber;     // 协程
            int threadId = -1;          // 任务指定的线程id
        };

    private:
        Xten::RWMutex _mutex;                    // 任务队列互斥锁
        std::string _name;                       // 调度器name
        std::vector<Xten::Thread::ptr> _threads; // 工作线程
        // 性能优化点---多线程对这个任务队列的操作需要加全局锁（锁的粒度比较大:考虑使用  多个任务队列 + 任务窃取 ）
        std::list<FuncOrFiber> _fun_fibers; // 任务队列
        Xten::Fiber::ptr _root_fiber;       // 创建线程的调度协程
    protected:
        std::vector<int> _thread_ids;             // 所有线程id
        int _threads_num;                         // 总线程数
        std::atomic<int> _active_threadNum = {0}; // 工作线程数
        std::atomic<int> _idle_threadNum = {0};   // 空闲线程数
        std::atomic<bool> _stopping = true;       // 是否终止
        std::atomic<bool> _auto_stopping = false; // 是否自动终止
        int _root_threadId = -1;                  // 创建线程参与调度的线程id
    };

    /// @brief  协程任务切换器 --切换协程任务运行的调度器
    class SwitchScheduler
    {
    public:
        // 构造函数自动切换
        SwitchScheduler(Scheduler *target);
        // 析构函数自动切回
        ~SwitchScheduler();

    private:
        Scheduler *_caller; // 原始协程调度器
    };
}

// std::cout << scheduler << std::endl; ---方便调用dump输出调度器状态信息
std::ostream &operator<<(std::ostream &os, const Xten::Scheduler &scheduler);

#endif