#ifndef __XTEN_WORKER_H__
#define __XTEN_WORKER_H__
#include "scheduler.h"
#include <unordered_map>
#include <memory>
#include "iomanager.h"
#include "mutex.h"
#include "nocopyable.hpp"
#include <vector>
#include "singleton.hpp"
namespace Xten
{
    // 单例WorkerManager通过智能指针管理整个程序的所有调度器（通过配置文件读取配置初始化程序的所有调度器）
    class WorkerManager : public singleton<WorkerManager>
    {
        WorkerManager();
        // 添加调度器
        void Add(Scheduler::ptr sche);
        // 获取调度器(通过类型string获取)
        Scheduler::ptr Get(const std::string &name);
        // 获取返回IOManager的智能指针
        IOManager::prt GetAsIOManager(const std::string &name);
        // 初始化
        bool Init();
        // 内部调用的初始化函数(直接通过配置文件进行初始化)
        bool Init(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &workers);
        // 停止所有workers
        void Stop();
        // 判断是否停止
        bool IsStopped() const { return _stop; }
        // 获取调度器数量
        uint32_t GetCount();
        // 输出workers的信息
        std::ostream &dump(std::ostream &os);
        ~WorkerManager();

    private:
        // 根据类型添加调度器
        void Add(const std::string &name, Scheduler::ptr sche);
        // key对应的是worker的类型，vector存储这一个类型的所有调度器
        std::unordered_map<std::string, std::vector<Scheduler::ptr>> _workers;
        bool _stop;
    };
    // `WorkerGroup` 这个类对 `Scheduler`（调度器）的封装，目的是---->【批量任务调度、并发控制和任务组同步】
    // 2. 并发控制
    //    通过 `batch_size` 控制每批次并发执行的任务数量，避免一次性调度过多任务导致资源压力。
    // 3. 任务组同步
    //    提供 `waitAll()` 方法，可以阻塞等待所有批量调度的任务执行完毕，实现任务组的同步（常用于并发批量处理后统一收集结果）。
    // 4. 简化业务代码
    //    业务层只需关心任务内容和数量，不用手动管理每个任务的调度、同步和资源释放。
    // 典型应用场景
    // 并发批量网络请求、批量文件处理、批量数据库操作等。
    // 需要“全部任务完成后再继续”的业务逻辑。
    class WorkerGroup : public NoCopyable, public std::enable_shared_from_this<WorkerGroup>
    {
    public:
        typedef std::shared_ptr<WorkerGroup> ptr;
        // 静态工厂方法创建WorkerGroup(不能依赖临时智能指针的析构函数析构WorkerGroup，此时引用计数非0)
        static WorkerGroup::ptr Create(uint32_t batchSize, Scheduler *scheduler = Xten::Scheduler::GetThis())
        {
            return std::make_shared<WorkerGroup>(batchSize, scheduler);
        }
        WorkerGroup(uint32_t batchSize, Scheduler *scheduler = Xten::Scheduler::GetThis());
        ~WorkerGroup();
        // 放入任务（有并发控制）
        void Schedule(std::function<void()> cb, int thread = -1);
        // 批量放入任务（有并发控制）
        void Schedule(const std::vector<std::function<void()>> &cbs, int thread = -1);
        // 等待所有任务完成
        void WaitAll();

    private:
        // 抽象出一个执行函数（真正调度函数）
        void doWork(WorkerGroup::ptr self, std::function<void()> cb);
        // 任务工作的调度器
        Scheduler *_scheduler;
        // 任务并发量
        uint32_t _batchSize;
        // 是否完成
        bool _isFinish;
        // 协程信号量，用来控制任务的并发
        FiberSemphore _fiberSem;
    };
    // 同步等待所有任务完成具有等待超时时间的group 【所有任务都会被调度执行，只不过不一定等待所有任务执行完（超时了）】
    class TimedWorkGroup : public NoCopyable, public std::enable_shared_from_this<TimedWorkGroup>
    {
    public:
        typedef std::shared_ptr<TimedWorkGroup> ptr;
        // 静态工厂方法创建
        static TimedWorkGroup::ptr Create(uint32_t batchSize, uint64_t timeoutMs, IOManager *iomanager = IOManager::GetThis())
        {
            auto workgp = std::make_shared<TimedWorkGroup>(batchSize, timeoutMs, iomanager);
            // 根据超时时间开启一个超时定时器 (在当前创建该group的调度器中)
            workgp->_timer = IOManager::GetThis()->addTimer(timeoutMs, std::bind(&TimedWorkGroup::OnTimer, workgp.get(),workgp), false);
            return workgp;
        }
        TimedWorkGroup(uint32_t batchSize, uint64_t timeoutMs, IOManager *iomanager = IOManager::GetThis());
        ~TimedWorkGroup();
        void Schedule(std::function<void()> cb, int thread = -1);
        void Schedule(std::vector<std::function<void()>> cbs, int thread = -1);
        void WaitAll();

    private:
        // 处理超时参数
        void OnTimer(TimedWorkGroup::ptr self);
        // 真正调度函数
        void doWork(TimedWorkGroup::ptr, std::function<void()> cb);
        uint32_t _batchSize;     // 并发量
        IOManager *_iomanager;   // 任务执行调度器
        FiberSemphore _fiberSem; // 协程信号量
        bool _isTimeouted;       // 是否超时
        Timer::ptr _timer;       // 定时器
        bool _isFinish;          // 是否终止
        uint64_t _timeoutMs;     // 超时时间
    };
}
#endif