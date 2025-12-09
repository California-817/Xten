#include "rock_stream.h"
#include "../log.h"
#include "../util.h"
#include "../iomanager.h"
#include "../config.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    static ConfigVar<int>::ptr g_tryCount = Config::LookUp<int>("rockclient.reconnect.trycount", 20, "rockClient reconnect tryCount");
    RockStream::RockStream(Socket::ptr socket, bool is_owner, bool auto_connect)
        : SocketStream(socket, is_owner)
    {
        _decoder = std::make_shared<RockMessageDecoder>();
        // _autoConnect = auto_connect;
    }
    RockStream::~RockStream()
    {
        if (IsConnected() && _disconnectCb)
            _disconnectCb(nullptr);
        XTEN_LOG_DEBUG(g_logger) << "RockStream::~RockStream "
                                 << this << " "
                                 << (_socket ? _socket->tostring() : "");
    }
    // 客户端请求函数
    RockResult::ptr RockStream::Request(RockRequest::ptr request, uint64_t timeout_ms)
    {
        // ReTry:
        if (IsConnected())
        {
            if (request->GetSn() == 0)
            {
                // 设置请求序列号
                request->SetSn(Atomic::addFetch(_sn, 1));
            }
            // 看是否有超时限制
            uint64_t begin_reqtime_ms = TimeUitl::GetCurrentMS();
            if (timeout_ms > 0)
            {
                _socket->SetRecvTimeOut(timeout_ms);
            }
            // 发送请求
            int ret = _decoder->SerializeToStream(shared_from_this(), request);
            if (ret < 0)
            {
                if (_disconnectCb)
                    _disconnectCb(shared_from_this());
                SocketStream::Close();
                return std::make_shared<RockResult>(request, nullptr, ERROR::IO_ERROR, "io_error", 0);
            }
            // 接收响应
            RockResponse::ptr rsp;
            Message::ptr msg = RecvMessage();
            uint64_t usedTime = TimeUitl::GetCurrentMS() - begin_reqtime_ms;
            if (!msg)
            {
                // recv error----->大概率是因为超时了
                if (timeout_ms > 0 && usedTime > timeout_ms)
                    return std::make_shared<RockResult>(request, nullptr, ERROR::TIMEOUT, "timeout", usedTime);
                else // 接收错误 但不是超时
                {
                    if (_disconnectCb)
                        _disconnectCb(shared_from_this());
                    SocketStream::Close();
                    return std::make_shared<RockResult>(request, nullptr, ERROR::IO_ERROR, "io_error", 0);
                }
            }
            rsp = std::dynamic_pointer_cast<RockResponse>(msg);
            RockResult::ptr result = std::make_shared<RockResult>(request, rsp,
                                                                  rsp->GetResult(), rsp->GetResultStr(),
                                                                  usedTime);
            result->server = GetPeerAddrString();
            return result;
        }
        else
        {
            // 链接未成功建立
            // if (_autoConnect && _socket)
            // {
            // while (_tryCount < g_tryCount->GetValue())
            // {
            // if (_socket->ReConnect())
            // {
            // _connectCb(shared_from_this());
            // _tryCount = 0;
            // goto ReTry;
            // }
            // usleep((++_tryCount) * 100);
            // }
            // 尝试很多次依旧连不上--->这次请求直接返回,下次请求的时候再次尝试连接
            // _tryCount = 0;
            // }
            RockResult::ptr result = std::make_shared<RockResult>(request, nullptr, ERROR::NOT_CONNECT, "not_connect", 0);
            return result;
        }
    }
    // 接受数据的函数
    Message::ptr RockStream::RecvMessage()
    {
        Message::ptr msg = _decoder->ParseFromStream(shared_from_this());
        if (!msg)
        {
            // SocketStream::Close();  超时返回的话继续发送
            return nullptr;
        }
        Message::MessageType type = (Message::MessageType)msg->GetMessageType();
        if (type == Message::MessageType::REQUEST)
        {
            // 请求
            RockRequest::ptr req = std::dynamic_pointer_cast<RockRequest>(msg);
            if (!req)
            {
                XTEN_LOG_ERROR(g_logger) << "doRecv request not RockRequest";
                return nullptr;
            }
            if (!_requestHandleCb)
            {
                XTEN_LOG_ERROR(g_logger) << "Not set the request handle";
                return nullptr;
            }
            return req;
        }
        else if (type == Message::MessageType::RESPONSE)
        {
            // 响应
            RockResponse::ptr rsp = std::dynamic_pointer_cast<RockResponse>(msg);
            if (!rsp)
            {
                XTEN_LOG_ERROR(g_logger) << "doRecv response not RockResponse";
                return nullptr;
            }
            return rsp;
        }
        else if (type == Message::MessageType::NOTIFY)
        {
            // 通知
            RockNotify::ptr ntf = std::dynamic_pointer_cast<RockNotify>(msg);
            if (!ntf)
            {
                XTEN_LOG_ERROR(g_logger) << "doRecv notify not RockNotify";
                return nullptr;
            }
            if (!_notifyHandleCb)
            {
                XTEN_LOG_ERROR(g_logger) << "Not set the notify handle";
                return nullptr;
            }
            // 将通知处理交给process调度器
            return ntf;
        }
        else
        {
            // invaild type
            XTEN_LOG_ERROR(g_logger) << "doRecv recv a invaild message type";
        }
        return nullptr;
    }
    RockSession::RockSession(Socket::ptr socket)
        : RockStream(socket, true, false)
    {
    }
    RockConnection::RockConnection()
        : RockStream(nullptr, true, true)
    {
    }
    // 与服务端建立连接
    bool RockConnection::Connect(Address::ptr addr)
    {
        _socket = Socket::CreateTCPSocket();
        bool ret = _socket->Connect(addr);
        if (ret && _connectCb)
        {
            // 连接success--->执行连接建立回调函数
            _connectCb(shared_from_this());
        }
        return ret;
    }

    bool RockConnectionPool::IsConnectionExpired()
    {
        // 判断连接是否超时
        uint64_t now_ms = TimeUitl::GetCurrentMS();
        uint64_t last_active_ms = _freeConns.front().second;
        if (_idleTolerance != 0 && now_ms - last_active_ms > _idleTolerance)
        {
            return true;
        }
        return false;
    }
    RockConnectionPool::RockConnectionPool(const std::string &ip, uint16_t port,
                                           const std::string &realType, size_t maxSize, unsigned long long idleTolerace)
        : _totalSize(maxSize),
          _realType(realType),
          _cond(_mutex),
          _isStop(false),
          _idleTolerance(idleTolerace),
          _ip(ip),
          _port(port)
    {
        XTEN_LOG_INFO(g_logger) << "RockConnectionPool create pool realType="
                                << realType << " maxSize=" << maxSize
                                << " idleTolerace=" << idleTolerace;
    }
    void RockConnectionPool::Add(RockConnection::ptr conn)
    {
        FiberMutex::Lock lock(_mutex);
        Address::ptr address = IPv4Address::Create(_ip.c_str(), _port);
        conn->Connect(address);
        _freeConns.push_back(std::make_pair(conn, TimeUitl::GetCurrentMS()));
    }
    RockConnectionPool::~RockConnectionPool()
    {
        {
            FiberMutex::Lock lock(_mutex);
            _isStop = true;
            while (!_busyConns.empty())
            {
                _cond.wait();
            }
            // for (auto &i : _freeConns)
            // {
            //     i.first->Close();
            // }
            XTEN_ASSERT(_busyConns.empty());
            _freeConns.clear();
        }
        _cond.broadcast();
    }
    RockConnection::ptr RockConnectionPool::Get()
    {
        // 1.从_freeConns获取
        FiberMutex::Lock lock(_mutex);
        while (!_isStop && _freeConns.empty())
        {
            _cond.wait();
        }
        if (_isStop)
            return nullptr;
        // 有conn
        RockConnection::ptr conn = _freeConns.front().first;
        uint64_t last_time=_freeConns.front().second;
        _freeConns.pop_front();
        _busyConns.push_back(conn);
        lock.unlock();
        // 2.判断连接是否超时，未超时直接返回
        do
        {
            if (!IsConnectionExpired())
            {
                // 未超时
                break;
            }
            // 3.超时则request一下，成功直接返回
            RockRequest::ptr req = std::make_shared<RockRequest>();
            req->SetCmd(0); // 心跳请求
            req->SetData("ping");
            RockResult::ptr result = conn->Request(req);
            if ((ERROR)result->resultCode == ERROR::IO_ERROR || (ERROR)result->resultCode == ERROR::NOT_CONNECT)
            {
                Address::ptr addr = conn->GetPeerAddr();
                // 3.2 request失败则重新连接
                if (conn->Connect(addr))
                    break;
                else
                // 3.3 连接失败则放回该连接
                {
                    XTEN_LOG_WARN(g_logger) << "RockConnectionPool::Get reconnect fail";
                    {
                        FiberMutex::Lock lock(_mutex);
                        //not update used time , ensure next get wont return this conn when no expired
                        _freeConns.push_back(std::make_pair(conn,last_time));
                        _busyConns.remove(conn);
                    }
                    return nullptr;
                }
            }
        } while (false);
        // 4.返回连接的智能指针使用separate counter 并使用自定义删除器
        return RockConnection::ptr(conn.get(),
                                   std::bind(&RockConnectionPool::releaseDeleter, this, std::placeholders::_1));
    }

    void RockConnectionPool::releaseDeleter(RockConnection *ptr)
    {
            FiberMutex::Lock lock(_mutex);
            RockConnection::ptr temp(ptr,Xten::nop<RockConnection>);
            auto iter=std::find(_busyConns.begin(),_busyConns.end(),temp);
            XTEN_ASSERT(iter!=_busyConns.end());

            temp=*iter; //This line is necessary,
            //or the pointer hold by it will be deleted after the second line

            _busyConns.erase(iter);
            _freeConns.push_back(std::make_pair(temp,TimeUitl::GetCurrentMS())); // update last used time
            _cond.signal(); //rouse fiber
            XTEN_LOG_DEBUG(g_logger)<<"Free "<<_realType<<" ,freeSize="<<_freeConns.size();
    }
}

// #include "rock_stream.h"
// #include "../log.h"
// #include "../util.h"
// #include "../address.h"
// namespace Xten
// {
//     static Logger::ptr g_logger = XTEN_LOG_NAME("g_logger");
//     RockStream::RockStream(Socket::ptr socket, bool is_owner, bool auto_connect)
//         : AsyncSocketStream(socket, is_owner, auto_connect)
//     {
//         _decoder = std::make_shared<RockMessageDecoder>();
//     }
//     RockStream::~RockStream()
//     {
//         XTEN_LOG_DEBUG(g_logger) << "RockStream::~RockStream "
//                                  << this << " "
//                                  << (_socket ? _socket->tostring() : "");
//     }
//     // 客户端请求函数
//     RockResult::ptr RockStream::Request(RockRequest::ptr request, uint64_t timeout_ms)
//     {
//         if (IsConnected())
//         {
//             if (request->GetSn() == 0)
//             {
//                 // 设置请求序列号
//                 request->SetSn(Atomic::addFetch(_sn, 1));
//             }
//             // 创建请求上下文
//             RockCtx::ptr reqctx = std::make_shared<RockCtx>();
//             reqctx->req = request;
//             reqctx->sn = request->GetSn();
//             reqctx->scheduler = Scheduler::GetThis();
//             reqctx->timeout = (uint32_t)(timeout_ms);
//             reqctx->fiber = Fiber::GetThis();
//             // 添加到map中管理
//             addCtx(reqctx);
//             // 看是否有超时限制
//             uint64_t begin_reqtime_ms = TimeUitl::GetCurrentMS();
//             if (timeout_ms > 0)
//             {
//                 reqctx->timer = IOManager::GetThis()->addTimer(timeout_ms, // 添加请求超时定时器
//                                                                std::bind(&RockStream::onTimer, this, shared_from_this(), reqctx), false);
//             }
//             // 发送请求到发送队列中(最终由write协程调用ctx的doSend函数进行发送)
//             enqueue(reqctx);
//             // 挂起当前请求协程
//             Fiber::YieldToHold(); // 不用担心协程上下文丢失而无法调度回来(在RockCtx中保存了协程对象的智能指针)

//             // ------------- Fiber Back ---------------------
//             // 此时由于该请求的 响应收到 or 请求超时 协程被重新调度
//             // 创建rockresult
//             RockResult::ptr result = std::make_shared<RockResult>(reqctx->req, reqctx->rsp,
//                                                                   reqctx->result, reqctx->resultStr,
//                                                                   TimeUitl::GetCurrentMS() - begin_reqtime_ms);
//             result->server = GetPeerAddrString();
//             return result;
//         }
//         else
//         {
//             // 链接未成功建立
//             RockResult::ptr result = std::make_shared<RockResult>(request, nullptr, ERROR::NOT_CONNECT, "not_connect", 0);
//             return result;
//         }
//     }
//     // 接受数据的函数
//     AsyncSocketStream::Ctx::ptr RockStream::doRecv()
//     {
//         Message::ptr msg = _decoder->ParseFromStream(shared_from_this());
//         if (!msg)
//         {
//             innerClose();
//             return nullptr;
//         }
//         Message::MessageType type = (Message::MessageType)msg->GetMessageType();
//         if (type == Message::MessageType::REQUEST)
//         {
//             // 请求
//             RockRequest::ptr req = std::dynamic_pointer_cast<RockRequest>(msg);
//             if (!req)
//             {
//                 XTEN_LOG_ERROR(g_logger) << "doRecv request not RockRequest";
//                 return nullptr;
//             }
//             if (!_requestHandleCb)
//             {
//                 XTEN_LOG_ERROR(g_logger) << "Not set the request handle";
//                 return nullptr;
//             }
//             // 将请求处理和响应发送交给process调度器
//             _processWorker->Schedule(std::bind(&RockStream::handleRequest, this,
//                                                std::dynamic_pointer_cast<RockStream>(shared_from_this()), req));
//         }
//         else if (type == Message::MessageType::RESPONSE)
//         {
//             // 响应
//             RockResponse::ptr rsp = std::dynamic_pointer_cast<RockResponse>(msg);
//             if (!rsp)
//             {
//                 XTEN_LOG_ERROR(g_logger) << "doRecv response not RockResponse";
//                 return nullptr;
//             }
//             uint32_t sn = rsp->GetSn();
//             // 获取并删除请求上下文
//             RockCtx::ptr reqctx = getAndDelCtxAs<RockCtx>(sn);
//             if (!reqctx)
//             {
//                 // 没有正确找到请求上下文(大概率是超时了)
//                 XTEN_LOG_WARN(g_logger) << "RockStream request timeout reponse=" << rsp->ToString();
//                 return nullptr;
//             }
//             reqctx->result = rsp->GetResult();
//             reqctx->resultStr = rsp->GetResultStr();
//             reqctx->rsp = rsp;
//             return reqctx;
//         }
//         else if (type == Message::MessageType::NOTIFY)
//         {
//             // 通知
//             RockNotify::ptr ntf = std::dynamic_pointer_cast<RockNotify>(msg);
//             if (!ntf)
//             {
//                 XTEN_LOG_ERROR(g_logger) << "doRecv notify not RockNotify";
//                 return nullptr;
//             }
//             if (!_notifyHandleCb)
//             {
//                 XTEN_LOG_ERROR(g_logger) << "Not set the notify handle";
//                 return nullptr;
//             }
//             // 将通知处理交给process调度器
//             _processWorker->Schedule(std::bind(&RockStream::handleNotify, this,
//                                                std::dynamic_pointer_cast<RockStream>(shared_from_this()), ntf));
//         }
//         else
//         {
//             // invaild type
//             XTEN_LOG_ERROR(g_logger) << "doRecv recv a invaild message type";
//         }
//         return nullptr;
//     }
//     // 服务端发送函数
//     int32_t RockStream::sendMessage(Message::ptr msg)
//     {
//         if (IsConnected())
//         {
//             RockSendCtx::ptr sendctx = std::make_shared<RockSendCtx>();
//             sendctx->msg = msg;
//             // 发送msg到发送队列中(最终由write协程调用sendCtx的doSend函数进行发送)
//             enqueue(sendctx);
//             return 1;
//         }
//         return -1;
//     }
//     // 处理请求函数
//     void RockStream::handleRequest(RockStream::ptr self, RockRequest::ptr request)
//     {
//         RockResponse::ptr rsp = request->CreateResponse();
//         bool handleRet = _requestHandleCb(request, rsp, self);
//         if (!handleRet)
//         {
//             // 发送响应到发送队列
//             sendMessage(rsp);
//             Close();
//         }
//         sendMessage(rsp);
//     }
//     void RockStream::handleNotify(RockStream::ptr self, RockNotify::ptr notify)
//     {
//         bool handleRet = _notifyHandleCb(notify, self);
//         if (!handleRet)
//         {
//             Close();
//         }
//     }
//     RockSession::RockSession(Socket::ptr socket)
//         : RockStream(socket, true, false)
//     {
//     }
//     RockConnection::RockConnection()
//         : RockStream(nullptr, true, true)
//     {
//     }
//     // 与服务端建立连接
//     bool RockConnection::Connect(Address::ptr addr)
//     {
//         _socket = Socket::CreateTCPSocket();
//         return _socket->Connect(addr);
//     }
// }