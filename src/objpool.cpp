#include "objpool.h"
#include "config.h"
#include "macro.h"
namespace Xten
{

    static Xten::ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        Config::LookUp("fiber.stack_size", (uint32_t)128 * 1024, "fiber stack size");
    // 定义协程对象池类型
    // 将 Fiber 放在结构体开头，这样返回的指针可以直接作为 Fiber* 使用
    template <size_t N>
    struct FiberType
    {
        // 大小为一个协程对象+协程栈大小
        char stack[N];
    };
    // 每个线程都实现一个协程objpool---->有错误：协程的创建和删除可能会跨线程
    // 解决办法：使用全局对象池+协程锁---性能会下降
    static ObjPool<FiberType<128 * 1024 + 88>> *t_fiber_objpool = nullptr;

    void *ObjPoolAllocator::Alloc(size_t size)
    {
        (void)size;
        if (!t_fiber_objpool)
        {
            t_fiber_objpool = new ObjPool<FiberType<128 * 1024 + 88>>();
        }
        return t_fiber_objpool->New();
    }
    void ObjPoolAllocator::Dealloc(void *ptr, size_t size)
    {
        (void)size;
        XTEN_ASSERT(t_fiber_objpool);
        t_fiber_objpool->Delete((FiberType<128 * 1024 + 88> *)ptr);
    }

    // interface
    Fiber *NewFiberFromObjPool(size_t stack_size, std::function<void()> func)
    {
        stack_size = 128 * 1024; // 固定栈大小
        void *ptr = ObjPoolAllocator::Alloc(sizeof(Fiber) + stack_size);
        return new (ptr) Fiber(stack_size, func, false); // placement new
    }
    void FreeFiberToObjPool(Fiber *ptr)
    {
        ptr->~Fiber();
        ObjPoolAllocator::Dealloc(ptr, sizeof(Fiber) + ptr->_stack_size);
    }

    std::string FiberObjPoolInfo()
    {
        if (t_fiber_objpool)
        {
            return "ObjPool Fiber hit:" + std::to_string(t_fiber_objpool->s_objpool_hit_counter) +
                   " free:" + std::to_string(t_fiber_objpool->s_objpool_free_counter) +
                   " mmap_syscall:" + std::to_string(t_fiber_objpool->s_mmap_syscall_counter)+"\n";
        }
    }
}