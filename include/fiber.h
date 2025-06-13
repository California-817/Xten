#ifndef __XTEN_FIBER_H__
#define __XTEN_FIBER_H__
// 封装有栈协程类
#include <memory>
#include <functional>
#define FIBER_UCONTEXT 0 // ucontext
#define FIBER_FCONTEXT 1 // fcontext

#ifndef FIBER_TYPE
#define FIBER_TYPE FIBER_FCONTEXT // 默认使用boost库1.82.0的fcontext
#endif
#if FIBER_TYPE == FIBER_UCONTEXT
#include <ucontext.h>
#elif FIBER_TYPE == FIBER_FCONTEXT
#include <boost/context/detail/fcontext.hpp>
#endif
namespace Xten
{
    class Fiber;
    Fiber* NewFiber();
    Fiber* NewFiber(size_t stack_size, std::function<void()> func, bool use_caller);
    void FreeFiber(Fiber* ptr);
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        typedef std::shared_ptr<Fiber> ptr;
        enum Status
        {
            INIT,  // 初始化
            EXEC,  // 执行
            HOLD,  // 挂起
            READY, // 准备执行
            TERM,  // 终止
            EXCEPT // 错误终止
        };
        // 线程主协程默认构造
        Fiber();
        // 子协程构造函数
        Fiber(size_t stack_size, std::function<void()> func, bool use_caller);
        ~Fiber();
        // 切入协程
        void SwapIn();
        void Call();
        // 切出协程
        void SwapOut();
        void Back();
        // 切出状态为hold
        void YieldToHold();
        // 切出状态为Ready
        void YieldToReady();
        // 重置协程
        void Reset(std::function<void()> func);
        // 获取协程状态
        Fiber::Status GetStatus() const;
        // 获取协程id
        static size_t GetFiberId() ;
        // 协程的真正入口函数--非用户传入
#if FIBER_TYPE==FIBER_UCONTEXT
        static void MainFunc();
        static void CallerMainFunc();
#elif FIBER_TYPE==FIBER_FCONTEXT
        static void MainFunc(boost::context::detail::transfer_t t);
        static void CallerMainFunc(boost::context::detail::transfer_t t);
#endif
        // 设置线程当前协程
        static void SetThis(Fiber *ts);
        // 获取线程当前协程
        static std::shared_ptr<Fiber> GetThis();
        // 获取总协程数
        static int64_t GetTotalFiberNums();

    private:
        bool _user_caller;           // 是否参与协程调度
        size_t _fiber_id;            // 协程id
        Status _status = INIT;       // 协程状态
        size_t _stack_size;          // 独立栈大小
        std::function<void()> _func; // 入口函数
#if FIBER_TYPE == FIBER_UCONTEXT     // 协程上下文结构---暂时支持两种类型
        ucontext_t _ctx;
#elif FIBER_TYPE == FIBER_FCONTEXT
        boost::context::detail::fcontext_t _ctx;
#endif
        char _stack[]; // 柔性数组 指向协程栈空间
    };
}
#endif
