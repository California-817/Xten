#ifndef __XTEN_TIMER_H__
#define __XTEN_TIMER_H__
#include <vector>
#include <list>
#include <functional>
#include <memory>
#include "mutex.h"
#include <queue>
namespace Xten
{
class TimerManager;
    //定时器任务
class Timer : public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    /// 定时器的智能指针类型
    typedef std::shared_ptr<Timer> ptr;
    //取消定时器
    bool cancel();

    //刷新设置定时器的执行时间
    bool refresh();

    // 重置定时器时间
    bool reset(uint64_t ms, bool from_now);
protected:
    Timer(uint64_t ms, std::function<void()> cb,
          bool recurring, TimerManager* manager);

    Timer(uint64_t next);
private:
    /// 是否循环定时器
    bool m_recurring = false;
    /// 执行周期
    uint64_t m_ms = 0;
    /// 精确的执行时间
    uint64_t m_next = 0;
    /// 回调函数
    std::function<void()> m_cb;
    /// 定时器管理器
    TimerManager* m_manager = nullptr;
private:

    // 定时器比较仿函数
    struct Comparator {

        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
};

//定时器管理器
class TimerManager {
friend class Timer;
public:
    /// 读写锁类型
    typedef RWMutex RWMutexType;

    TimerManager();

    virtual ~TimerManager();
    //添加定时器
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                        ,bool recurring = false);

    // 添加条件定时器
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                        ,std::weak_ptr<void> weak_cond
                        ,bool recurring = false);

    // 到最近一个定时器执行的时间间隔(毫秒)
    uint64_t getNextTimer();

    //获取需要执行的定时器的回调函数列表
    void listExpiredCb(std::vector<std::function<void()> >& cbs);

    // 是否有定时器
    bool hasTimer();
protected:

    // 当有新的定时器插入到定时器的首部,执行该函数
    virtual void onTimerInsertedAtFront() = 0;

    //将定时器添加到管理器中
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
private:
    //检测服务器时间是否被调后
    bool detectClockRollover(uint64_t now_ms);
private:
    /// Mutex
    RWMutexType m_mutex;
    /// 定时器集合
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    /// 是否触发onTimerInsertedAtFront
    bool m_tickled = false;
    /// 上次执行时间
    uint64_t m_previouseTime = 0;
};
}
#endif