#include "timer.h"
#include "util.h"

namespace Xten
{
    bool Timer::Comparator::operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const
    {
        if (!lhs && !rhs)
        {
            return false;
        }
        if (!lhs)
        {
            return true;
        }
        if (!rhs)
        {
            return false;
        }
        if (lhs->m_next < rhs->m_next)
        {
            return true;
        }
        if (rhs->m_next < lhs->m_next)
        {
            return false;
        }
        return lhs.get() < rhs.get();
    }

    Timer::Timer(uint64_t ms, std::function<void()> cb,
                 bool recurring, TimerManager *manager)
        : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager)
    {
        m_next = TimeUitl::GetCurrentMS() + m_ms;
    }

    Timer::Timer(uint64_t next)
        : m_next(next)
    {
    }

    bool Timer::cancel()
    {
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if (m_cb)
        {
            m_cb = nullptr;
            auto it = m_manager->m_timers.find(shared_from_this());
            m_manager->m_timers.erase(it);
            return true;
        }
        return false;
    }

    bool Timer::refresh()
    {
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if (!m_cb)
        {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
        {
            return false;
        }
        m_manager->m_timers.erase(it);
        m_next = TimeUitl::GetCurrentMS() + m_ms;
        m_manager->m_timers.insert(shared_from_this());
        return true;
    }

    bool Timer::reset(uint64_t ms, bool from_now)
    {
        if (ms == m_ms && !from_now)
        {
            return true;
        }
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if (!m_cb)
        {
            return false;
        }
        auto it = m_manager->m_timers.find(shared_from_this());
        if (it == m_manager->m_timers.end())
        {
            return false;
        }
        m_manager->m_timers.erase(it);
        uint64_t start = 0;
        if (from_now)
        {
            start = TimeUitl::GetCurrentMS();
        }
        else
        {
            start = m_next - m_ms;
        }
        m_ms = ms;
        m_next = start + m_ms;
        m_manager->addTimer(shared_from_this(), lock);
        return true;
    }

    TimerManager::TimerManager()
    {
        m_previouseTime = TimeUitl::GetCurrentMS();
    }

    TimerManager::~TimerManager()
    {
    }

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring)
    {
        Timer::ptr timer = Xten::protected_make_shared<Timer>(ms, cb, recurring, this);
        RWMutexType::WriteLock lock(m_mutex);
        addTimer(timer, lock);
        return timer;
    }

    static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb)
    {
        std::shared_ptr<void> tmp = weak_cond.lock();
        if (tmp)
        {
            cb();
        }
    }

    Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring)
    {
        return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
    }

    uint64_t TimerManager::getNextTimer()
    {
        RWMutexType::ReadLock lock(m_mutex);
        m_tickled = false;
        if (m_timers.empty())
        {
            return ~0ull;
        }

        const Timer::ptr &next = *m_timers.begin();
        uint64_t now_ms = TimeUitl::GetCurrentMS();
        if (now_ms >= next->m_next)
        {
            return 0;
        }
        else
        {
            return next->m_next - now_ms;
        }
    }

    void TimerManager::listExpiredCb(std::vector<std::function<void()>> &cbs)
    {
        uint64_t now_ms = TimeUitl::GetCurrentMS();
        std::vector<Timer::ptr> expired;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if (m_timers.empty())
            {
                return;
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        if (m_timers.empty())
        {
            return;
        }
        bool rollover = detectClockRollover(now_ms);
        if (!rollover && ((*m_timers.begin())->m_next > now_ms))
        {
            return;
        }

        Timer::ptr now_timer = Xten::protected_make_shared<Timer>(now_ms);
        auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
        while (it != m_timers.end() && (*it)->m_next == now_ms)
        {
            ++it;
        }
        expired.insert(expired.begin(), m_timers.begin(), it);
        m_timers.erase(m_timers.begin(), it);
        cbs.reserve(expired.size());

        for (auto &timer : expired)
        {
            cbs.push_back(timer->m_cb);
            if (timer->m_recurring)
            {
                timer->m_next = now_ms + timer->m_ms;
                m_timers.insert(timer);
            }
            else
            {
                timer->m_cb = nullptr;
            }
        }
    }

    void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock &lock)
    {
        auto it = m_timers.insert(val).first;
        bool at_front = (it == m_timers.begin()) && !m_tickled;
        if (at_front)
        {
            m_tickled = true;
        }
        lock.unlock();

        if (at_front)
        {
            onTimerInsertedAtFront();
        }
    }

    bool TimerManager::detectClockRollover(uint64_t now_ms)
    {
        bool rollover = false;
        if (now_ms < m_previouseTime &&
            now_ms < (m_previouseTime - 60 * 60 * 1000))
        {
            rollover = true;
        }
        m_previouseTime = now_ms;
        return rollover;
    }

    bool TimerManager::hasTimer()
    {
        RWMutexType::ReadLock lock(m_mutex);
        return !m_timers.empty();
    }

    TimerW::TimerW(int expire, int sub, std::function<void()> callback, bool recurring, TimerWheelManager *manager)
        : _expire(expire),
          _sub_time(sub),
          _callback(callback),
          _recurring(recurring),
          _manager(manager)
    {
    }
    void TimerW::Cancel()
    {
        _cancel = true;
    }
    // 刷新定时器
    void TimerW::Refresh()
    {
        // 取消当前定时器
        _cancel = true;
        // 定时器中存的是间隔槽数量
        _manager->AddTimer(_sub_time * 10, _callback, _recurring);
    }
    static uint64_t gettime()
    {
        uint64_t t;
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
        struct timespec ti;
        clock_gettime(CLOCK_MONOTONIC, &ti); // CLOCK_MONOTONIC,从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
        t = (uint64_t)ti.tv_sec * (1000 / 10);
        t += ti.tv_nsec / (1000 * 1000 * 10);
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        t = (uint64_t)tv.tv_sec * (1000 / 10);
        t += tv.tv_usec / (1000 * 10);
#endif
        return t;
    }
    TimerWheelManager::TimerWheelManager()
        : _time(0),
          _current(0)
    {
        _near.resize(TIME_NEAR);
        _t.resize(4);
        for (int i = 0; i < _t.size(); i++)
        {
            _t[i].resize(TIME_LEVEL);
        }
        _current_point = gettime();
    }

    TimerW::ptr TimerWheelManager::AddTimer(int time_ms, std::function<void()> func, bool recurring)
    {
        // 传入的是ms
        // 一个槽对应是10ms-----定时器的精度确定
        time_ms = time_ms / 10;
        // 锁
        SpinLock::Lock lock(_mutex);
        TimerW::ptr timer = Xten::protected_make_shared<TimerW>(_time + time_ms, time_ms, func, recurring, this);
        if (time_ms <= 0)
        {
            timer->_callback();
            return nullptr;
        }
        // 添加任务结点到定时器中
        add_node(timer);
        return timer;
    }
    static void OnConditionTimer(std::function<void()> func, std::weak_ptr<void> cond)
    {
        std::shared_ptr<void> tmp = cond.lock();
        // 条件存在执行定时器回调函数
        if (tmp)
        {
            func();
        }
    }
    // 添加条件定时器
    void TimerWheelManager::AddConditionTimer(int time_ms, std::function<void()> func, std::weak_ptr<void> cond, bool recurring)
    {
        AddTimer(time_ms, std::bind(&OnConditionTimer, func, cond), recurring);
    }
    void TimerWheelManager::DelTimer(TimerW::ptr timer)
    {
        timer->_cancel = true;
    }
    void TimerWheelManager::CleatTimer()
    {
        // 锁
        SpinLock::Lock lock(_mutex);
        for (auto &TI : _near)
        {
            TI.clear();
        }
        for (int i = 0; i < 4; i++)
        {
            for (auto &TI : _t[i])
            {
                TI.clear();
            }
        }
    }
    void TimerWheelManager::ExpireTimer()
    {
        // 获取当前系统运行时间，不受系统时间被用户的影响
        uint64_t cp = gettime();
        // 当前系统启动时间与定时器记录的系统启动时间不相等
        if (cp != _current_point)
        {
            // 获取上述两者的差值
            uint32_t diff = (uint32_t)(cp - _current_point);
            // 更新定时器记录的系统运行时间
            _current_point = cp;
            // 更新timer的运行时间
            _current += diff;

            // 更新定时器的时间(time的值)，并执行对应的过期任务
            for (int i = 0; i < diff; i++)
            {
                // 每执行一次timer_update，其内部的函数
                // timer_shift: time+1，time代表从timer启动后至今一共经历了多少次tick
                // timer_execute: 执行near中的定时器
                timer_update();
            }
        }
    }

    void TimerWheelManager::move_list(int i, int idx)
    {
        for (auto &task : _t[i][idx])
        {
            add_node(task);
        }
        _t[i][idx].clear();
    }
    void TimerWheelManager::timer_execute(std::vector<std::function<void()>> &tasks)
    {
        // 取出time低8位对应的值
        int idx = _time & TIME_NEAR_MASK;

        // 如果低8位值对应的near数组元素有链表，则取出
        auto iter = _near[idx].begin();
        while (iter != _near[idx].end())
        {
            // 取出对应的定时器回调
            if (!(*iter)->_cancel)
            {
                std::function<void()> func = (*iter)->_callback;
                tasks.push_back(func);
                // 循环定时器
                if ((*iter)->_recurring)
                {
                    TimerW::ptr recu_timer = Xten::protected_make_shared<TimerW>(_time + ((*iter)->_sub_time),
                                                                      (*iter)->_sub_time, func, true, this);
                    add_node(recu_timer);
                }
            }

            iter = _near[idx].erase(iter);
        }
    }
    void TimerWheelManager::timer_shift()
    {
        int mask = TIME_NEAR;
        // 时间片+1
        uint32_t ct = ++_time;
        // 时间片溢出，无符号整数，循环，time重置0
        if (ct == 0)
        {
            // 将对应的t[3][0]链表取出，重新移动到定时器中
            move_list(3, 0);
        }
        else
        {
            // ct右移8位，进入到从动轮
            uint32_t time = ct >> TIME_NEAR_SHIFT;
            // 第 i 层时间轮
            int i = 0;
            // 判断是否需要重新映射？
            // 即循环判断当前层级对应的数位是否全0，即溢出产生进位
            while ((ct & (mask - 1)) == 0)
            {
                // 取当前层级的索引值
                int idx = time & TIME_LEVEL_MASK;
                // idx=0 说明当前层级溢出，继续循环判断直至当前层级不溢出
                if (idx != 0)
                {
                    // 将对应的t[i][idx]链表取出，依次移动到定时器中
                    move_list(i, idx);
                    break;
                }
                // time比mask领先一个时间轮级别
                mask <<= TIME_LEVEL_SHIFT; // mask 左移
                time >>= TIME_LEVEL_SHIFT; // time 右移
                ++i;                       // 时间轮层级增加
            }
        }
    }
    void TimerWheelManager::timer_update()
    {
        std::vector<std::function<void()>> _tasks;
        {
            // 锁
            SpinLock::Lock lock(_mutex);
            //  执行任务
            timer_execute(_tasks);
            /// time+1，并判断是否进行重新映射
            timer_shift();
            // 若发生重新映射，若time的指向有任务，则需要执行
            timer_execute(_tasks);
        }
        for (auto &func : _tasks)
        {
            func(); // 执行定时器对应的任务
        }
    }
    void TimerWheelManager::add_node(TimerW::ptr timer)
    {
        uint32_t time = timer->_expire;      // 过期时间
        uint32_t current_time = _time;       // 当前时间
        uint32_t msec = time - current_time; // 剩余时间

        // 根据 expire-time 的差值将结点放入相应的层级-----间隔时间确定任务所属时间轮层级
        //[0, 2^8)
        if (msec < TIME_NEAR)
        {
            _near[time & TIME_NEAR_MASK].push_back(timer);
        }
        //[2^8, 2^14)
        else if (msec < (1 << (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT)))
        {
            // 绝对时间确定任务所属该时间轮层级的那个槽位
            _t[0][((time >> TIME_NEAR_SHIFT) & TIME_LEVEL_MASK)].push_back(timer);
        }
        //[2^14, 2^20)
        else if (msec < (1 << (TIME_NEAR_SHIFT + 2 * TIME_LEVEL_SHIFT)))
        {
            _t[1][((time >> (TIME_NEAR_SHIFT + TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)].push_back(timer);
        }
        //[2^20, 2^26)
        else if (msec < (1 << (TIME_NEAR_SHIFT + 3 * TIME_LEVEL_SHIFT)))
        {
            _t[2][((time >> (TIME_NEAR_SHIFT + 2 * TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)].push_back(timer);
        }
        //[2^26, 2^32)
        else
        {
            _t[3][((time >> (TIME_NEAR_SHIFT + 3 * TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)].push_back(timer);
        }
    }
}