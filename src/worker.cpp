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
        auto conf = g_workers_conf->GetValue();
        return Init(conf);
    }
    // 内部调用的初始化函数(直接通过配置文件进行初始化)
    bool WorkerManager::Init(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &workers)
    {
        for (auto &elem : workers)
        {
            std::string name = elem.first;
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
                sche->dump(os)<<std::endl;
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
}