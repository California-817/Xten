#include "../include/worker.h"
#include "../include/config.h"
namespace Xten
{
    static ConfigVar<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>::ptr g_workers_conf = Config::LookUp("workers",
                                                                                                                                         std::unordered_map<std::string, std::unordered_map<std::string, std::string>>(),
                                                                                                                                         "workers config");
    WorkerManager::WorkerManager()
        : _stop(true)
    {
    }
    // 添加调度器
    void WorkerManager::Add(Scheduler::ptr sche)
    {
        if (sche)
        {
            _workers[sche->GetName()].push_back(sche);
        }
    }
    // 获取调度器(通过类型string获取)
    Scheduler::ptr WorkerManager::Get(const std::string &name)
    {
        auto iter = _workers.find(name);
        if (iter != _workers.end())
        {
            // 找到了对应类型worker的数组
            if (iter->second.size() == 1)
            {
                // 只有一个调度器
                return iter->second[0];
            }
            static std::atomic_uint32_t s_count = 0;
            return iter->second[(s_count++) % iter->second.size()];
        }
        return nullptr;
    }
    // 获取返回IOManager的智能指针
    IOManager::prt WorkerManager::GetAsIOManager(const std::string &name)
    {
        return std::dynamic_pointer_cast<IOManager>(Get(name));
    }
    // 初始化
    bool WorkerManager::Init()
    {
        //在这里可以直接获取，因为这个函数不会频繁调用且配置启动后不需要修改
        auto conf = g_workers_conf->GetValue();
        return Init(conf);
    }
    // 内部调用的初始化函数(直接通过配置文件进行初始化)
    bool WorkerManager::Init(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &workers)
    {
        for (auto &elem : workers)
        {
            std::string name = elem.first; //调度器组的类型 accept io process
#define XX(map, key, default_val) \
    ((map).count(key) ? atoi((map).at(key).c_str()) : (default_val))
            int worker_num = XX(elem.second, "worker_num", 1);
            int thread_num = XX(elem.second, "thread_num", 1);
#undef XX
            // auto iter=elem.second.find("worker_num");
            // if(iter==elem.second.end())
            // {
            // worker_num=atoi(iter->second.c_str());
            // }
            for (int i = 0; i < worker_num; i++)
            {
                Scheduler::ptr sche;
                if (!i)
                {
                    // 第一个调度器
                    sche = std::make_shared<IOManager>(thread_num, false, name);
                }
                else
                {
                    // 后续调度器
                    sche = std::make_shared<IOManager>(thread_num, false, name + "-" + std::to_string(i));
                }
                Add(name, sche);
            }
        }
        _stop = _workers.empty();
        return true;
    }
    // 停止所有workers
    void WorkerManager::Stop()
    {
        if (_stop)
        {
            return;
        }
        for (auto &vecSche : _workers)
        {
            for (auto &sche : vecSche.second)
            {
                sche->Schedule([]() {});
                sche->Stop();
            }
        }
        _stop = true;
        _workers.clear();
    }
    // 获取调度器数量
    uint32_t WorkerManager::GetCount()
    {
        uint32_t count = 0;
        for (auto &elem : _workers)
        {
            count += elem.second.size();
        }
        return count;
    }
    // 输出workers的信息
    std::ostream &WorkerManager::dump(std::ostream &os)
    {
        for (auto &vecSche : _workers)
        {
            for (auto &sche : vecSche.second)
            {
                sche->dump(os) << std::endl;
            }
        }
        return os;
    }
    WorkerManager::~WorkerManager()
    {
        Stop();
    }
    // 根据name添加调度器
    void WorkerManager::Add(const std::string &name, Scheduler::ptr sche)
    {
        if (sche)
        {
            _workers[name].push_back(sche);
        }
    }

    WorkerGroup::WorkerGroup(uint32_t batchSize, Scheduler *scheduler)
        : _isFinish(false),
          _batchSize(batchSize),
          _scheduler(scheduler),
          _fiberSem(batchSize)
    {
    }
    WorkerGroup::~WorkerGroup()
    {
        // 等待所有任务完成（一定是所有任务都执行完毕后再调用这个析构函数，因为任务中传入了智能指针）
        WaitAll();
    }
    // 放入任务
    void WorkerGroup::Schedule(std::function<void()> cb, int thread)
    {
        if (cb)
        {
            auto self = shared_from_this();
            _fiberSem.wait(); // 获取到信号量才能放入任务（控制并发量）
            _scheduler->Schedule(std::bind(&WorkerGroup::doWork, this, self, cb), thread);
        }
    }
    // 批量放入任务
    void WorkerGroup::Schedule(const std::vector<std::function<void()>> &cbs, int thread)
    {
        if (!cbs.empty())
        {
            std::vector<std::function<void()>> workcbs;
            auto self = shared_from_this();
            for (auto &cb : cbs)
            {
                _fiberSem.wait();
                workcbs.push_back(std::bind(&WorkerGroup::doWork, this, self, cb));
            }
            _scheduler->Schedule(workcbs.begin(), workcbs.end(), thread);
        }
    }
    // 等待所有任务完成
    void WorkerGroup::WaitAll()
    {
        if (!_isFinish)
        {
            _isFinish = true;
            for (int i = 0; i < _batchSize; i++)
            {
                // 获取所有信号量-->达到等待所有任务退出的效果
                // 因为如果还有没有完成的任务，实际上会占用信号量，只有任务完成后post信号量，全post之后就能在这里获取到所有信号量
                _fiberSem.wait();
            }
        }
    }
    // 抽象出一个执行函数
    void WorkerGroup::doWork(WorkerGroup::ptr self, std::function<void()> cb)
    {
        cb();
        // 执行完毕后释放信号量（表示该任务完成，可以放入新任务）
        _fiberSem.post();
    }
    TimedWorkGroup::TimedWorkGroup(uint32_t batchSize, uint64_t timeoutMs, IOManager *iomanager)
        : _batchSize(batchSize),
          _timeoutMs(timeoutMs),
          _iomanager(iomanager),
          _isFinish(false),
          _isTimeouted(false),
          _fiberSem(batchSize),
          _timer(nullptr)
    {
    }
    TimedWorkGroup::~TimedWorkGroup()
    {
        WaitAll();
    }
    void TimedWorkGroup::Schedule(std::function<void()> cb, int thread)
    {
        if (!_isTimeouted && cb)
        {
            _fiberSem.wait();
        }
        // 无论是否超时，都将任务放入调度器执行
        auto self = shared_from_this();
        _iomanager->Schedule(std::bind(&TimedWorkGroup::doWork, this, self, cb), thread);
    }
    void TimedWorkGroup::Schedule(std::vector<std::function<void()>> cbs, int thread)
    {
        for (auto &cb : cbs)
        {
            Schedule(std::move(cb), thread);
        }
    }
    void TimedWorkGroup::WaitAll()
    {
        if (!_isFinish)
        {
            _isFinish = true;
            for (int i = 0; (i < _batchSize) && !_isTimeouted; i++)
            {
                //等待所有任务完成，但是如果超时了则不会继续等待
                _fiberSem.wait();
            }
            // 是未超时出来的
            if (!_isTimeouted && _timer)
            {
                // 取消超时定时器
                _timer->cancel();
                _timer = nullptr;
            }
        }
    }
    // 处理超时参数
    void TimedWorkGroup::OnTimer(TimedWorkGroup::ptr self)
    {
        _timer = nullptr;
        _isTimeouted = true;
        _fiberSem.notifyAll();
    }
    // 真正调度函数
    void TimedWorkGroup::doWork(TimedWorkGroup::ptr, std::function<void()> cb)
    {
        cb();
        _fiberSem.post();
    }
}