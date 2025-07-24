#include "thread.h"
#include "log.h"
namespace Xten
{
    static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");
    // 用静态局部变量(仅当前cpp文件可见)作为线程的局部存储变量
    static thread_local Thread *t_thread = nullptr; // 使用 thread_local 关键字确保每个线程都有自己独立的副本，互不干扰
    static thread_local std::string t_name = "UNKNOW";
    Thread::Thread()
        : _name(""), _sem(0), _func(nullptr), _thread(0), _id(0)
    {
    }
    void Thread::Init(std::function<void()> func, const std::string &name)
    {
        if (name.empty())
            _name = "UNKNOW";
        // 创建线程
        _func=func;
        int ret = pthread_create(&_thread, nullptr, &Thread::run, this);
        if (ret)
        { // 创建失败
            XTEN_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << ret
                                     << " name=" << name;
            throw std::logic_error("pthread creat error");
        }
        // 创建成功 确保子线程走到用户传入的函数时再返回
        _sem.wait();
    }
    Thread::Thread(std::function<void()> func, const std::string &name)
        : _func(func), _name(name), _sem(0)
    {
        if (name.empty())
            _name = "UNKNOW";
        // 创建线程
        int ret = pthread_create(&_thread, nullptr, &Thread::run, this);
        if (ret)
        { // 创建失败
            XTEN_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << ret
                                     << " name=" << name;
            throw std::logic_error("pthread creat error");
        }
        // 创建成功 确保子线程走到用户传入的函数时再返回
        _sem.wait();
    }
    pid_t Thread::getId() // 获取lwp的id
    {
        return _id;
    }
    std::string Thread::getName() // 获取线程name
    {
        return _name;
    }
    void Thread::join() // 等待线程退出
    {
        if (_thread)
        {
            // 正在运行则阻塞  执行完则直接返回
            int rt = pthread_join(_thread, nullptr);
            if (rt)
            {
                XTEN_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                                         << " name=" << _name;
                throw std::logic_error("pthread_join error");
            }
            _thread = 0; // 用户join说明用户已经对执行玩的线程进行回收了 Thread析构就不会detach了
        }
    }
    Thread::~Thread()
    {
        if (_thread) // 调用detach一定是用户没有主动join 一旦用户join就不能detach
        {            // 线程执行完后，如果没有被join，仍然可以调用 detach
            // 线程为执行完 调用detach让os管理
            pthread_detach(_thread); // 线程分离 让os管理 而不是强制销毁线程 我的操作相当于std::thread析构之前添加detach
            // std::thread的析构函数中 如果线程仍在执行中 析构会出错 1.detach分离  2.join等待线程执行完
        }
    }
    // 静态函数---用来获取当前线程的一些值（线程局部存储）
    Thread *Thread::GetThis() // 获取当前线程的this指针
    {
        return t_thread;
    }
    std::string Thread::GetName() // 获取当前线程name
    {
        return t_name;
    }
    void Thread::SetName(const std::string &name) // 设置当前线程name--主要用于主线程（非用户创建）
    {
        if (name.empty())
            return;
        // 先获取当前线程 --通过线程局部存储的指针
        Thread *ts = t_thread;
        if (ts) // ts一般不为空 除非是主线程
        {
            ts->_name = name;
        }
        t_name = name;
    }
    void *Thread::run(void *args) // 线程的实际运行函数
    {
        Thread *ts = static_cast<Thread *>(args);
        t_thread = ts; // 该子线程的线程局部存储变量设置成这个子线程结构体的指针
        t_name = ts->_name;
        ts->_id = Xten::ThreadUtil::GetThreadId();                        // 获取tid
        pthread_setname_np(ts->_thread, ts->_name.substr(0, 15).c_str()); // 在系统层面为thread命名
        // 通过 swap，将任务函数从对象中“提取”出来，确保即使对象被析构或修改，也不会影响到当前要执行的任务。
        // 将线程执行的函数与线程对象分离
        std::function<void()> cb;
        cb.swap(ts->_func);
        // 通过信号量让主线程的构造函数唤醒向下执行
        ts->_sem.post(); // post后说明该线程一些工作处理完毕--让处于构造函数的主线程返回
        cb();
        return nullptr;
    }
}