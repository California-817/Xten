#include "mutex.h"
#include "macro.h"
#include "scheduler.h"
namespace Xten
{
    Semaphore::Semaphore(uint32_t count)
    {
        if (sem_init(&_semaphore, 0, count))
        {
            std::cout << "init semaphore error" << std::endl;
        }
    }
    // 等待信号量
    void Semaphore::wait()
    {
        if (sem_wait(&_semaphore))
        {
            throw std::logic_error("wait semaphore error");
        }
    }
    // 释放信号量
    void Semaphore::post()
    {
        if (sem_post(&_semaphore))
        {
            throw std::logic_error("post semaphore error");
        }
    }
    Semaphore::~Semaphore()
    {
        if (sem_destroy(&_semaphore))
        {
            std::cout << "destory semaphore error" << std::endl;
        }
    }
    FiberSemphore::FiberSemphore(size_t initial_concurrency)
        : _concurrency(initial_concurrency)
    {
    }
    // 尝试获取协程信号量（非阻塞）
    bool FiberSemphore::trywait()
    {
        XTEN_ASSERT(Scheduler::GetThis());
        {
            SpinLock::Lock lock(_mutex);
            if (_concurrency > 0)
            {
                // 可以获取信号量
                _concurrency--;
                return true;
            }
            return false;
        }
    }
    // 获取协程信号量（未获取成功 协程阻塞挂起）
    void FiberSemphore::wait()
    {
        XTEN_ASSERT(Scheduler::GetThis());
        {
            SpinLock::Lock lock(_mutex);
            if (_concurrency > 0u)
            {
                // 可以获取信号量
                _concurrency--;
                return;
            }
            // 获取失败-->协程需要挂起
            _waitQueue.push_back(std::make_pair(Scheduler::GetThis(), Fiber::GetThis()));
        }
        // 这里锁已经释放--->{}防止协程挂起锁未释放导致其他协程获取不了信号量
        Fiber::YieldToHold(); // 协程挂起
    }
    // 释放信号量
    void FiberSemphore::post()
    {
        XTEN_ASSERT(Scheduler::GetThis());
        {
            SpinLock::Lock lock(_mutex);
            if (!_waitQueue.empty())
            {
                // 等待队列不为空-->释放信号量直接唤醒一个协程即可（不需要++信号量）
                auto fiber = _waitQueue.front();
                _waitQueue.pop_front();
                // 重新调度协程
                fiber.first->Schedule(fiber.second);
                return;
            }
            else
            {
                // 等待队列此时为空，++信号量
                _concurrency++;
                return;
            }
        }
    }
    // 唤醒所有等待信号量的协程
    void FiberSemphore::notifyAll()
    {
        SpinLock::Lock lock(_mutex);
        for (auto &fiber : _waitQueue)
        {
            // 调度所有等待协程
            fiber.first->Schedule(fiber.second);
        }
        _waitQueue.clear();
    }
    FiberSemphore::~FiberSemphore()
    {
        // 析构的时候需要保证没有等待协程
        XTEN_ASSERT(_waitQueue.empty());
    }







    FiberMutex::~FiberMutex()
    {
#ifndef NDEBUG
        Xten::Mutex::Lock scopeLock(m_mutex);
        XTEN_ASSERT(!m_owner);
        XTEN_ASSERT(m_waiters.empty());
#endif
    }

    void
    FiberMutex::lock()
    {
        XTEN_ASSERT(Scheduler::GetThis());
        {
            Xten::Mutex::Lock scopeLock(m_mutex);
            XTEN_ASSERT(m_owner != Fiber::GetThis());
            XTEN_ASSERT(std::find(m_waiters.begin(), m_waiters.end(),
                                  std::make_pair(Scheduler::GetThis(), Fiber::GetThis())) == m_waiters.end());
            if (!m_owner)
            {
                // 无协程持有锁
                m_owner = Fiber::GetThis();
                return;
            }
            // 已有协程持有锁，当前协程加入等待队列
            m_waiters.push_back(std::make_pair(Scheduler::GetThis(),
                                               Fiber::GetThis()));
        }
        Fiber::YieldToHold();
#ifndef NDEBUG
        Xten::Mutex::Lock scopeLock(m_mutex);
        XTEN_ASSERT(m_owner == Fiber::GetThis());
        XTEN_ASSERT(std::find(m_waiters.begin(), m_waiters.end(),
                              std::make_pair(Scheduler::GetThis(), Fiber::GetThis())) == m_waiters.end());
#endif
    }

    void
    FiberMutex::unlock()
    {
        Xten::Mutex::Lock lock(m_mutex);
        unlockNoLock();
    }

    bool
    FiberMutex::unlockIfNotUnique()
    {
        Xten::Mutex::Lock lock(m_mutex);
        XTEN_ASSERT(m_owner == Fiber::GetThis());
        if (!m_waiters.empty())
        {
            unlockNoLock();
            return true;
        }
        return false;
    }

    void
    FiberMutex::unlockNoLock()
    {
        XTEN_ASSERT(m_owner == Fiber::GetThis());
        m_owner.reset();
        if (!m_waiters.empty())
        {
            std::pair<Scheduler *, Fiber::ptr> next = m_waiters.front();
            m_waiters.pop_front();
            m_owner = next.second;
            next.first->Schedule(next.second);
        }
    }




    FiberCondition::~FiberCondition()
    {
#ifndef NDEBUG
        Xten::Mutex::Lock lock(m_mutex);
        XTEN_ASSERT(m_waiters.empty());
#endif
    }

    void
    FiberCondition::wait()
    {
        XTEN_ASSERT(Scheduler::GetThis());
        {
            Xten::Mutex::Lock lock(m_mutex); //保证只有一个线程上的协程能操作等待队列
            Xten::Mutex::Lock lock2(m_fiberMutex.m_mutex); //保证协程锁中的状态正确
            XTEN_ASSERT(m_fiberMutex.m_owner == Fiber::GetThis());
            //将当前协程放入等待队列
            m_waiters.push_back(std::make_pair(Scheduler::GetThis(),
                                               Fiber::GetThis()));
            //释放协程锁
            m_fiberMutex.unlockNoLock();
        }
        //挂起协程
        Fiber::YieldToHold();
#ifndef NDEBUG
        Xten::Mutex::Lock lock2(m_fiberMutex.m_mutex);
        XTEN_ASSERT(m_fiberMutex.m_owner == Fiber::GetThis());
#endif
    }

    void
    FiberCondition::signal()
    {
        //获取一个等待的协程
        std::pair<Scheduler *, Fiber::ptr> next;
        {
            Xten::Mutex::Lock lock(m_mutex);
            if (m_waiters.empty())
                return;
            next = m_waiters.front();
            m_waiters.pop_front();
        }
        Xten::Mutex::Lock lock2(m_fiberMutex.m_mutex);
        //查看协程锁状态
        XTEN_ASSERT(m_fiberMutex.m_owner != next.second); //该等待协程不能持有锁且不能等待锁
        XTEN_ASSERT(std::find(m_fiberMutex.m_waiters.begin(),
                              m_fiberMutex.m_waiters.end(), next) == m_fiberMutex.m_waiters.end());
        if (!m_fiberMutex.m_owner)
        {
            //协程锁没被获取，直接获取锁并调度该协程
            m_fiberMutex.m_owner = next.second;
            next.first->Schedule(next.second);
        }
        else
        {
            //协程锁已被获取，将该协程加入锁的等待队列----->记住这是条件变量的定义
            m_fiberMutex.m_waiters.push_back(next);
        }
    }

    void
    FiberCondition::broadcast()
    {
        Xten::Mutex::Lock lock(m_mutex);
        if (m_waiters.empty())
            return;
        Xten::Mutex::Lock lock2(m_fiberMutex.m_mutex);

        std::list<std::pair<Scheduler *, Fiber::ptr>>::iterator it;
        for (it = m_waiters.begin();
             it != m_waiters.end();
             ++it)
        {
            std::pair<Scheduler *, Fiber::ptr> &next = *it;
            XTEN_ASSERT(m_fiberMutex.m_owner != next.second);
            XTEN_ASSERT(std::find(m_fiberMutex.m_waiters.begin(),
                                  m_fiberMutex.m_waiters.end(), next) == m_fiberMutex.m_waiters.end());
            if (!m_fiberMutex.m_owner)
            {
                m_fiberMutex.m_owner = next.second;
                next.first->Schedule(next.second);
            }
            else
            {
                m_fiberMutex.m_waiters.push_back(next);
            }
        }
        m_waiters.clear();
    }

}