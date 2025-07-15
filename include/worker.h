#ifndef __XTEN_WORKER_H__
#define __XTEN_WORKER_H__
#include"scheduler.h"
#include<unordered_map>
#include<memory>
#include"iomanager.h"
#include<vector>
#include"singleton.hpp"
namespace Xten
{
    //单例WorkerManager通过智能指针管理整个程序的所有调度器（通过配置文件读取配置初始化程序的所有调度器）
    class WorkerManager: public singleton<WorkerManager>
    {
        WorkerManager();
        //添加调度器
        void Add(Scheduler::ptr sche);
        //获取调度器(通过类型string获取)
        Scheduler::ptr Get(const std::string& name);
        //获取返回IOManager的智能指针
        IOManager::prt GetAsIOManager(const std::string& name);
        //初始化
        bool Init();
        //内部调用的初始化函数(直接通过配置文件进行初始化)
        bool Init(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& workers);
        //停止所有workers
        void Stop();
        //判断是否停止
        bool IsStopped() const {return _stop;}
        //获取调度器数量
        uint32_t GetCount();
        //输出workers的信息
        std::ostream& dump(std::ostream& os);
        ~WorkerManager();
    private:
        //根据类型添加调度器
        void Add(const std::string& name,Scheduler::ptr sche);
        //key对应的是worker的类型，vector存储这一个类型的所有调度器
        std::unordered_map<std::string,std::vector<Scheduler::ptr>> _workers;
        bool _stop;
    };
}
#endif