#ifndef __XTEN_THREAD_H__
#define __XTEN_THREAD_H__
#include "const.h"
#include "nocopyable.hpp"
#include"mutex.h"
#include<pthread.h>
#include<functional>
namespace Xten
{
    class Thread : public NoCopyable
    {
    public:
        typedef std::shared_ptr<Thread> ptr;
        Thread();
        void Init(std::function<void()> func, const std::string &name);
        Thread(std::function<void()> func,const std::string& name);
        pid_t getId(); //获取lwp的id
        std::string getName(); //获取线程name
        void join(); //等待线程退出
        ~Thread();
    public:  //静态函数---用来获取当前线程的一些值（线程局部存储）
        static Thread* GetThis(); //获取当前线程的this指针
        static std::string GetName(); //获取当前线程name
        static void SetName(const std::string& name);//设置当前线程name--主要用于主线程（非用户创建）
    private:
        static void* run(void* args);// 线程的实际运行函数
    private:
        Xten::Semaphore _sem; //信号量用于保证构造顺序
        pthread_t _thread; //线程创建后返回的tcb线程控制块在地址空间的地址
        pid_t _id; //线程的lwp的id
        std::function<void()> _func; //线程执行函数
        std::string _name; //线程name
    };
}
#endif