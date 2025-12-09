#ifndef __XTEN_ASYNC_SOCKET_H__
#define __XTEN_ASYNC_SOCKET_H__
#include "../iomanager.h"
#include "socket_stream.h"
#include "../timer.h"
#include <list>
#include <boost/any.hpp>
namespace Xten
{
    // AsyncSocketStream相比于普通SocketStream的优势
    // 1.异步非阻塞：避免单个请求阻塞整个线程
    // 2.并发处理：可以同时处理多个请求
    // 3.请求-响应匹配：通过序列号正确匹配异步响应
    // 4.连接管理：自动重连、请求超时处理
    class AsyncSocketStream : public SocketStream, public std::enable_shared_from_this<AsyncSocketStream>
    {
    public:
        typedef std::shared_ptr<AsyncSocketStream> ptr;
        typedef std::function<bool(AsyncSocketStream::ptr)> connect_cb;
        typedef std::function<bool(AsyncSocketStream::ptr)> disconnect_cb;
        AsyncSocketStream(Socket::ptr socket, bool is_owner = true, bool auto_connect = false);
        // 响应错误码
        enum ERROR
        {
            OK = 0,           // 成功
            TIMEOUT = -1,     // 超时
            IO_ERROR = -2,    // io错误
            NOT_CONNECT = -3, // 未连接
        };
        // 发送的基类上下文
        struct SendCtx
        {
            typedef std::shared_ptr<SendCtx> ptr;
            virtual ~SendCtx() = default;
            // doSend方法(真正的发送请求数据函数)
            virtual bool doSend(AsyncSocketStream::ptr stream) = 0;
        };
        // 一个发送请求上下文
        struct Ctx : public SendCtx
        {
            typedef std::shared_ptr<Ctx> ptr;
            Ctx()
                : sn(0),
                  timed(false),
                  timeout(0),
                  fiber(nullptr),
                  scheduler(nullptr),
                  timer(nullptr),
                  result(ERROR::OK)
            {
            }
            virtual ~Ctx() = default;
            // 一个请求上下文保存的信息

            uint32_t sn;      // 请求序列号
            bool timed;       // 请求是否超时
            uint32_t timeout; // 超时时间

            Fiber::ptr fiber;     // 发起请求的协程
            Scheduler *scheduler; // 调度器
            Timer::ptr timer;     // 超时定时器

            uint32_t result;              // 响应码
            std::string resultStr = "ok"; // 响应字符串
            // 请求收到响应后的rsp函数
            void doRsp()
            {
                Scheduler *sche = scheduler;
                if (!Atomic::compareAndSwapBool(scheduler, sche, (Scheduler *)nullptr))
                {
                    // 说明已经有一个协程进来了并修改scheduler=nullptr
                    return;
                }
                // 保证只有一个协程能执行下述操作(write函数和超时定时器函数都有可能执行此函数)
                if (!sche || !fiber)
                {
                    return;
                }
                // 如果没超时 取消超时定时器
                if (timer)
                {
                    timer->cancel();
                    timer = nullptr;
                }
                if (timed)
                {
                    // 超时了
                    result = ERROR::TIMEOUT;
                    resultStr = "timeout";
                }
                // 重新调度之前请求响应而挂起的协程
                sche->Schedule(fiber);
            }
        };
        // 启动异步socket流
        bool Start(AsyncSocketStream::ptr self = nullptr);
        // 关闭异步socket流
        virtual void Close() override;

    public:
        // 设置io调度器
        void SetIOWorker(IOManager *sche) { _ioWorker = sche; }
        // 设置工作调度器
        void SetProcessWorker(IOManager *sche) { _processWorker = sche; }
        // 获取io调度器
        IOManager *GetIOWorker() const { return _ioWorker; }
        // 获取工作调度器
        IOManager *GetProcessWorker() const { return _processWorker; }
        // 设置连接回调
        void SetConnectCb(connect_cb cb) { _connectCb = cb; }
        // 设置断连回调
        void SetDisConnectCb(disconnect_cb cb) { _disconnectCb = cb; }
        // 获取回调
        connect_cb GetConnectCb() const { return _connectCb; }
        disconnect_cb GetDisConnectCb() { return _disconnectCb; }
        // 放置T类型数据
        template <class T>
        void SetData(const T &data) { _data = data; }
        // 获取T类型数据
        template <class T>
        T GetData() const
        {
            try
            {
                return boost::any_cast<T>(_data);
            }
            catch (...)
            { // TODO LOG
            }
            return T();
        }

    protected:
        // 写协程函数
        virtual void doWrite(AsyncSocketStream::ptr self);
        // 读协程函数
        virtual void doRead(AsyncSocketStream::ptr self);
        // 开启读写协程
        void startRead();
        void startWrite();
        // 接受数据的函数
        virtual Ctx::ptr doRecv() = 0;
        // 请求超时函数
        void onTimer(AsyncSocketStream::ptr self, Ctx::ptr ctx);
        virtual void onClose() {}
        // 根据请求的sn获取请求上下文ctx
        Ctx::ptr getCtx(uint32_t sn);
        // 根据请求的sn获取并删除管理的请求上下文ctx
        Ctx::ptr getAndDelCtx(uint32_t sn);
        // 获取子类ctx
        template <class T>
        std::shared_ptr<T> getCtxAs(uint32_t sn)
        {
            return std::dynamic_pointer_cast<T>(getCtx(sn));
        }
        // 获取并删除子类ctx
        template <class T>
        std::shared_ptr<T> getAndDelCtxAs(uint32_t sn)
        {
            return std::dynamic_pointer_cast<T>(getAndDelCtx(sn));
        }
        // 添加对请求上下文ctx的管理 [ sn <---> ctx ]
        bool addCtx(Ctx::ptr ctx);
        // 将请求放入发送队列
        bool enqueue(SendCtx::ptr sendctx);
        // 等待读写协程退出
        void waitFiber();
        // 内部进行关闭socket流
        bool innerClose();

    protected:
        // 请求上下文管理
        std::unordered_map<uint32_t, Ctx::ptr> _reqCtxs;
        // 发送数据的队列
        std::list<SendCtx::ptr> _sendCtxQue;
        // 读写协程启动的信号量
        FiberSemphore _waitFiberSem;

        // 锁保证多线程下reqCtxs和senCtxQue的线程安全
        FiberMutex _queMtx;

        // 写协程无任务挂起信号量
        FiberCondition _cond;
        RWMutex _ctxsMtx;
        uint32_t _sn;              // 该异步socket的请求序列号ctx的sn
        bool _autoConnect;         // 是否自动重连
        uint32_t _tryConnectCount; // 重连次数
        Timer::ptr _timer;         // 重连定时器

        IOManager *_ioWorker;      // io调度器
        IOManager *_processWorker; // 逻辑处理调度器

        connect_cb _connectCb;       // socket连接建立回调
        disconnect_cb _disconnectCb; // socket连接关闭回调

        boost::any _data; // 存放该socket流的任意类型数据
    public:
        bool _recving; // 正在接收
    };
}

#endif