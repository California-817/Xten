#include "../include/fiber.h"
#include "log.h"
#include <cstring>
#include <sys/mman.h>
#include "scheduler.h"
#include "config.h"
#include "macro.h"
namespace Xten
{
    static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");

    static Xten::ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        Config::LookUp("fiber.stack_size", (uint32_t)128 * 1024, "fiber stack size");

    static int64_t s_total_num = 0; // 总协程数
    static int64_t s_fiber_id = 0;  // 协程id

    static thread_local Fiber::ptr t_main_fiber = nullptr; // 线程主协程
    static thread_local Fiber *t_cur_fiber = nullptr;      // 线程当前正在执行的协程

    // 协程栈空间开辟器
    class MallocStackAllocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }
        static void Dealloc(void *ptr)
        {
            free(ptr);
        }
    };
    class MmapStackAllocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        }
        static void Dealloc(void *ptr, size_t size)
        {
            munmap(ptr, size);
        }
    };
    using StackAllocatorType = MmapStackAllocator; // 使用mmap开辟栈空间

    Fiber *NewFiber()
    {
        return new Fiber();
    }
    Fiber *NewFiber(size_t stack_size, std::function<void()> func, bool use_caller)
    {
        stack_size = stack_size ? stack_size : g_fiber_stack_size->GetValue();
        // 将协程对象和协程栈空间开辟在同一连续区域
        void *fiber_ptr = StackAllocatorType::Alloc(sizeof(Fiber) + stack_size);
        return new (fiber_ptr) Fiber(stack_size, func, use_caller); // placement new
        // 柔性数组自动指向尾部多余的栈空间起始地址
    }
    // 这个FreeFiber用于子协程的删除---子协程智能指针删除器不能使用delete Fiber  -----使用自定义删除器为这个
    // 子协程的空间是比Fiber的空间更大的 并且不是new出来的 而是用空间开辟器开出来的
    void FreeFiber(Fiber *ptr)
    {
        ptr->~Fiber();
        StackAllocatorType::Dealloc(ptr, 0);
    }

    // 线程主协程默认构造
    Fiber::Fiber()
        : _fiber_id(0), _status(Status::EXEC), _stack_size(0)
    {
#if FIBER_TYPE == FIBER_UCONTEXT
        if (getcontext(&_ctx)) // 主协程上下文为当前线程
        {
            XTEN_LOG_ERROR(g_logger) << "main fiber getcontext failed, errno:"
                                     << errno << " errstring:" << strerror(errno);
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
        //默认主协程的fcontext不需要初始化
#endif
        SetThis(this); // 设置当前运行协程
        s_total_num++;
        XTEN_LOG_DEBUG(g_logger) << "main fiber create success";
    }

    // 子协程构造函数
    Fiber::Fiber(size_t stack_size, std::function<void()> func, bool use_caller)
        : _fiber_id(++s_fiber_id), _stack_size(stack_size), _func(func), _user_caller(use_caller)
    {
#if FIBER_TYPE == FIBER_UCONTEXT
        if (getcontext(&_ctx))
        {
            XTEN_ASSERTINFO(false, "getcontext error");
        }
        _ctx.uc_link = nullptr;
        _ctx.uc_stack.ss_size = _stack_size;
        _ctx.uc_stack.ss_sp = (char *)_stack;
        if (_user_caller)
        {                                                  // 加入调度
            makecontext(&_ctx, &Fiber::CallerMainFunc, 0); // 创建子协程上下文
        }
        else
        {
            makecontext(&_ctx, &Fiber::MainFunc, 0);
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
        if (_user_caller)
        {
            _ctx =Xten::make_fcontext((char *)_stack + _stack_size, _stack_size, &Fiber::CallerMainFunc);
        } // 这里栈空间的首地址是高地址
        else
        {
            _ctx =Xten::make_fcontext((char *)_stack + _stack_size, _stack_size, &Fiber::MainFunc);
        }

#endif
        s_total_num++;
        XTEN_LOG_DEBUG(g_logger) << "Fiber::id=" << _fiber_id << " create success";
    }

    Fiber::~Fiber()
    {
        --s_total_num;
        if (_stack_size) // 子协程析构
        {
            XTEN_ASSERT((_status == Status::EXCEPT ||
                        _status ==Status::INIT ||
                        _status ==Status::TERM))
        }
        else // 主协程析构
        {
            XTEN_ASSERT(!_func);
            XTEN_ASSERT(_status == Status::EXEC);
            Fiber *cur = t_cur_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }
        XTEN_LOG_DEBUG(g_logger) << "~Fiber:id=" << _fiber_id
                                 << " total fiber nums=" << s_total_num;
    }

    // 切入协程
    void Fiber::SwapIn() // 调度协程切到目标协程
    {
        XTEN_ASSERT((_status != Status::EXEC));
        SetThis(this);
        _status = Status::EXEC;
#if FIBER_TYPE == FIBER_UCONTEXT
        if (swapcontext(&Scheduler::GetScheduleFiber()->_ctx, &_ctx) == -1)
        {
            // 切换失败
            XTEN_ASSERTINFO(false, "call context failed");
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
            // 这里切进去不会为 t_main_fiber->_ctx 赋值 仍为nullptr    从里面切出来的时候才会赋值
            //第二个参数为需要传入协程执行函数的用户数据
            Xten::jump_fcontext(&Scheduler::GetScheduleFiber()->_ctx,_ctx,0);
#endif
    }

    void Fiber::Call() // 主协程切到目标协程
    {
        SetThis(this);
        _status = Status::EXEC;
#if FIBER_TYPE == FIBER_UCONTEXT
        if (swapcontext(&t_main_fiber->_ctx, &_ctx) == -1)
        {
            // 切换失败
            XTEN_ASSERTINFO(false, "call context failed");
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
            // 这里切进去不会为 t_main_fiber->_ctx 赋值 仍为nullptr    从里面切出来的时候才会赋值
            Xten::jump_fcontext(&t_main_fiber->_ctx,_ctx,0);
#endif
    }

    // 切出协程
    void Fiber::SwapOut() // 切回调度协程
    {
        SetThis(Scheduler::GetScheduleFiber());
#if FIBER_TYPE == FIBER_UCONTEXT
        if (swapcontext(&_ctx, &Scheduler::GetScheduleFiber()->_ctx) == -1)
        {
            // 切换失败
            XTEN_ASSERTINFO(false, "back context failed");
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
        Xten::jump_fcontext(&_ctx,Scheduler::GetScheduleFiber()->_ctx, 0);
#endif
    }

    // 切出状态为hold
    void Fiber::YieldToHold()
    {
        Fiber::ptr fb = GetThis();
        XTEN_ASSERT(fb->_status == Status::EXEC);
        // fb->_status=Status::HOLD;  ---在Schedule调度后进行状态设置
        fb->SwapOut();
    }

    // 切出状态为Ready
    void Fiber::YieldToReady()
    {
        Fiber::ptr fb = GetThis();
        XTEN_ASSERT(fb->_status == Status::EXEC);
        fb->_status = Status::READY;
        fb->SwapOut();
    }

    void Fiber::Back() // 切回主协程
    {
        SetThis(t_main_fiber.get());
#if FIBER_TYPE == FIBER_UCONTEXT
        if (swapcontext(&_ctx, &t_main_fiber->_ctx) == -1)
        {
            // 切换失败
            XTEN_ASSERTINFO(false, "back context failed");
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
        Xten::jump_fcontext(&_ctx,t_main_fiber->_ctx, 0);
#endif
    }

    // 重置协程
    void Fiber::Reset(std::function<void()> func)
    {
        XTEN_ASSERT(_stack);
        XTEN_ASSERT((_status == Status::INIT ||
                    _status ==Status::EXCEPT ||
                    _status ==Status::TERM));
        _func = func;
#if FIBER_TYPE == FIBER_UCONTEXT
        if (getcontext(&_ctx))
        {
            XTEN_ASSERTINFO(false, "getcontext error");
        }
        _ctx.uc_link = nullptr;
        _ctx.uc_stack.ss_size = _stack_size;
        _ctx.uc_stack.ss_sp = _stack;
        if (_user_caller)
        {                                                  // 加入调度
            makecontext(&_ctx, &Fiber::CallerMainFunc, 0); // 创建子协程上下文
        }
        else
        {
            makecontext(&_ctx, &Fiber::MainFunc, 0);
        }
#elif FIBER_TYPE == FIBER_FCONTEXT
        if (_user_caller)
        {
            _ctx = Xten::make_fcontext((char *)_stack + _stack_size, _stack_size, &Fiber::CallerMainFunc);
        } // 这里栈空间的首地址是高地址
        else
        {
            _ctx = Xten::make_fcontext((char *)_stack + _stack_size, _stack_size, &Fiber::MainFunc);
        }

#endif
        _status = Status::INIT;
    }

    // 获取协程状态
    Fiber::Status Fiber::GetStatus() const
    {
        return _status;
    }

    // 获取协程id
    size_t Fiber::GetFiberId()
    {
        if (t_cur_fiber)
        {
            return t_cur_fiber->_fiber_id;
        }
        return 0;
    }

    // 协程的真正入口函数--非用户传入
#if FIBER_TYPE == FIBER_UCONTEXT
    void Fiber::MainFunc() {
#elif FIBER_TYPE == FIBER_FCONTEXT
    void Fiber::MainFunc(intptr_t t){
#endif
        // 获取当前协程
        Fiber::ptr cur = GetThis();
        XTEN_ASSERT(cur);
        try
        {
            cur->_func();
            cur->_func = nullptr;
            cur->_status = Status::TERM;
        }
        catch (const std::exception &e)
        {
            cur->_func = nullptr;
            cur->_status = Status::EXCEPT;
            XTEN_LOG_ERROR(g_logger) << "Fiber Except: " << e.what()
                                     << " fiber_id=" << cur->GetFiberId()
                                     << std::endl
                                     << Xten::BackTraceUtil::backtraceTostring(100);
        }
        catch (...)
        {
            cur->_func = nullptr;
            cur->_status = Status::EXCEPT;
            XTEN_LOG_ERROR(g_logger) << "Fiber Except: "
                                     << " fiber_id=" << cur->GetFiberId()
                                     << std::endl
                                     << Xten::BackTraceUtil::backtraceTostring(100);
        }
        // 协程执行完毕---必须取出裸指针并删除智能指针 ---切出去不会回来
        // 不会走到末尾析构智能指针---引用计数永远+1
        Fiber *ptr = cur.get();
        cur.reset(); // 提前将引用计数--
        ptr->SwapOut();
        XTEN_ASSERTINFO(false, "fiber never swapin to here fiberid=" + std::to_string((int)cur->GetFiberId()));
    }
#if FIBER_TYPE == FIBER_UCONTEXT
    void Fiber::CallerMainFunc(){
#elif FIBER_TYPE == FIBER_FCONTEXT
    void Fiber::CallerMainFunc(intptr_t t){
#endif
        // 获取当前协程
        Fiber::ptr cur = GetThis();
        XTEN_ASSERT(cur);
        try
        {
            cur->_func();
            cur->_func = nullptr;
            cur->_status = Status::TERM;
        }
        catch (const std::exception &e)
        {
            cur->_func = nullptr;
            cur->_status = Status::EXCEPT;
            XTEN_LOG_ERROR(g_logger) << "Fiber Except: " << e.what()
                                     << " fiber_id=" << cur->GetFiberId()
                                     << std::endl
                                     << Xten::BackTraceUtil::backtraceTostring(100);
        }
        catch (...)
        {
            cur->_func = nullptr;
            cur->_status = Status::EXCEPT;
            XTEN_LOG_ERROR(g_logger) << "Fiber Except: "
                                     << " fiber_id=" << cur->GetFiberId()
                                     << std::endl
                                     << Xten::BackTraceUtil::backtraceTostring(100);
        }
        // 协程执行完毕---必须取出裸指针并删除智能指针 ---切出去不会回来
        // 不会走到末尾析构智能指针---引用计数永远+1
        Fiber *ptr = cur.get();
        cur.reset(); // 提前将引用计数--
        ptr->Back();
        XTEN_ASSERTINFO(false, "fiber never swapin to here fiberid=" + std::to_string((int)cur->GetFiberId()));
    }

    // 设置线程当前协程
    void Fiber::SetThis(Fiber *ts)
    {
        t_cur_fiber = ts;
    }

    // 获取线程当前协程
    Fiber::ptr Fiber::GetThis()
    {
        if (t_cur_fiber)
        {
            return t_cur_fiber->shared_from_this();
        }
        // 没有运行协程---肯定要创建主协程
        Fiber::ptr main_fiber(NewFiber());
        XTEN_ASSERT((main_fiber.get() == t_cur_fiber));
        t_main_fiber = main_fiber; // 线程主协程赋值
        auto p = t_cur_fiber->shared_from_this();
        return p;
    }

    int64_t Fiber::GetTotalFiberNums()
    {
        return s_total_num;
    }
}