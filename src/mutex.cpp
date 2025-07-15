#include "../include/mutex.h"
#include "../include/macro.h"
#include "../include/scheduler.h"
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
            //调度所有等待协程
            fiber.first->Schedule(fiber.second);
        }
        _waitQueue.clear();
    }
    FiberSemphore::~FiberSemphore()
    {
        //析构的时候需要保证没有等待协程
        XTEN_ASSERT(_waitQueue.empty());
    }
}