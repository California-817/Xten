#ifndef __XTEN_MUTEX_H__
#define __XTEN_MUTEX_H__
#include "const.h"
#include "nocopyable.hpp"
#include <pthread.h>
#include <semaphore.h>
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
    // 4.信号量
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

}
#endif