#ifndef __XTEN_FIBER_H__
#define __XTEN_FIBER_H__
#include <memory>
#include <functional>
#define FIBER_UCONTEXT 0 // ucontext
#define FIBER_FCONTEXT 1 // fcontext
#define FIBER_COCTX 2    // coctx

// #ifndef FIBER_TYPE
// #define FIBER_TYPE FIBER_FCONTEXT // 默认使用boost库的fcontext
// #endif
#if FIBER_TYPE == FIBER_UCONTEXT
#include <ucontext.h>
#elif FIBER_TYPE == FIBER_FCONTEXT
#include "fcontext/fcontext.h"
#elif FIBER_TYPE == FIBER_COCTX
#include "libco/coctx.h"
#endif
namespace Xten
{
        class Fiber;
        Fiber *NewFiber();
        Fiber *NewFiber(size_t stack_size, std::function<void()> func, bool use_caller);
        void FreeFiber(Fiber *ptr);
        class Scheduler;
        // 封装有栈协程类
        class Fiber : public std::enable_shared_from_this<Fiber>
        {
        public:
                friend class Scheduler;
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
                // 切入协程---由调度协程切入
                void SwapIn();
                // 切入协程---由线程默认主协程切入
                void Call();
                // 切出协程--切到调度协程
                void SwapOut();
                // 切出协程--切到线程默认主协程
                void Back();
                // 切出状态为hold
                static void YieldToHold();
                // 切出状态为Ready
                static void YieldToReady();
                // 重置协程
                void Reset(std::function<void()> func);
                // 获取协程状态
                Fiber::Status GetStatus() const;
                // 获取协程id
                static size_t GetFiberId();
                // 协程的真正入口函数--非用户传入
#if FIBER_TYPE == FIBER_UCONTEXT
                static void MainFunc();
                static void CallerMainFunc();
#elif FIBER_TYPE == FIBER_FCONTEXT
                static void MainFunc(intptr_t t);
                static void CallerMainFunc(intptr_t t);
#elif FIBER_TYPE == FIBER_COCTX
                static void* MainFunc(void* s1 ,void* s2);
                static void* CallerMainFunc(void* s1 ,void* s2);
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
#if FIBER_TYPE == FIBER_UCONTEXT             // 协程上下文结构---暂时支持三种类型
                ucontext_t _ctx;
#elif FIBER_TYPE == FIBER_FCONTEXT
                Xten::fcontext_t _ctx;
#elif FIBER_TYPE == FIBER_COCTX
                coctx_t _ctx;
#endif
                char _stack[]; // 柔性数组 指向协程栈空间
        };
}
#endif
