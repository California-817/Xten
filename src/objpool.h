#ifndef __XTEN_OBJPOOL_H__
#define __XTEN_OBJPOOL_H__
#include "fiber.h"
#include <sys/mman.h>
#include "log.h"
#include "nocopyable.hpp"
#include "mutex.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
#define MMAP_SIZE 132 * 1024 * 20 // 20个协程的空间
    // 定义linux平台向os申请堆空间的函数
    inline void *SystemCallMemory(size_t size)
    {
#ifdef __linux__
        void *ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED)
            return nullptr;
#endif
        return ptr;
    }
    // 非线程安全对象池模板类-----使用空闲链表实现
    template <class T>
    class ObjPool : public NoCopyable
    {
    public:
        friend std::string FiberObjPoolInfo();
        typedef Mutex MutexType;
        ObjPool();
        ~ObjPool();
        // 返回一个对象的指针
        T *New();
        // 释放一个指针指向的对象
        void Delete(T *ptr);

    private:
        char *_memory;                       // 指向可用内存空间
        size_t _freeCapacity;                // 剩余空间大小
        void *_freeList;                     // 空闲链表管理上层释放的空间
        MutexType _mutex;                    // 对象池锁
        uint32_t s_mmap_syscall_counter = 0; // mmap次数
        uint32_t s_objpool_hit_counter = 0;  // 对象池命中次数
        uint32_t s_objpool_free_counter = 0; // 对象池未命中次数
    };

    template <class T>
    ObjPool<T>::ObjPool()
        : _memory(nullptr),
          _freeCapacity(0),
          _freeList(nullptr)
    {
    }
    template <class T>
    ObjPool<T>::~ObjPool()
    {
        // 析构定长内存池需要保证内存都已经释放----暂时不考虑，因为上层应用层会一直使用
        XTEN_LOG_INFO(g_logger) << "ObjPool<T>::~ObjPool()" << std::endl;
    }
    // 返回一个对象的指针
    template <class T>
    T *ObjPool<T>::New()
    {
        T *obj = nullptr;
        // 1.先看空闲链表是否有回收空间
        MutexType::Lock lock(_mutex);
        if (_freeList)
        {
            // 拿到下一个内存块地址
            obj = (T *)_freeList;
            _freeList = *((void **)_freeList);
            s_objpool_hit_counter++;
        } // 无回收空间
        else
        {
            size_t objSize = sizeof(T) > sizeof(void *) ? sizeof(T) : sizeof(void *);
            if (_freeCapacity < objSize) // 这一块空间的剩余空间不足一个对象(剩余的空间不会被利用，内部碎片问题)
            {
                _freeCapacity = MMAP_SIZE; // 一次开辟20个协程的空间
                // 重新开辟空间
                // 原来的空间都被应用层使用，后续归还到空闲链表继续使用，无需关心释放问题
                _memory = (char *)SystemCallMemory(_freeCapacity);
                s_mmap_syscall_counter++;
                if (_memory == nullptr) // failed
                    throw std::bad_alloc();
            }
            // success or 剩余空间足够
            obj = (T *)_memory;
            _memory += objSize;
            _freeCapacity -= objSize;
        }
        return obj;
    }
    // 释放一个指针指向的对象
    template <class T>
    void ObjPool<T>::Delete(T *ptr)
    {
        MutexType::Lock lock(_mutex);
        // 2.归还空间到空闲链表---头插
        *((void **)ptr) = _freeList;
        _freeList = ptr;
        s_objpool_free_counter++;
    }

    class ObjPoolAllocator
    {
    public:
        static void *Alloc(size_t size);
        static void Dealloc(void *ptr, size_t size);
    };

    // interface -----> 性能反而下降？？？服了
    Fiber *NewFiberFromObjPool(size_t stack_size, std::function<void()> func);
    void FreeFiberToObjPool(Fiber *ptr);

    std::string FiberObjPoolInfo();
} // namespace Xten

#endif