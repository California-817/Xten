#ifndef __XTEN_MUTEX_H__
#define __XTEN_MUTEX_H__
#include "const.h"
#include "nocopyable.hpp"
#include <pthread.h>
#include "fiber.h"
#include <semaphore.h>
#include <condition_variable>
namespace Xten
{
    // 自动加解锁的lockguard
    template <class T> // 模板定义放在头文件
    class XtenLockguard
    {
    public:
        XtenLockguard(T &mtx) : _mtx(mtx), _b_lock(false)
        {
            _mtx.lock(); // 自动加锁
            _b_lock = true;
        }
        void lock()
        {
            if (!_b_lock)
            {
                _mtx.lock(); // 防止死锁
                _b_lock = true;
            }
        }
        void unlock()
        {
            if (_b_lock)
            {
                _mtx.unlock();
                _b_lock = false;
            }
        }
        ~XtenLockguard()
        {
            unlock();
        }

    private:
        T &_mtx; // 不允许拷贝
        bool _b_lock;
    };
    // 读锁自动加解锁
    template <class T>
    class XtenReadLockguard
    {
    public:
        XtenReadLockguard(T &mtx) : _mtx(mtx), _b_lock(false)
        {
            _mtx.rdlock(); // 自动加锁
            _b_lock = true;
        }
        void lock()
        {
            if (!_b_lock)
            {
                _mtx.rdlock(); // 防止死锁
                _b_lock = true;
            }
        }
        void unlock()
        {
            if (_b_lock)
            {
                _mtx.unlock();
                _b_lock = false;
            }
        }
        ~XtenReadLockguard()
        {
            unlock();
        }

    private:
        T &_mtx; // 不允许拷贝
        bool _b_lock;
    };
    template <class T>
    class XtenWriteLockguard
    {
    public:
        XtenWriteLockguard(T &mtx) : _mtx(mtx), _b_lock(false)
        {
            _mtx.wrlock(); // 自动加锁
            _b_lock = true;
        }
        void lock()
        {
            if (!_b_lock)
            {
                _mtx.wrlock(); // 防止死锁
                _b_lock = true;
            }
        }
        void unlock()
        {
            if (_b_lock)
            {
                _mtx.unlock();
                _b_lock = false;
            }
        }
        ~XtenWriteLockguard()
        {
            unlock();
        }

    private:
        T &_mtx; // 不允许拷贝
        bool _b_lock;
    };
    // 读锁自动加解锁
    template <class T>
    class XtenWPReadLockguard
    {
    public:
        XtenWPReadLockguard(T &mtx) : _mtx(mtx), _b_lock(false)
        {
            _mtx.rdlock(); // 自动加锁
            _b_lock = true;
        }
        void lock()
        {
            if (!_b_lock)
            {
                _mtx.rdlock(); // 防止死锁
                _b_lock = true;
            }
        }
        void unlock()
        {
            if (_b_lock)
            {
                _mtx.rdUnlock();
                _b_lock = false;
            }
        }
        ~XtenWPReadLockguard()
        {
            unlock();
        }

    private:
        T &_mtx; // 不允许拷贝
        bool _b_lock;
    };
    template <class T>
    class XtenWPWriteLockguard
    {
    public:
        XtenWPWriteLockguard(T &mtx) : _mtx(mtx), _b_lock(false)
        {
            _mtx.wrlock(); // 自动加锁
            _b_lock = true;
        }
        void lock()
        {
            if (!_b_lock)
            {
                _mtx.wrlock(); // 防止死锁
                _b_lock = true;
            }
        }
        void unlock()
        {
            if (_b_lock)
            {
                _mtx.wrUnlock();
                _b_lock = false;
            }
        }
        ~XtenWPWriteLockguard()
        {
            unlock();
        }

    private:
        T &_mtx; // 不允许拷贝
        bool _b_lock;
    };
    // 1.互斥锁
    class Mutex : public NoCopyable
    {
    public:
        typedef XtenLockguard<Mutex> Lock; // 简化lockguard的使用
        Mutex()
        {
            pthread_mutex_init(&_mutex, nullptr);
        }
        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }

    private:
        pthread_mutex_t _mutex;
    };
    // 2.读写锁
    class RWMutex : public NoCopyable
    {
    public:
        typedef XtenWriteLockguard<RWMutex> WriteLock; // 简化lockguard的使用
        typedef XtenReadLockguard<RWMutex> ReadLock;   // 简化lockguard的使用
        RWMutex()
        {
            pthread_rwlock_init(&_rw_lock, nullptr);
        }
        void rdlock()
        {
            pthread_rwlock_rdlock(&_rw_lock);
        }
        void wrlock()
        {
            pthread_rwlock_wrlock(&_rw_lock);
        }
        void unlock()
        {
            pthread_rwlock_unlock(&_rw_lock);
        }
        ~RWMutex()
        {
            pthread_rwlock_destroy(&_rw_lock);
        }

    private:
        pthread_rwlock_t _rw_lock; // 读写锁
    };
    // 2.2读写锁（写优先）
    class WPRWMutex : public NoCopyable
    {
    public:
        typedef XtenWPReadLockguard<WPRWMutex> ReadLock;
        typedef XtenWPWriteLockguard<WPRWMutex> WriteLock;
        WPRWMutex()
            : _activeReader(0),
              _activeWriter(false),
              _waitWriter(0)
        {
        }
        // 写加锁
        void wrlock()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _waitWriter++;
            while (_activeReader || _activeWriter)
            {
                _writeCv.wait(lock);
            }
            // 没有读线程占有锁
            _activeWriter = true;
            _waitWriter--;
        }
        // 读加锁
        void rdlock()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            while (_activeWriter || _waitWriter)
            {
                // 有写线程独占，或者有写线程等待锁（写优先策略）
                _readCv.wait(lock);
            }
            // 无写者
            _activeReader++;
        }
        // 写解锁
        void wrUnlock()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _activeWriter = false;
            if (_waitWriter)
            {
                // 有写者等待 优先唤醒写者
                _writeCv.notify_one();
            }
            else
            {
                // 无写者唤醒可能的读者
                _readCv.notify_all();
            }
        }
        // 读解锁
        void rdUnlock()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _activeReader--;
            if (_waitWriter && _activeReader == 0)
            {
                // 有写者等待 优先唤醒写者
                _writeCv.notify_one();
            }
            else
            {
                // 无写者唤醒可能的读者
                _readCv.notify_all();
            }
        }

    private:
        int _activeReader;                // 正在进行读操作的read线程数量
        bool _activeWriter;               // 是否有正在独占写操作的写线程
        int _waitWriter;                  // 正在等待锁资源的写线程（写优先）
        std::condition_variable _readCv;  // 读线程等待条件变量
        std::condition_variable _writeCv; // 写线程等待条件变量
        std::mutex _mutex;                // 互斥锁
    };

    // 3.自旋锁
    class SpinLock : public NoCopyable
    {
    public:
        typedef XtenLockguard<SpinLock> Lock; // 简化lockguard的使用
        SpinLock()
        {
            pthread_spin_init(&_spin_lock, 0);
        }
        void lock()
        {
            pthread_spin_lock(&_spin_lock);
        }
        void unlock()
        {
            pthread_spin_unlock(&_spin_lock);
        }
        ~SpinLock()
        {
            pthread_spin_destroy(&_spin_lock);
        }

    private:
        pthread_spinlock_t _spin_lock; // 自旋锁
    };
    // 4.信号量(线程级别)
    class Semaphore : public NoCopyable
    {
    public:
        Semaphore(uint32_t count = 0);
        void wait(); // 等待信号量
        void post(); // 释放信号量
        ~Semaphore();

    private:
        sem_t _semaphore; // 信号量
    };
    // 5.协程信号量--控制协程的并发，[并且还能保证只是进行协程等待挂起而不是直接挂起线程]
    class Scheduler;
    class FiberSemphore : public NoCopyable
    {
    public:
        FiberSemphore(size_t initial_concurrency = 0);
        // 尝试获取协程信号量（非阻塞）
        bool trywait();
        // 获取协程信号量（未获取成功 协程阻塞挂起）
        void wait();
        // 释放信号量
        void post();
        // 唤醒所有等待信号量的协程
        void notifyAll();
        ~FiberSemphore();

    private:
        // 这个线程锁是为了保证协程在多线程情况下操作信号量和等待队列的线程安全
        SpinLock _mutex;
        size_t _concurrency;                                      // 当前信号量的值
        std::list<std::pair<Scheduler *, Fiber::ptr>> _waitQueue; // 协程未获取到信号量的等待队列
    };
    // 协程锁(相比于线程锁的优点)
    // 1.协程环境友好：框架本身就是基于协程的网络框架，协程锁在协程环境中比线程锁更轻量级
    // 2.避免线程阻塞：协程锁在等待时只会挂起当前协程，不会阻塞整个线程
    // 3.更好的并发性能：在高并发场景下，协程锁的切换开销比线程锁小得多
    class Scheduler;
    class FiberMutex : public NoCopyable
    {
    public:
        typedef XtenLockguard<FiberMutex> Lock;
        FiberMutex();
        // 加锁
        void lock();
        // 尝试加锁
        bool tryLock();
        // 解锁
        void unlock();
        ~FiberMutex();

    private:
        SpinLock _mutex;
        std::list<std::pair<Scheduler *, Fiber::ptr>> _waitQueue;
        std::atomic_bool _lock;
    };
}
#endif