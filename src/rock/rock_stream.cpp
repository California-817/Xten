#include "rock_stream.h"
#include "../log.h"
#include "../util.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("g_logger");
    RockStream::RockStream(Socket::ptr socket, bool is_owner, bool auto_connect)
        : AsyncSocketStream(socket, is_owner, auto_connect)
    {
        _decoder = std::make_shared<RockMessageDecoder>();
    }
    RockStream::~RockStream()
    {
        XTEN_LOG_DEBUG(g_logger) << "RockStream::~RockStream "
                                 << this << " "
                                 << (_socket ? _socket->tostring() : "");
    }
    // 客户端请求函数
    RockResult::ptr RockStream::Request(RockRequest::ptr request, uint64_t timeout_ms)
    {
        if (IsConnected())
        {
            if (request->GetSn() == 0)
            {
                // 设置请求序列号
                request->SetSn(Atomic::addFetch(_sn, 1));
            }
            // 创建请求上下文
            RockCtx::ptr reqctx = std::make_shared<RockCtx>();
            reqctx->req = request;
            reqctx->sn = request->GetSn();
            reqctx->scheduler = Scheduler::GetThis();
            reqctx->timeout = (uint32_t)(timeout_ms);
            reqctx->fiber = Fiber::GetThis();
            // 添加到map中管理
            addCtx(reqctx);
            // 看是否有超时限制
            uint64_t begin_reqtime_ms = TimeUitl::GetCurrentMS();
            if (timeout_ms > 0)
            {
                reqctx->timer = IOManager::GetThis()->addTimer(timeout_ms, // 添加请求超时定时器
                                                               std::bind(&RockStream::onTimer, this, shared_from_this(), reqctx), false);
            }
            // 发送请求到发送队列中(最终由write协程调用ctx的doSend函数进行发送)
            enqueue(reqctx);
            // 挂起当前请求协程
            Fiber::YieldToHold(); // 不用担心协程上下文丢失而无法调度回来(在RockCtx中保存了协程对象的智能指针)

            // ------------- Fiber Back ---------------------
            // 此时由于该请求的 响应收到 or 请求超时 协程被重新调度
            // 创建rockresult
            RockResult::ptr result = std::make_shared<RockResult>(reqctx->req, reqctx->rsp,
                                                                  reqctx->result, reqctx->resultStr,
                                                                  TimeUitl::GetCurrentMS());
            result->server = GetPeerAddrString();
            return result;
        }
        else
        {
            // 链接未成功建立
            RockResult::ptr result = std::make_shared<RockResult>(request, nullptr, ERROR::NOT_CONNECT, "not_connect", 0);
            return result;
        }
    }
    // 接受数据的函数
    AsyncSocketStream::Ctx::ptr RockStream::doRecv()
    {
        Message::ptr msg = _decoder->ParseFromStream(shared_from_this());
        if (!msg)
        {
            innerClose();
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
            // 将请求处理和响应发送交给process调度器
            _processWorker->Schedule(std::bind(&RockStream::handleRequest, this,
                                               std::dynamic_pointer_cast<RockStream>(shared_from_this()), req));
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
            uint32_t sn = rsp->GetSn();
            // 获取并删除请求上下文
            RockCtx::ptr reqctx = getAndDelCtxAs<RockCtx>(sn);
            if (!reqctx)
            {
                // 没有正确找到请求上下文(大概率是超时了)
                XTEN_LOG_WARN(g_logger) << "RockStream request timeout reponse=" << rsp->ToString();
                return nullptr;
            }
            reqctx->result = rsp->GetResult();
            reqctx->resultStr = rsp->GetResultStr();
            reqctx->rsp = rsp;
            return reqctx;
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
            _processWorker->Schedule(std::bind(&RockStream::handleNotify, this,
                                               std::dynamic_pointer_cast<RockStream>(shared_from_this()), ntf));
        }
        else
        {
            // invaild type
            XTEN_LOG_ERROR(g_logger) << "doRecv recv a invaild message type";
        }
        return nullptr;
    }
    // 服务端发送函数
    int32_t RockStream::sendMessage(Message::ptr msg)
    {
        if (IsConnected())
        {
            RockSendCtx::ptr sendctx = std::make_shared<RockSendCtx>();
            sendctx->msg = msg;
            // 发送msg到发送队列中(最终由write协程调用sendCtx的doSend函数进行发送)
            enqueue(sendctx);
            return 1;
        }
        return -1;
    }
    // 处理请求函数
    void RockStream::handleRequest(RockStream::ptr self, RockRequest::ptr request)
    {
        RockResponse::ptr rsp = request->CreateResponse();
        bool handleRet = _requestHandleCb(request, rsp, self);
        if (!handleRet)
        {
            // 发送响应到发送队列
            sendMessage(rsp);
            Close();
        }
        sendMessage(rsp);
    }
    void RockStream::handleNotify(RockStream::ptr self, RockNotify::ptr notify)
    {
        bool handleRet = _notifyHandleCb(notify, self);
        if (!handleRet)
        {
            Close();
        }
    }
    RockSession::RockSession(Socket::ptr socket)
        : RockStream(socket, true, false)
    {
    }
    RockConnection::RockConnection()
        : RockStream(nullptr,true,true)
    {
    }
    // 与服务端建立连接
    bool RockConnection::Connect(Address::ptr addr)
    {
        _socket=Socket::CreateTCPSocket();
        return _socket->Connect(addr);
    }

}