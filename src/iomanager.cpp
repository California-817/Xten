#include "iomanager.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "macro.h"
namespace Xten
{
    static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");

    enum EpollCtlOp
    {
    };
    static std::ostream &operator<<(std::ostream &os, const EpollCtlOp &op)
    {
        switch ((int)op)
        {
#define XX(ctl) \
    case ctl:   \
        return os << #ctl;
            XX(EPOLL_CTL_ADD);
            XX(EPOLL_CTL_MOD);
            XX(EPOLL_CTL_DEL);
        default:
            return os << (int)op;
        }
#undef XX
    }

    static std::ostream &operator<<(std::ostream &os, EPOLL_EVENTS events)
    {
        if (!events)
        {
            return os << "0";
        }
        bool first = true;
#define XX(E)          \
    if (events & E)    \
    {                  \
        if (!first)    \
        {              \
            os << "|"; \
        }              \
        os << #E;      \
        first = false; \
    }
        XX(EPOLLIN);
        XX(EPOLLPRI);
        XX(EPOLLOUT);
        XX(EPOLLRDNORM);
        XX(EPOLLRDBAND);
        XX(EPOLLWRNORM);
        XX(EPOLLWRBAND);
        XX(EPOLLMSG);
        XX(EPOLLERR);
        XX(EPOLLHUP);
        XX(EPOLLRDHUP);
        XX(EPOLLONESHOT);
        XX(EPOLLET);
#undef XX
        return os;
    }

    // 获取事件对应上下文
    IOManager::FdContext::EventContext &IOManager::FdContext::getEvContext(IOManager::Event ev)
    {
        switch (ev)
        {
        case IOManager::Event::READ:
            return read;
        case IOManager::Event::WRITE:
            return write;
        default:
            XTEN_ASSERT(false);
        }
        return read;
    }
    // 重置事件上下文
    void IOManager::FdContext::resetEvContext(EventContext &evctx)
    {
        evctx.cb = nullptr;
        evctx.fiber.reset();
        evctx.scheduler = nullptr;
    }
    // 触发事件上下文
    void IOManager::FdContext::triggerEvent(IOManager::Event ev)
    {
        if (XTEN_UNLIKELY(!(events & ev)))
        {
            XTEN_LOG_ERROR(g_logger) << "fd=" << fd
                                     << " triggerEvent event=" << ev
                                     << " events=" << events
                                     << "\nbacktrace:\n"
                                     << Xten::BackTraceUtil::backtraceTostring(100);
            return;
        }
        IOManager::FdContext::EventContext &evctx = getEvContext(ev);
        Scheduler *sche = evctx.scheduler;
        events = (Event)(events & ~ev); // 事件触发后取消该事件
        // 将事件的回调处理执行体放入调度队列中 由线程调度
        XTEN_ASSERTINFO(sche, "EventContext dont have Scheduler");
        if (evctx.cb)
        {
            sche->Schedule(std::move(evctx.cb));
        }
        if (evctx.fiber)
        {
            sche->Schedule(std::move(evctx.fiber));
        }
        evctx.scheduler = nullptr;
        XTEN_ASSERT(!evctx.fiber &&
                    !evctx.cb &&
                    !evctx.scheduler);
    }

    IOManager::IOManager(int threadNum, bool userCaller, const std::string &name)
        : Scheduler(threadNum, userCaller, name)
    {
        // 创建eventpoll结构
        _epfd = epoll_create(1);
        XTEN_ASSERT(_epfd >= 0);
        // 创建通知管道
        int ret = pipe(_pipeTicklefd);
        XTEN_ASSERT(!ret);
        // 将管道读事件设置非阻塞并放入epoll
        int flags = fcntl(_pipeTicklefd[0], F_GETFL);
        int ret2 = fcntl(_pipeTicklefd[0], F_SETFL, flags | O_NONBLOCK);
        XTEN_ASSERT(!ret2);
        struct epoll_event ev;
        bzero(&ev, sizeof ev);
        ev.events = EPOLLIN | EPOLLET;
        // 联合体的所有成员从相同的内存地址开始存储，但一次只能存储其中一个成员的值---联合体的大小由其最大成员的大小决定
        ev.data.fd = _pipeTicklefd[0]; // 参数传入fd---->data是一个联合体
        int ret3 = epoll_ctl(_epfd, EPOLL_CTL_ADD, _pipeTicklefd[0], &ev);
        XTEN_ASSERT(!ret3);
        // 初始化fdcontexts
        FdContextsResize(32);
        Scheduler::Start();
        //开启定时器 定期进行多任务队列的负载均衡
#if OPTIMIZE==ON
        // addTimer(150,std::bind(&Scheduler::autoLoadBalance,this),true);
#endif
    }
    IOManager::~IOManager()
    {
        // 调用调度器的stop函数
        Scheduler::Stop();
        close(_pipeTicklefd[0]);
        close(_pipeTicklefd[1]);
        close(_epfd);
        for (int i = 0; i < _fdContexts.size(); i++)
        {
            if (_fdContexts[i])
            {
                delete _fdContexts[i];
            }
        }
    }

    // 添加事件
    int IOManager::AddEvent(int fd, Event ev, std::function<void()> func)
    {
        FdContext *fd_ctx = nullptr;
        // 在这个范围访问共享的fds数组----加大粒度锁
        {
            RWMutex::ReadLock rlock(_mutex);
            // fd大于数组的最大下标
            if (fd > (_fdContexts.size() - 1))
            {
                rlock.unlock();
                RWMutex::WriteLock wlock(_mutex);
                // 扩容
                FdContextsResize(fd * 2);
                fd_ctx = _fdContexts[fd];
            }
            else
            {
                fd_ctx = _fdContexts[fd];
                rlock.unlock();
            }
        }
        // 在这个范围拿到了fd上下文结构----加每个fd上下文结构的内部锁（小粒度）
        {
            SpinLock::Lock lock(fd_ctx->mutex);
            if (XTEN_UNLIKELY(ev & fd_ctx->events))
            {
                // 已经有这个事件了
                XTEN_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                         << " event=" << (EPOLL_EVENTS)ev
                                         << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
                XTEN_ASSERT(false);
            }
            // 1.对epoll中进行事件的设置
            //  操作类型
            int opt = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
            struct epoll_event st_ev;
            // 设置ET触发----减少多线程epoll_wait一个eventpoll时产生惊群现象对性能的影响
            st_ev.events = EPOLLET | fd_ctx->events | ev;
            st_ev.data.ptr = (void *)fd_ctx; // 参数设置成fd_ctx指针 便于触发事件时进行处理事件
            int rt = epoll_ctl(_epfd, opt, fd_ctx->fd, &st_ev);
            if (XTEN_UNLIKELY(rt))
            {
                XTEN_LOG_ERROR(g_logger) << "epoll_ctl(" << _epfd << ", "
                                         << (EpollCtlOp)opt << ", " << fd << ", " << (EPOLL_EVENTS)st_ev.events << "):"
                                         << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                         << (EPOLL_EVENTS)fd_ctx->events;
                return -1;
            }
            // 待处理事件++
            _pendingEventNum++;

            // 2.对fdContext上下文结构进行设置
            fd_ctx->events = (Event)(fd_ctx->events | ev);
            FdContext::EventContext &evctx = fd_ctx->getEvContext(ev);
            XTEN_ASSERT(!evctx.fiber ||
                        !evctx.cb ||
                        !evctx.scheduler);
            // 线程当前调度器（也许是nullptr）
            evctx.scheduler = Scheduler::GetThis();
            if (func) // 传入回调
            {
                evctx.cb.swap(func);
            }
            else
            { // 没有传入回调---执行当前协程
                evctx.fiber = Xten::Fiber::GetThis();
                XTEN_ASSERTINFO(evctx.fiber->GetStatus() == Fiber::Status::EXEC, "fiber state != EXEC");
            }
        }
        return 0;
    }
    // 删除事件
    bool IOManager::DelEvent(int fd, Event ev)
    {
        FdContext *fd_ctx = nullptr;
        {
            RWMutex::ReadLock rlock(_mutex);
            if (XTEN_UNLIKELY(fd > _fdContexts.size() - 1))
            {
                XTEN_LOG_ERROR(g_logger) << "delEvent assert fd=" << fd
                                         << " over range event=" << (EPOLL_EVENTS)ev;
                return false;
            }
            fd_ctx = _fdContexts[fd];
        }
        {
            SpinLock::Lock lock(fd_ctx->mutex);
            if (XTEN_UNLIKELY(!(fd_ctx->events & ev)))
            {
                return false;
            }
            // 1.epoll中删除事件
            Event new_events = (Event)(fd_ctx->events & (~ev));
            struct epoll_event epev;
            epev.data.ptr = (void *)fd_ctx;
            epev.events = new_events | EPOLLET;
            int opt = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            int ret = epoll_ctl(_epfd, opt, fd_ctx->fd, &epev);
            if (XTEN_UNLIKELY(ret))
            {
                XTEN_LOG_ERROR(g_logger) << "epoll_ctl(" << _epfd << ", "
                                         << (EpollCtlOp)opt << ", " << fd << ", " << (EPOLL_EVENTS)epev.events << "):"
                                         << ret << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                         << (EPOLL_EVENTS)fd_ctx->events;
                return false;
            }
            // 2.fdcontexts中删除事件
            fd_ctx->events = new_events;
            FdContext::EventContext &evctx = fd_ctx->getEvContext(ev);
            fd_ctx->resetEvContext(evctx);
            _pendingEventNum--;
        }
        return true;
    }
    // 取消事件
    bool IOManager::CancelEvent(int fd, Event ev)
    {
        FdContext *fd_ctx = nullptr;
        {
            RWMutex::ReadLock rlock(_mutex);
            if (XTEN_UNLIKELY(fd >= _fdContexts.size()))
            {
                XTEN_LOG_ERROR(g_logger) << "cancel event assert fd=" << fd
                                         << " over range event=" << (EPOLL_EVENTS)ev;
                return false;
            }
            fd_ctx = _fdContexts[fd];
        }
        {
            // 1.epoll中取消
            SpinLock::Lock lock(fd_ctx->mutex);
            if (XTEN_UNLIKELY(!(fd_ctx->events & ev)))
            {
                return false;
            }
            Event new_event = (Event)(fd_ctx->events & ~ev);
            int opt = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            struct epoll_event epev;
            epev.data.ptr = (void *)fd_ctx;
            epev.events = EPOLLET | new_event;
            int ret = epoll_ctl(_epfd, opt, fd_ctx->fd, &epev);
            if (XTEN_UNLIKELY(ret))
            {
                XTEN_LOG_ERROR(g_logger) << "epoll_ctl(" << _epfd << ", "
                                         << (EpollCtlOp)opt << ", " << fd << ", " << (EPOLL_EVENTS)epev.events << "):"
                                         << ret << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                         << (EPOLL_EVENTS)fd_ctx->events;
                return false;
            }
            // 2.fdcontext直接触发这个事件
            fd_ctx->triggerEvent(ev);
            _pendingEventNum--;
        }
        return true;
    }
    // 取消fd上所有事件
    bool IOManager::CancelAll(int fd)
    {
        FdContext *fd_ctx = nullptr;
        {
            RWMutex::ReadLock rlock(_mutex);
            if (XTEN_UNLIKELY(fd >= _fdContexts.size()))
            {
                XTEN_LOG_DEBUG(g_logger) << "cancel all event assert fd=" << fd<<" size="<<_fdContexts.size();
                return true;
            }
            fd_ctx = _fdContexts[fd];
        }
        {
            SpinLock::Lock lock(fd_ctx->mutex);
            if (!fd_ctx->events)
            {
                return false;
            }
            int opt = EPOLL_CTL_DEL;
            struct epoll_event epev;
            epev.data.ptr = (void *)fd_ctx;
            epev.events = 0;
            int ret = epoll_ctl(_epfd, opt, fd_ctx->fd, &epev);
            if (XTEN_UNLIKELY(ret))
            {
                XTEN_LOG_ERROR(g_logger) << "epoll_ctl(" << _epfd << ", "
                                         << (EpollCtlOp)opt << ", " << fd << ", " << (EPOLL_EVENTS)epev.events << "):"
                                         << ret << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                         << (EPOLL_EVENTS)fd_ctx->events;
                return false;
            }
            // 有读事件
            if (fd_ctx->events & Event::READ)
            {
                fd_ctx->triggerEvent(Event::READ);
                _pendingEventNum--;
            }
            // 写事件
            if (fd_ctx->events & Event::WRITE)
            {
                fd_ctx->triggerEvent(Event::WRITE);
                _pendingEventNum--;
            }
            XTEN_ASSERT(!fd_ctx->events);
        }
        return true;
    }
    // 获取当前调度器指针
    IOManager *IOManager::GetThis() // 重定义
    {
        return dynamic_cast<IOManager *>(Scheduler::GetThis());
    }
    // 通知线程有任务
    void IOManager::Tickle() // override
    {
        // 无空闲线程--无需通知
        if (!Scheduler::HasIdleThread())
        {
            return;
        }
        int ret = write(_pipeTicklefd[1], "T", 1);
        if (XTEN_UNLIKELY(ret != 1))
        {
            XTEN_ASSERTINFO(false, "Tickle failed");
        }
    }
    // 返回是否可以终止
    bool IOManager::IsStopping() // override
    {
        uint64_t timeout = 0;
        return IsStopping(timeout);
    }
    // 线程无任务执行idle空闲协程 ----基于epoll_wait封装idle协程
    void IOManager::Idle() // override
    {
        XTEN_LOG_DEBUG(g_logger) << "idle";
        int MAX_EVENT = 256; // 一次epoll_wait返回的最大事件数
        struct epoll_event *epevs = new epoll_event[MAX_EVENT];
        std::shared_ptr<epoll_event> shared_epevs = std::shared_ptr<epoll_event>(epevs, [](epoll_event *ptr)
                                                                                 { delete[] ptr; });
        while (true)
        {
            uint64_t timeout = 0;
            // 判断idle协程是否满足终止条件---满足则idle协程退出循环
            if (XTEN_UNLIKELY(IsStopping(timeout)))
            {
                break;
            }
            // 拿到定时器中最早过期时间
            int ret = 0;
            do
            {
                static const int MAX_TIMEOUT = 3000; // 3s
                uint64_t next_time = 0;
                // 超时时间由定时器最早过期时间和MAX_TIMEOUT的较小值决定
                if (timeout != ~0ull)
                {
                    next_time = timeout < MAX_TIMEOUT ? timeout : MAX_TIMEOUT;
                }
                else
                {
                    next_time = MAX_TIMEOUT;
                }
                // 多线程epoll_wait该epfd ------可能产生惊群现象
                ret = epoll_wait(_epfd, epevs, MAX_EVENT, (int)(next_time));
                if (ret == -1 && errno == EINTR)
                { // epoll被信号中断返回
                    continue;
                }
                else
                { // epoll超时返回 有事件就绪
                    break;
                }
            } while (true);
            // 1.处理超时事件------多线程安全
            std::vector<std::function<void()>> expire_funcS;
            TimerManager::listExpiredCb(expire_funcS);
            if (!expire_funcS.empty())
            {
                Schedule(expire_funcS.begin(), expire_funcS.end());
                expire_funcS.clear();
            }
            // 2.处理就绪事件
            for (int i = 0; i < ret; i++)
            {
                struct epoll_event epev = epevs[i];
                // 2.1.tickle通知事件------不做多线程同步机制
                if (epev.data.fd == _pipeTicklefd[0])
                {
                    char dummy[256];
                    while (read(_pipeTicklefd[0], dummy, sizeof dummy) > 0)
                    {
                        continue;
                    }
                    // 是tickle事件处理完后再处理下一个fd
                    continue;
                }
                // 2.2.处理注册的io事件-------多线程安全
                FdContext *fd_ctx = (FdContext *)epev.data.ptr; // 如果tickle事件走到这 由于data是联合体 导致ptr得出来是0x04-----段错误
                {
                    // 同一个事件只能同时被一个线程处理
                    SpinLock::Lock lock(fd_ctx->mutex);
                    if (epev.events & (EPOLLERR | EPOLLHUP)) // 异常事件转化成读写事件进行处理
                    {
                        epev.events |= ((EPOLLIN | EPOLLOUT) & fd_ctx->events);
                    }
                    // 获取触发的事件
                    int real_event = Event::NONE;
                    if (epev.events & EPOLLIN)
                    {
                        real_event |= Event::READ;
                    }
                    if (epev.events & EPOLLOUT)
                    {
                        real_event |= Event::WRITE;
                    }
                    // 这个判断是I/O事件分发机制中非常重要的一环--->[确保只有真正被监听的事件就绪后才会被触发执行]
                    if ((real_event & fd_ctx->events) == Event::NONE)
                    {
                        continue;
                    }
                    // 获取fd_ctx中还没触发的事件
                    int leave_event = (fd_ctx->events & ~real_event);
                    int opt = leave_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                    // 重新设置epoll
                    epev.events = EPOLLET | leave_event;
                    int ret2 = epoll_ctl(_epfd, opt, fd_ctx->fd, &epev);
                    if (XTEN_UNLIKELY(ret2))
                    {
                        XTEN_LOG_ERROR(g_logger) << "epoll_ctl(" << _epfd << ", "
                                                 << (EpollCtlOp)opt << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)epev.events << "):"
                                                 << ret2 << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                                                 << (EPOLL_EVENTS)fd_ctx->events;
                        continue;
                    }
                    // 处理就绪事件----将就绪事件设置的执行体放入调度队列
                    if (real_event & Event::READ)
                    {
                        // 读事件触发
                        fd_ctx->triggerEvent(Event::READ);
                        _pendingEventNum--;
                    }
                    if (real_event & Event::WRITE)
                    {
                        // 写事件触发
                        fd_ctx->triggerEvent(Event::WRITE);
                        _pendingEventNum--;
                    }
                }
            }
            // 一次idle协程从epoll_wait唤醒并处理完事件---切回调度协程
            Fiber::ptr cur = Fiber::GetThis();
            Fiber *ptr = cur.get();
            cur.reset();
            ptr->SwapOut();
        }
        // while(true)循环退出---->调度器的终止条件就绪了
    }
    // 有更早过期任务
    void IOManager::onTimerInsertedAtFront() // override
    {
        Tickle();
    }
    //_fdContexts扩容
    void IOManager::FdContextsResize(int size)
    {
        _fdContexts.resize(size);
        for (int i = 0; i < _fdContexts.size(); i++)
        {
            if (_fdContexts[i] == nullptr)
            {
                _fdContexts[i] = new FdContext();
                _fdContexts[i]->fd = i; // 下标值就是fd
            }
        }
    }
    bool IOManager::IsStopping(uint64_t &timeout)
    {
        timeout = TimerManager::getNextTimer();
        return timeout == ~0ull &&
               _pendingEventNum == 0 &&
               Scheduler::IsStopping();
    }

    Timer::ptr AddTimerToIOManager(uint64_t ms, std::function<void()> cb, bool recurring = false)
    {
        return Xten::IOManager::GetThis()->addTimer(ms, cb, recurring);
    }
}