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
    IOManagerRB::FdContext::EventContext &IOManagerRB::FdContext::getEvContext(IOManagerRB::Event ev)
    {
    }
    // 重置事件上下文
    void IOManagerRB::FdContext::resetEvContext(EventContext &evctx)
    {
    }
    // 触发事件上下文
    void IOManagerRB::FdContext::triggerEvent(IOManagerRB::Event ev)
    {
    }

    IOManagerRB::IOManagerRB(int threadNum = 1, bool userCaller = true, const std::string &name = "")
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
        ev.data.fd = _pipeTicklefd[0];
        int ret3 = epoll_ctl(_epfd, EPOLL_CTL_ADD, _pipeTicklefd[0], &ev);
        XTEN_ASSERT(!ret3);
        // 初始化fdcontexts
        FdContextsResize(32);
        Scheduler::Start();
    }
    IOManagerRB::~IOManagerRB()
    {
    }

    // 添加事件
    int IOManagerRB::AddEvent(int fd, Event ev, std::function<void()> func)
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
                FdContextsResize(fd * 1.5);
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
    bool IOManagerRB::DelEvent(int fd, Event ev)
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
            epev.data.ptr = fd_ctx;
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
            return true;
        }
    }
    // 取消事件
    bool IOManagerRB::CancelEvent(int fd, Event ev)
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
            if (XTEN_UNLIKELY(!(fd_ctx->events & ev)))
            {
                return false;
            }
            Event new_event = (Event)(fd_ctx->events & ~ev);
            int opt = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            struct epoll_event epev;
            epev.data.ptr = fd_ctx;
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
    }
    // 取消fd上所有事件
    bool IOManagerRB::CancelAll(int fd)
    {
        FdContext *fd_ctx = nullptr;
        {
            RWMutex::ReadLock rlock(_mutex);
            if (XTEN_UNLIKELY(fd >= _fdContexts.size()))
            {
                XTEN_LOG_ERROR(g_logger) << "cancel all event assert fd=" << fd;
                return false;
            }
            fd_ctx = _fdContexts[fd];
        }
        {
            SpinLock::Lock lock(fd_ctx->mutex);
            int opt = EPOLL_CTL_DEL;
            struct epoll_event epev;
            epev.data.ptr = fd_ctx;
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
            return true;
        }
    }
    // 获取当前调度器指针
    IOManagerRB *IOManagerRB::GetThis()
    {
    }
    // 通知线程有任务
    void IOManagerRB::Tickle()
    {
    }
    // 返回是否可以终止
    bool IOManagerRB::IsStopping()
    {
    }
    // 线程无任务执行idle空闲协程
    void IOManagerRB::Idle()
    {
    }
    // 有更早过期任务
    void IOManagerRB::onTimerInsertedAtFront()
    {
    }
    //_fdContexts扩容
    void IOManagerRB::FdContextsResize(int size)
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
    bool IOManagerRB::IsStopping(uint64_t timeout)
    {
    }
}