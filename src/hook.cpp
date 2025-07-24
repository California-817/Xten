#include "hook.h"
#include "log.h"
#include "config.h"
#include "iomanager.h"
#include "fdmanager.h"
#include "timer.h"
#include <dlfcn.h>
#include <time.h>
#include <sys/types.h>
 #include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(usleep)       \
    XX(nanosleep)    \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl)        \
    XX(ioctl)        \
    XX(getsockopt)   \
    XX(setsockopt)
static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");
namespace Xten
{

    /// @brief // 默认是非hook --实现线程级别hook
    static thread_local bool t_hookable = false;
    /// @brief connect超时参数
    static ConfigVar<uint64_t>::ptr g_tcp_connect_timeout =
        Config::LookUp<uint64_t>("tcp.connect.timeout", (uint64_t)5000, "tcp connect timeout");
    static uint64_t s_tcp_connect_timeout = -1;

    /// @brief 获取原始接口函数并赋值给xxx_f 供外部使用原始接口
    static bool hook_init()
    {
        static bool is_init = false;
        if (is_init)
        {
            return true;
        }
        // 去libc库中寻找原始的系统调用接口实现赋值给 XXX_f
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX);
#undef XX
        is_init = true;
        return true;
    }
    struct __Hook_Init
    {
        __Hook_Init()
        {
            hook_init();
            s_tcp_connect_timeout = g_tcp_connect_timeout->GetValue();
            g_tcp_connect_timeout->AddListener([](const uint64_t &old, const uint64_t &new_value)
                                               {
                                XTEN_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                         << old << " to " << new_value;
                s_tcp_connect_timeout = new_value; });
        }
    };

    static __Hook_Init __hookinit; //在main函数之前初始化原始接口
    bool is_hook_enable()
    {
        return t_hookable;
    }
    void set_hook_enable(bool ishook)
    {
        t_hookable = ishook;
    }
}

/// @brief 条件定时器的执行条件
struct timer_condition
{
    int cancelled = 0;
};
/// @brief socket读写函数的模板hook函数
template <class OriginFun, class... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_name, uint32_t event, int timeout_type, Args &&...args)
{
    // 1.未设置hook属性
    if (!Xten::is_hook_enable())
    {
        return fun(fd, std::forward<Args>(args)...);
    }
    // 2.设置了 先看fdMgr
    Xten::FdCtx::ptr fdctx = Xten::FdCtxMgr::GetInstance()->Get(fd, false);
    // 框架层未管理该fdctx
    if (!fdctx)
    {
        return fun(fd, std::forward<Args>(args)...);
    }
    // fd关闭
    if (fdctx->IsClose())
    {
        errno = EBADF;
        return -1;
    }
    // 不是socket读写 或者 用户设置了非阻塞
    if (!fdctx->IsSocket() || fdctx->GetUserNoBlock())
    {
        return fun(fd, std::forward<Args>(args)...);
    }
    // 是socket且是阻塞io
    uint64_t timeout = fdctx->GetTimeOut(timeout_type);
    std::shared_ptr<timer_condition> timecond = std::make_shared<timer_condition>();
retry: // 再次回来重试
    // 尝试读写
    ssize_t ret = fun(fd, std::forward<Args>(args)...);
    // 读写被信号中断
    while (ret == -1 && errno == EINTR)
    {
        ret = fun(fd, std::forward<Args>(args)...);
    }
    // 读写条件不就绪 ---给上层的感觉是阻塞 内部hook实现切换回调度协程 再由调度协程切换到其他工作协程或者idle协程
    if (ret == -1 && errno == EAGAIN)
    {
        std::weak_ptr<timer_condition> wkcond(timecond);
        Xten::IOManager *iom = Xten::IOManager::GetThis();
        Xten::Timer::ptr timer;
        if (timeout != (uint64_t)-1)
        {
            // 有超时时间--添加条件定时器
            timer = iom->addConditionTimer(timeout, [wkcond, event, iom, fd]()
                                           {
                std::shared_ptr<timer_condition> cond=wkcond.lock();
                //已經取消或者條件不存在(大概率不存在这种情况)
                if(!cond || cond->cancelled)
                {
                    return;
                }
                //條件依然存在--超时取消事件
                cond->cancelled=ETIMEDOUT;
                iom->CancelEvent(fd,(Xten::IOManager::Event)(event)); }, wkcond);
        }
        // 向io调度器添加io事件
        int rt = iom->AddEvent(fd, (Xten::IOManager::Event)(event)); // 不传入默认是当前协程
        // 添加事件失败
        if (XTEN_UNLIKELY(rt))
        {
            XTEN_LOG_ERROR(g_logger) << hook_name << " addEvent("
                                     << fd << ", " << event << ")";
            if (timer)
            {
                timer->cancel();
            }
            return -1;
        }
        // 添加成功 --挂起当前协程
        Xten::Fiber::YieldToHold();
        // 由于事件就绪 协程重新放回调度队列 再次由线程切入
        if (timer)
        {
            // 取消超时器
            timer->cancel();
        }
        if (timecond->cancelled)
        {
            // 超时返回
            errno = timecond->cancelled;
            return -1;
        }
        // 未超时返回---重新继续读取数据
        goto retry;
    }
    return ret;
}

extern "C"
{
#define XX(name) name##_fun name##_f = nullptr;
    HOOK_FUN(XX)
#undef XX
    // 定义一个与系统库函数同名的函数.在链接阶段.链接器会优先使用自定义的函数.而不是 libc 中的版本
    // sleep
    unsigned int sleep(unsigned int seconds)
    {
        if (!Xten::is_hook_enable())
        {
            return sleep_f(seconds);
        }
        Xten::IOManager *iom = Xten::IOManager::GetThis();
        Xten::Fiber::ptr fiber = Xten::Fiber::GetThis();
        iom->addTimer(seconds * 1000, [iom, fiber]()
                      { iom->Schedule(fiber, -1); });
        Xten::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec)
    {
        if (!Xten::is_hook_enable())
        {
            return usleep_f(usec);
        }
        Xten::IOManager *iom = Xten::IOManager::GetThis();
        Xten::Fiber::ptr fiber = Xten::Fiber::GetThis();
        iom->addTimer(usec / 1000, [iom, fiber]()
                      { iom->Schedule(fiber, -1); });
        Xten::Fiber::YieldToHold();
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        if (!Xten::is_hook_enable())
        {
            return nanosleep_f(req, rem);
        }
        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
        Xten::IOManager *iom = Xten::IOManager::GetThis();
        Xten::Fiber::ptr fiber = Xten::Fiber::GetThis();
        iom->addTimer(timeout_ms, [iom, fiber]()
                      { iom->Schedule(fiber, -1); });
        Xten::Fiber::YieldToHold();
        return 0;
    }
    // socket
    int socket(int domain, int type, int protocol)
    {
        if (!Xten::is_hook_enable())
        {
            return socket_f(domain, type, protocol);
        }
        int ret = socket_f(domain, type, protocol);
        if (ret >= 0)
        {
            Xten::FdCtxMgr::GetInstance()->Get(ret, true);
        }
        return ret;
    }
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    {
        int ret = do_io(sockfd, accept_f, "accept", Xten::IOManager::Event::READ, SO_RCVTIMEO, addr, addrlen);
        if (ret >= 0)
        {
            // 接收成功---在框架层面管理这个accept返回的socketfd
            Xten::FdCtxMgr::GetInstance()->Get(ret, true);
        }
        return ret;
    }
    int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms)
    {
        if (!Xten::is_hook_enable())
        {
            struct timeval tv{int(timeout_ms / 1000), int(timeout_ms % 1000 * 1000)};
            socklen_t len = sizeof tv;
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, len);
            return connect_f(fd, addr, addrlen);
        }
        Xten::FdCtx::ptr fdctx = Xten::FdCtxMgr::GetInstance()->Get(fd);
        if (!fdctx || fdctx->IsClose())
        {
            errno = EBADF;
            return -1;
        }
        if (!fdctx->IsSocket() || fdctx->GetUserNoBlock())
        {
            return connect_f(fd, addr, addrlen);
        }
        // 是socket且是阻塞connect
        // 尝试连接
        int ret = connect_f(fd, addr, addrlen);
        while (ret == -1 && errno == EINTR)
        {
            // 信号中断
            ret = connect_f(fd, addr, addrlen);
        }
        // 连接正在处理中
        if (ret == -1 && errno == EINPROGRESS)
        {
            Xten::Timer::ptr timer;
            Xten::IOManager *iom = Xten::IOManager::GetThis();
            std::shared_ptr<timer_condition> tmcond=std::make_shared<timer_condition>();
            std::weak_ptr<timer_condition> wkcond(tmcond);
            if (timeout_ms != (uint64_t)-1)
            {
                // 设置了超时时间
                timer = iom->addConditionTimer(timeout_ms, [wkcond, iom, fd]()
                                               {
                std::shared_ptr<timer_condition> cond=wkcond.lock();
                if(!cond || cond->cancelled)
                {
                    return;
                }
                //超时
                cond->cancelled=ETIMEDOUT;
                iom->CancelEvent(fd,Xten::IOManager::Event::WRITE); }, wkcond);
            }
            // 向io调度器中添加写事件
            int rt = iom->AddEvent(fd, Xten::IOManager::Event::WRITE);
            if (XTEN_UNLIKELY(rt))
            {
                XTEN_LOG_ERROR(g_logger) << "connect " << " addEvent("
                                         << fd << ", " << "WRITE" << ")";
                if (timer)
                {
                    timer->cancel();
                }
                return -1;
            }
            Xten::Fiber::YieldToHold();
            if (timer)
            {
                timer->cancel();
            }
            if (tmcond->cancelled)
            {
                // 超时
                errno = tmcond->cancelled;
                return -1;
            }
            // 未超时返回--条件就绪返回 (不需要返回再重新连接)
            int error = 0;
            socklen_t len = sizeof error;
            int ret2 = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
            if (ret2 == -1)
            {
                return -1;
            }
            if (!error)
            {
                // 没有错误
                return 0;
            }
            else
            {
                // 有错误
                errno = error;
                return -1;
            }
        }
        return ret;
    }
    // 在os层面 connect是不能设置超时参数的
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return connect_with_timeout(sockfd, addr, addrlen, Xten::s_tcp_connect_timeout);
    }
    // read
    ssize_t read(int fd, void *buf, size_t count)
    {
        return do_io(fd, read_f, "read", Xten::IOManager::Event::READ, SO_RCVTIMEO, buf, count);
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
    {
        return do_io(fd, readv_f, "readv", Xten::IOManager::Event::READ, SO_RCVTIMEO, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    {
        return do_io(sockfd, recv_f, "recv", Xten::IOManager::Event::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
    {
        return do_io(sockfd, recvfrom_f, "recvfrom", Xten::IOManager::Event::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
    {
        return do_io(sockfd, recvmsg_f, "recvmsg", Xten::IOManager::Event::READ, SO_RCVTIMEO, msg, flags);
    }

    // write
    ssize_t write(int fd, const void *buf, size_t count)
    {
        return do_io(fd, write_f, "write", Xten::IOManager::Event::WRITE, SO_SNDTIMEO, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
    {
        return do_io(fd, writev_f, "writev", Xten::IOManager::Event::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int s, const void *msg, size_t len, int flags)
    {
        return do_io(s, send_f, "send", Xten::IOManager::Event::WRITE, SO_SNDTIMEO, msg, len, flags);
    }

    ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
    {
        return do_io(s, sendto_f, "sendto", Xten::IOManager::Event::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
    }

    ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
    {
        return do_io(s, sendmsg_f, "sendmsg", Xten::IOManager::Event::WRITE, SO_SNDTIMEO, msg, flags);
    }
    int close(int fd)
    {
        if (!Xten::is_hook_enable())
        {
            return close_f(fd);
        }
        Xten::FdCtx::ptr fdctx = Xten::FdCtxMgr::GetInstance()->Get(fd);
        if (fdctx)
        {
            Xten::IOManager *iom = Xten::IOManager::GetThis();
            // 取消io调度器中事件
            if (iom)
            {
                iom->CancelAll(fd);
            }
            // 删除对fdctx的管理
            Xten::FdCtxMgr::GetInstance()->Del(fd);
        }
        return close_f(fd);
    }
    // socket属性相关
    int fcntl(int fd, int cmd, ... /* arg */)
    {
        va_list va;
        va_start(va, cmd);
        switch (cmd)
        {
        case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            Xten::FdCtx::ptr ctx = Xten::FdCtxMgr::GetInstance()->Get(fd);
            if (!ctx || ctx->IsClose() || !ctx->IsSocket())
            {
                return fcntl_f(fd, cmd, arg);
            }
            ctx->SetUserNoBlock(arg & O_NONBLOCK);
            if (ctx->GetSysNoBlock())
            {
                arg |= O_NONBLOCK;
            }
            else
            {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETFL:
        {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            Xten::FdCtx::ptr ctx = Xten::FdCtxMgr::GetInstance()->Get(fd);
            if (!ctx || ctx->IsClose() || !ctx->IsSocket())
            {
                return arg;
            }
            if (ctx->GetUserNoBlock())
            {
                return arg | O_NONBLOCK;
            }
            else
            {
                return arg & ~O_NONBLOCK;
            }
        }
        break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
        break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        {
            struct flock *arg = va_arg(va, struct flock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
        {
            struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
        }
    }

    int ioctl(int d, unsigned long int request, ...)
    {
        va_list va;
        va_start(va, request);
        void *arg = va_arg(va, void *);
        va_end(va);

        if (FIONBIO == request)
        {
            bool user_nonblock = !!*(int *)arg;
            Xten::FdCtx::ptr ctx = Xten::FdCtxMgr::GetInstance()->Get(d);
            if (!ctx || ctx->IsClose() || !ctx->IsSocket())
            {
                return ioctl_f(d, request, arg);
            }
            ctx->SetUserNoBlock(user_nonblock);
        }
        return ioctl_f(d, request, arg);
    }

    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
    {
        return getsockopt_f(sockfd, level, optname, optval, optlen);
    }
    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        if (!Xten::is_hook_enable())
        {
            return setsockopt_f(sockfd, level, optname, optval, optlen);
        }
        if (level == SOL_SOCKET)
        {
            if (optname == SO_SNDTIMEO || optname == SO_RCVTIMEO)
            {
                Xten::FdCtx::ptr fdctx = Xten::FdCtxMgr::GetInstance()->Get(sockfd);
                if (fdctx)
                {
                    // 在框架层面设置超时时间
                    struct timeval *tm = (struct timeval *)(optval);
                    fdctx->SetTimeOut(optname, tm->tv_sec * 1000 + tm->tv_usec / 1000);
                    return 0; // 框架层面设置超时时间直接返回
                }
            }
        }
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
}
