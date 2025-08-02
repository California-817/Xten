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
#include <random>
#include <vector>

#define OFF 0
#define ON 1
#ifndef OPTIMIZE
#define OPTIMIZE ON
#endif
namespace Xten
{
    /// @brief  基类协程调度器
    class Scheduler
    {
    public:
        typedef std::shared_ptr<Scheduler> ptr;
        // 任务分配策略
        enum DistributePolicy
        {
            ROUND_ROBIN,  // 轮询分配
            LEAST_LOADED, // 最少队列分配
            RANDOM        // 随机分配
        };
        // 默认让创建线程参与协程调度
        Scheduler(int threadNum = 1, bool use_caller = true, const std::string &name = "");
        virtual ~Scheduler();
        // 启动
        void Start();
        // 停止
        void Stop();
        // 放任务  Task表示任务类型  1.fiber::ptr  2.std::function
        // Task &&task 这里的 T&&是万能引用 ---只有在模板类型推导的情况下才是万能引用（否则是普通右值引用）
        // 引用折叠规则 传入左值--左值引用   传入右值--右值引用
        // 对于 OPTIMIZE=ON threadId为线程的下标[use_caller 0 thread_0 1 thread_2 1 ....] or [thread_0 0 thread_1 1 thread_2 2 ....]
        // 对于 OPTIMIZE=OFF threadId为线程的LWP进程id
        template <class Task>
        void Schedule(Task &&task, int threadId = -1)
        {
            bool tickle_me = false;
            FuncOrFiber fcb(std::forward<Task>(task), threadId); // 这里的forward完美转发是必须的 保持原始语义
#if OPTIMIZE == OFF
            {
                RWMutex::WriteLock lock(_mutex);
                if (_fun_fibers.empty())
                {
                    tickle_me = true;
                }
                XTEN_ASSERT((fcb.fiber != nullptr || fcb.func != nullptr));
                _fun_fibers.push_back(fcb);
            }
#elif OPTIMIZE == ON
            int bestQueue = selectBestQueue(threadId);
            tickle_me = true;
            XTEN_ASSERT((fcb.fiber != nullptr || fcb.func != nullptr));
            {
                // 向bestQueue队列中放入任务
                RWMutex::WriteLock lock(*_localMtx[bestQueue]);
                for (int i = 0; i < _queueSizes.size(); i++)
                {
                    tickle_me &= (_queueSizes[i].load() == 0);
                }
                _localQueues[bestQueue].push_back(fcb);
                _queueSizes[bestQueue]++;
            }
#endif
            if (tickle_me)
            {
                Tickle();
            }
        }
        // 对于 OPTIMIZE=ON threadId为线程的下标[use_caller 0 thread_0 1 thread_2 1 ....] or [thread_0 0 thread_1 1 thread_2 2 ....]
        // 对于 OPTIMIZE=OFF threadId为线程的LWP进程id
        template <class InputIterator>
        void Schedule(InputIterator begin, InputIterator end, int threadId = -1)
        {
            bool tickle_me = false;
#if OPTIMIZE == OFF
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
#elif OPTIMIZE == ON
            int bestQueue = selectBestQueue(threadId);
            tickle_me = true;
            {
                // 向bestQueue队列中放入任务
                RWMutex::WriteLock lock(*_localMtx[bestQueue]);
                for (int i = 0; i < _queueSizes.size(); i++)
                {
                    tickle_me &= (_queueSizes[i].load() == 0);
                }
                while (begin != end)
                {
                    FuncOrFiber fcb(std::move(*begin), threadId);
                    XTEN_ASSERT((fcb.fiber != nullptr || fcb.func != nullptr));
                    _localQueues[bestQueue].push_back(fcb);
                    _queueSizes[bestQueue]++;
                    begin++;
                }
            }
#endif
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
            FuncOrFiber(const std::function<void()> &fc, int id = -1) // 左值引用
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
            FuncOrFiber(const Xten::Fiber::ptr &fb, int id = -1) // 左值引用
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
#if OPTIMIZE == ON
        // 挑选合适的队列
        int selectBestQueue(int threadId)
        {
            if (threadId != -1 && threadId < _localQueues.size())
            {
                // 指定了执行线程
                return threadId;
            }
            // 未指定(根据不同的分配策略执行不同分配)
            switch (_policy)
            {
            case DistributePolicy::ROUND_ROBIN:
            {
                size_t index = _roundRobinCounter.fetch_add(1) % _localQueues.size();
                return static_cast<int>(index);
            }
            break;
            case DistributePolicy::RANDOM:
            {
                static thread_local std::random_device rd;
                static thread_local std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, _localQueues.size() - 1);
                return dis(gen);
            }
            break;
            case DistributePolicy::LEAST_LOADED:
            default:
            {
                int bestQueue = 0;
                size_t min = _queueSizes[0].load();
                for (size_t i = 1; i < _queueSizes.size(); i++)
                {
                    size_t cur = _queueSizes[i].load();
                    if (min > cur)
                    {
                        // 找到了更少任务队列
                        min = cur;
                        bestQueue = i;
                    }
                }
                return bestQueue;
            }
            break;
            }
            return 0;
        }
public:
        // 进行定期的自动负载均衡
        void autoLoadBalance()
        {
            // 1.找到任务最多和最少的队列
            int maxQueue = -1;
            int minQueue = -1;
            size_t maxTask = _queueSizes[0].load();
            size_t minTask = maxTask;
            for (int i = 1; i < _queueSizes.size(); i++)
            {
                size_t cur = _queueSizes[i].load();
                if (cur > maxTask)
                {
                    maxQueue = i;
                    maxTask = cur;
                    continue;
                }
                if (cur < minTask)
                {
                    minQueue = i;
                    minTask = cur;
                    continue;
                }
            }
            // 判断差值是否大于阈值
            if (maxQueue > -1 && minQueue > -1 && maxTask - minTask > 10)
            {
                // 进行负载均衡
                std::vector<FuncOrFiber> tasks;
                tasks.reserve(5);
                {
                    RWMutex::WriteLock wlock(*_localMtx[maxQueue]);
                    auto it = _localQueues[maxQueue].rbegin();
                    size_t moved = 0;
                    while (it != _localQueues[maxQueue].rend() && moved < 5)
                    {
                        // 检查任务是否指定了特定线程执行
                        if (it->threadId == -1)
                        { // 只移动未指定线程的任务
                            tasks.push_back(*it);
                            it = std::reverse_iterator(_localQueues[maxQueue].erase(std::next(it).base()));
                            _queueSizes[maxQueue]--;
                            moved++;
                        }
                        else
                        {
                            ++it;
                        }
                    }
                }
                // 移动到最闲队列
                if (!tasks.empty())
                {
                    RWMutex::WriteLock wlock(*_localMtx[minQueue]);
                    for (auto &task : tasks)
                    {
                        _localQueues[minQueue].push_back(task);
                        _queueSizes[minQueue]++;
                    }
                }
            }
        }

#endif
    private:
        std::string _name;                       // 调度器name
        std::vector<Xten::Thread::ptr> _threads; // 工作线程
#if OPTIMIZE == OFF
        // 性能优化点---多线程对这个任务队列的操作需要加全局锁（锁的粒度比较大:考虑使用  多个任务队列 + 任务窃取 ）
        std::list<FuncOrFiber> _fun_fibers; // 任务队列
        Xten::RWMutex _mutex;               // 任务队列互斥锁
#elif OPTIMIZE == ON
    public:
        // 使用多线程多任务队列+任务窃取进行优化
        std::unordered_map<int, int> _threadIdToIndex;            // 线程id与队列下标映射关系
        Xten::RWMutex _mapMtx;                                    // 保证这个映射map的线程安全
        std::vector<std::list<FuncOrFiber>> _localQueues;         // 线程本地队列
        std::vector<std::unique_ptr<Xten::RWMutex>> _localMtx;    // 线程本地锁
        DistributePolicy _policy = DistributePolicy::ROUND_ROBIN; // 分配策略（默认轮询分配策略）
        std::vector<std::atomic<size_t>> _queueSizes;             // 每个队列的负载情况
        std::atomic<size_t> _roundRobinCounter = {0};             // 轮询分配的计数器

        // 工作统计
        std::atomic<uint64_t> _localHits = {0};     // 本地队列命中总数
        std::atomic<uint64_t> _steals = {0};        // 窃取成功任务数量
        std::atomic<uint64_t> _stealAttempts = {0}; // 尝试窃取次数

#endif
        Xten::Fiber::ptr _root_fiber; // 创建线程的调度协程
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