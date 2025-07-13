#ifndef __XTEN_TIMER_H__
#define __XTEN_TIMER_H__
#include <vector>
#include <list>
#include <functional>
#include <memory>
#include"thread.h"
#include "mutex.h"
#include <queue>

// tick时钟指针uint32_t -----每部分字段代表该层时间轮的时钟指针索引
//   t[3]  t[2]  t[1]  t[0]  near
//  +-----+-----+----+-----+-----+
//  | 6位 | 6位 | 6位 | 6位 | 8位 |
//  +-----+-----+----+----+------+
#define TIME_NEAR_SHIFT 8 // 主时间轮槽位是256个----对应uint32_t的低八位
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6 //从时间轮槽数 64个
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR - 1)
#define TIME_LEVEL_MASK (TIME_LEVEL - 1)

namespace Xten
{
    class TimerManager;
    // 基于红黑树的定时器任务
    class Timer : public std::enable_shared_from_this<Timer>
    {
        friend class TimerManager;

    public:
        /// 定时器的智能指针类型
        typedef std::shared_ptr<Timer> ptr;
        // 取消定时器
        bool cancel();

        // 刷新设置定时器的执行时间
        bool refresh();

        // 重置定时器时间
        bool reset(uint64_t ms, bool from_now);

    protected:
        Timer(uint64_t ms, std::function<void()> cb,
              bool recurring, TimerManager *manager);

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
        TimerManager *m_manager = nullptr;

    private:
        // 定时器比较仿函数
        struct Comparator
        {

            bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
        };
    };

    // 基于红黑树的定时器管理器
    class TimerManager
    {
        friend class Timer;

    public:
        /// 读写锁类型
        typedef RWMutex RWMutexType;

        TimerManager();

        virtual ~TimerManager();
        // 添加定时器
        Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

        // 添加条件定时器
        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false);

        // 到最近一个定时器执行的时间间隔(毫秒)
        uint64_t getNextTimer();

        // 获取需要执行的定时器的回调函数列表
        void listExpiredCb(std::vector<std::function<void()>> &cbs);

        // 是否有定时器
        bool hasTimer();

    protected:
        // 当有新的定时器插入到定时器的首部,执行该函数
        virtual void onTimerInsertedAtFront() = 0;

        // 将定时器添加到管理器中
        void addTimer(Timer::ptr val, RWMutexType::WriteLock &lock);

    private:
        // 检测服务器时间是否被调后
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

    class TimerWheelManager;
    // 基于时间轮实现的定时器任务
    class TimerW
    {
    protected:
        //防止随意构造定时器任务
        TimerW(int expire, int sub, std::function<void()> callback, bool recurring, TimerWheelManager *manager);
    public:
        friend class TimerWheelManager;
        using ptr = std::shared_ptr<TimerW>;
        ~TimerW() = default;
        //取消定时器
        void Cancel();
        // 刷新定时器
        void Refresh();

    private:
        bool _recurring;                   // 是否是循环定时器
        uint32_t _expire;                  // 任务过期时间
        std::function<void()> _callback;   // 任务回调函数
        std::atomic<bool> _cancel = false; // 删除任务，遇到该标记则取消任务的执行;
        uint32_t _sub_time;                // 间隔过期时间
        TimerWheelManager *_manager;
    };
    class IOManager;
    /// @brief 时间轮定时器任务管理方案
    class TimerWheelManager
    {
    public:
        typedef std::shared_ptr<TimerWheelManager> ptr;
        TimerWheelManager(IOManager* iom=nullptr);
        virtual ~TimerWheelManager() ;
        // 添加定时器
        TimerW::ptr AddTimer(int time_ms, std::function<void()> func, bool recurring = false);
        // 添加条件定时器
        void AddConditionTimer(int time_ms, std::function<void()> func, std::weak_ptr<void> cond, bool recurring = false);
        // 删除定时器
        void DelTimer(TimerW::ptr timer);
        // 清理所有定时器
        void ClearTimer();
        //暂停定时器处理线程
        void StopTimer();
        // 处理过期定时器
        void ExpireTimer();

    private:
        // 移动链表节点到主时间轮
        void move_list(int i, int idx);
        //将过期任务回调存入vector统一处理
        void timer_execute(std::vector<std::function<void()>> &tasks);
        //时间指针+1 并判断是否需要下沉过期任务
        void timer_shift();
        //处理过期定时器
        void timer_update();
        //无锁向时间轮添加定时器
        void add_node(TimerW::ptr timer);

    private:
        std::vector<std::list<TimerW::ptr>> _near;           // 最低级的时间轮，主动轮
        std::vector<std::vector<std::list<TimerW::ptr>>> _t; // 其他层级的时间轮，从动轮
        Xten::SpinLock _mutex;                               // 自旋锁，O(1)
        uint32_t _time;                                      // 32为 tick 指针，当前时间片
        uint64_t _current;                                   // timer运行时间，精度10ms
        uint64_t _current_point;                             // 系统运行时间，精度10ms
        Xten::Thread _timerThread;                           //专门处理定时任务的定时器线程
        std::atomic<bool> _b_stop;                          //是否终止定时器
        IOManager* _iom;                                     //执行定时器任务的io调度器
    };
}

#endif