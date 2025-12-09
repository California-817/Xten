#include "async_socket_stream.h"
#include "log.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    AsyncSocketStream::AsyncSocketStream(Socket::ptr socket, bool is_owner, bool auto_connect)
        : SocketStream(socket, is_owner),
          _waitFiberSem(2),
          _sn(0),
          _autoConnect(auto_connect),
          _tryConnectCount(0),
          _ioWorker(nullptr),
          _processWorker(nullptr),
          _cond(_queMtx)
    {
    }
    // 启动异步socket流
    bool AsyncSocketStream::Start(AsyncSocketStream::ptr self)
    {
        if (!_ioWorker)
        {
            _ioWorker = Xten::IOManager::GetThis();
        }
        if (!_processWorker)
        {
            _processWorker = Xten::IOManager::GetThis();
        }
        do
        {
            // 保证上次开启的读写协程已经退出
            waitFiber();
            if (!IsConnected())
            {
                if (!_socket->ReConnect())
                {
                    // 未建立链接并且重新建立链接失败
                    innerClose();
                    _waitFiberSem.post();
                    _waitFiberSem.post();
                    break;
                }
            }
            if (_connectCb)
            {
                if (!_connectCb(shared_from_this()))
                {
                    // 执行connectCb失败
                    innerClose();
                    _waitFiberSem.post();
                    _waitFiberSem.post();
                    break;
                }
            }
            // 链接成功并且connectCb调用成功--->启动读写协程
            startRead();
            startWrite();
            _tryConnectCount = 0;
            return true;
        } while (false);
        // 启动异步socket失败
        _tryConnectCount++;
        if (_autoConnect)
        {
            // 启动重连
            if (_timer)
            {
                _timer->cancel();
                _timer = nullptr;
            }
            uint64_t tryTime = _tryConnectCount * 2 * 50;
            if (tryTime > 2000)
            {
                tryTime = 2000;
            }
            _timer = _ioWorker->addTimer(tryTime, std::bind(&AsyncSocketStream::Start, this, shared_from_this()), false);
        }
        return false;
    }
    // 关闭异步socket流
    void AsyncSocketStream::Close()
    {
        _autoConnect = false;
        // 此时不一定是io调度器，但是关闭socket需要cancelAll事件，需要在这个io调度器中取消事件
        SwitchScheduler ss(_ioWorker);
        if (_timer)
        {
            _timer->cancel();
            _timer = nullptr;
        }
        SocketStream::Close();
    }
    // 写协程函数
    void AsyncSocketStream::doWrite(AsyncSocketStream::ptr self)
    {
        try
        {
            while (IsConnected())
            {
                std::list<SendCtx::ptr> copyQue;
                {
                    FiberMutex::Lock lock(_queMtx);
                    while (_sendCtxQue.empty() && IsConnected())
                    {
                        _cond.wait();
                    }
                    // 有任务被唤醒
                    _sendCtxQue.swap(copyQue); // 先交换出来，再发送（减小锁粒度）
                }
                for (auto &ctx : copyQue)
                {
                    if (!ctx->doSend(shared_from_this())) // 发送该请求
                    {
                        innerClose();
                        break;
                    }
                }
            }
        }
        catch (...)
        {
        }
        {
            FiberMutex::Lock lock(_queMtx);
            _sendCtxQue.clear();
        }
        _waitFiberSem.post();
    }
    // 读协程函数
    void AsyncSocketStream::doRead(AsyncSocketStream::ptr self)
    {
        try
        {
            while (IsConnected())
            {
                _recving = true;
                // 接收响应并返回对应请求的上下文
                Ctx::ptr ctx = doRecv();
                _recving = false;
                if (ctx)
                {
                    // 这个函数来设置请求的响应结果，并唤醒等待响应的协程
                    ctx->doRsp();
                }
            }
        }
        catch (...)
        {
        }
        innerClose();
        _waitFiberSem.post();
        if (_autoConnect)
        {
            _timer = _ioWorker->addTimer(10, std::bind(&AsyncSocketStream::Start, this, shared_from_this()), false);
        }
    }
    // 开启读写协程
    void AsyncSocketStream::startRead()
    {
        _ioWorker->Schedule(std::bind(&AsyncSocketStream::doRead, this, shared_from_this()));
    }
    void AsyncSocketStream::startWrite()
    {
        _ioWorker->Schedule(std::bind(&AsyncSocketStream::doWrite, this, shared_from_this()));
    }
    // 请求超时函数
    void AsyncSocketStream::onTimer(AsyncSocketStream::ptr self, Ctx::ptr ctx)
    {
        XTEN_LOG_INFO(g_logger) << "on timeout: sn=" << ctx->sn;
        {
            RWMutex::WriteLock wlock(_ctxsMtx);
            _reqCtxs.erase(ctx->sn);
        }
        ctx->timed = true;
        ctx->timer = nullptr;
        ctx->doRsp();
    }
    // 根据请求的sn获取请求上下文ctx
    AsyncSocketStream::Ctx::ptr AsyncSocketStream::getCtx(uint32_t sn)
    {
        RWMutex::ReadLock rlock(_ctxsMtx);
        auto iter = _reqCtxs.find(sn);
        if (iter != _reqCtxs.end())
        {
            return iter->second;
        }
        return nullptr;
    }

    // 根据请求的sn获取并删除管理的请求上下文ctx
    AsyncSocketStream::Ctx::ptr AsyncSocketStream::getAndDelCtx(uint32_t sn)
    {
        RWMutex::ReadLock rlock(_ctxsMtx);
        auto iter = _reqCtxs.find(sn);
        if (iter != _reqCtxs.end())
        {
            _reqCtxs.erase(iter);
            return iter->second;
        }
        return nullptr;
    }
    // 添加对请求上下文ctx的管理 [ sn <---> ctx ]
    bool AsyncSocketStream::addCtx(Ctx::ptr ctx)
    {
        RWMutex::WriteLock wlock(_ctxsMtx);
        _reqCtxs.insert(std::make_pair(ctx->sn, ctx));
        return true;
    }
    // 将请求放入发送队列
    bool AsyncSocketStream::enqueue(AsyncSocketStream::SendCtx::ptr sendctx)
    {
        XTEN_ASSERT(sendctx);
        {
            FiberMutex::Lock lock(_queMtx);
            _sendCtxQue.push_back(sendctx);
        }
        _cond.signal();
        return true;
    }
    // 等待读写协程退出
    void AsyncSocketStream::waitFiber()
    {
        // 读写协程退出后释放两个信号量后这个函数才返回
        _waitFiberSem.wait();
        _waitFiberSem.wait();
    }
    // 内部进行关闭socket流
    bool AsyncSocketStream::innerClose()
    {
        XTEN_ASSERT(_ioWorker == IOManager::GetThis());
        if (IsConnected() && _disconnectCb)
        {
            _disconnectCb(shared_from_this());
        }
        onClose();
        SocketStream::Close();
        _cond.broadcast();
        // 获取所有未得到响应的请求上下文
        std::unordered_map<uint32_t, Ctx::ptr> copyReqCtxs;
        {
            RWMutex::WriteLock wlock(_ctxsMtx);
            _reqCtxs.swap(copyReqCtxs);
        }
        // 清理发送队列
        {
            FiberMutex::Lock lock(_queMtx);
            _sendCtxQue.clear();
        }
        // 进行请求的响应处理
        for (auto &ctx : copyReqCtxs)
        {
            ctx.second->result = ERROR::IO_ERROR;
            ctx.second->resultStr = "io_error";
            ctx.second->doRsp();
        }
        return true;
    }
}