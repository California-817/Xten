#include "../include/fiber.h"
#include "log.h"
#include <cstring>
#include <sys/mman.h>
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

#endif
        s_total_num++;
        XTEN_LOG_DEBUG(g_logger) << "Fiber::id=" << _fiber_id << " create success";
    }

    Fiber::~Fiber()
    {
        --s_total_num;
        if (_stack) // 子协程析构
        {
            XTEN_ASSERT(_status == Status::EXCEPT ||
                        Status::INIT ||
                        Status::TERM)
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
    void Fiber::SwapIn()
    {
    }

    void Fiber::Sall()
    {
    }

    // 切出协程
    void Fiber::SwapOut()
    {
    }

    // 切出状态为hold
    void Fiber::YieldToHold()
    {
    }

    // 切出状态为Ready
    void Fiber::YieldToReady()
    {
    }

    void Fiber::Back()
    {
    }

    // 重置协程
    void Fiber::Reset()
    {
    }

    // 获取协程状态
    Fiber::Status Fiber::GetStatus() const
    {
        return _status;
    }

    // 获取协程id
    size_t Fiber::GetFiberId() const
    {
        return _fiber_id;
    }

    // 协程的真正入口函数--非用户传入
    void Fiber::MainFunc()
    {
    }

    void Fiber::CallerMainFunc()
    {
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
        return t_cur_fiber->shared_from_this();
    }

    int64_t Fiber::GetTotalFiberNums()
    {
        return s_total_num;
    }
}