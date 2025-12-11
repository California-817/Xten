#ifndef __XTEN_ROCK_STREAM_H__
#define __XTEN_ROCK_STREAM_H__

#define SYNC 0
#define ASYNC 1
#ifndef ROCK_CATEGORY
#define ROCK_CATEGORY ASYNC
#endif

#if ROCK_CATEGORY == SYNC

#include "../streams/socket_stream.h"
#include "rock_protocol.h"
#include"../mutex.h"
#include <functional>
#include <sstream>
#include<list>
namespace Xten
{
    // 响应错误码
    enum ERROR
    {
        OK = 0,           // 成功
        TIMEOUT = -1,     // 超时
        IO_ERROR = -2,    // io错误
        NOT_CONNECT = -3, // 未连接
    };
    // 对请求结果进行的上层应用层封装
    struct RockResult
    {
        typedef std::shared_ptr<RockResult> ptr;
        RockResult(RockRequest::ptr req, RockResponse::ptr rsp,
                   uint32_t code, std::string rtstr, uint64_t time, std::string ser = "NULL")
            : request(req),
              response(rsp),
              resultCode(code),
              resultStr(rtstr),
              usedTime(time),
              server(ser)
        {
        }
        std::string toString() const
        {
            std::stringstream ss;
            ss << "req: " << (request ? request->ToString() : "NULL") << "\n"
               << "rsp: " << (response ? response->ToString() : "NULL") << "\n"
               << "resultCode: " << resultCode << "\n"
               << "resultStr: " << resultStr << "\n"
               << "usedTime: " << usedTime << "\n"
               << "Server: " << server << std::endl;
            return ss.str();
        }
        RockRequest::ptr request;   // 请求
        RockResponse::ptr response; // 响应
        uint32_t resultCode;        // 响应码
        std::string resultStr;      // 响应码描述
        uint64_t usedTime;          // 请求耗时(ms)
        std::string server;         // 请求服务器
    };
    // 进行分布式通信的rock流
    class RockStream : public SocketStream, public std::enable_shared_from_this<RockStream>
    {
    public:
        typedef std::shared_ptr<RockStream> ptr;
        typedef std::function<bool(RockRequest::ptr request, RockResponse::ptr response,
                                   RockStream::ptr stream)>
            requestCb;
        typedef std::function<bool(RockNotify::ptr notify, RockStream::ptr stream)>
            notifyCb;
        typedef std::function<bool(RockStream::ptr)> connect_cb;
        typedef std::function<bool(RockStream::ptr)> disconnect_cb;
        RockStream(Socket::ptr socket, bool is_owner = true, bool auto_connect = false);
        virtual ~RockStream();

        // 设置连接回调
        void SetConnectCb(connect_cb cb) { _connectCb = cb; }
        // 设置断连回调
        void SetDisConnectCb(disconnect_cb cb) { _disconnectCb = cb; }
        // 获取回调
        connect_cb GetConnectCb() const { return _connectCb; }
        disconnect_cb GetDisConnectCb() { return _disconnectCb; }
        // 调用响应函数
        bool OnConnect(RockStream::ptr s)
        {
            return _connectCb(s);
        }
        bool OnDisConnect(RockStream::ptr s)
        {
            return _disconnectCb(s);
        }
        // 设置请求处理函数(由RockModule提供并注册)
        void SetRequestHandleCb(requestCb reqcb) { _requestHandleCb = reqcb; }
        // 设置通知处理函数(由RockModule提供并注册)
        void SetNotifyHandleCb(notifyCb ntycb) { _notifyHandleCb = ntycb; }
        // 获取请求处理函数
        requestCb GetRequestHandleCb() const { return _requestHandleCb; }
        // 获取通知处理函数
        notifyCb GetNotifyHandleCb() const { return _notifyHandleCb; }

        // 客户端请求函数(返回RockResult)
        RockResult::ptr Request(RockRequest::ptr request, uint64_t timeout_ms = 0);

        // 服务端接收请求并处理函数
        Message::ptr RecvMessage();
        // 服务端发送消息函数
        int SendMessage(Message::ptr msg)
        {
            return _decoder->SerializeToStream(shared_from_this(), msg);
        }

    protected:
        RockMessageDecoder::ptr _decoder; // rock协议报文编解码器

        connect_cb _connectCb;       // 连接建立回调
        disconnect_cb _disconnectCb; // 连接断开回调
        requestCb _requestHandleCb;  // 请求处理函数
        notifyCb _notifyHandleCb;    // 通知处理函数

        // bool _autoConnect; // 是否自动重新连接
        // int _tryCount = 0; // 尝试重新连接次数
        uint32_t _sn;      // 该socket的请求序列号sn
    };
    // 分布式服务端的RockSession
    class RockSession : public RockStream
    {
    public:
        typedef std::shared_ptr<RockSession> ptr;
        RockSession(Socket::ptr socket);
        virtual ~RockSession() = default;
    };
    // 分布式客户端的RockConnection
    class RockConnection : public RockStream
    {
    public:
        typedef std::shared_ptr<RockConnection> ptr;
        RockConnection();
        // 与服务端建立连接
        bool Connect(Address::ptr addr);
        virtual ~RockConnection() = default;
    };
    class RockConnectionPool : public NoCopyable
    {
    public:
        typedef std::shared_ptr<RockConnectionPool> ptr;
        RockConnectionPool(const std::string& ip,uint16_t port,const std::string &realType = "RockConnection", size_t maxSize = 10, 
            unsigned long long idleTolerace_ms = 0);
        // 添加一个连接---添加的是业务实现的子类连接
        void Add(RockConnection::ptr conn);
        ~RockConnectionPool();
        // 获取一个连接
        RockConnection::ptr Get();
        size_t GetMaxSize() const { return _totalSize; }
        unsigned long long GetIdleTolerance() const { return _idleTolerance; }
        std::string GetRealType() const { return _realType; }
    private:
        // 自定义删除器释放连接
        void releaseDeleter(RockConnection *ptr);
        bool IsConnectionExpired();

    private:
        std::string _realType;                                                    // 连接的真实类型---业务决定
        std::list<RockConnection::ptr> _busyConns;                                // 正在使用的连接
        FiberMutex _mutex;                                                        // 保护连接池的协程互斥锁
        std::list<std::pair<RockConnection::ptr, unsigned long long>> _freeConns; // 空闲连接及其上次使用时刻
        FiberCondition _cond;                                                     // 连接池条件变量
        size_t _totalSize;                                                        // 连接池最大连接数
        std::atomic<bool> _isStop;                                                // 连接池是否停止工作
        unsigned long long _idleTolerance;                                        // 空闲连接最大存活时间
        std::string _ip;
        uint16_t _port;
    };
}








#elif ROCK_CATEGORY == ASYNC

// // 客户端默认自动重连，用户上层只要进行初始的第一次start，后续如果由于长时间没发包给服务端导致链接被服务器断开，客户端的这个RockConnect不会被销毁[即使链接不存在]
// // 客户端首先会通过innerclose关闭此次连接的读写协程，然后在读协程内部通过定时器立即调用start函数进行重连，若start失败则又会按照一定规则累加的超时时间重连服务端
// // 直到最终重连成功，重新开启两个读写协程进行io操作，若在链接未建立过程发起request，则直接返回超时结果
// // 对于服务端RockSession，连接长时间未交互，则通过read超时使得读协程退出并调用innerclose，唤醒并间接退出写协程，由于auto_connect参数为false，由于此时只有读写
// // 协程带有RockSession的智能指针，并且不自动重连，服务端将tcp链接以及框架层的RockSession全部销毁，服务端会调用～RockStream

#include "../streams/async_socket_stream.h"
#include "rock_protocol.h"
#include "../mutex.h"
#include <list>
namespace Xten
{
    // 对请求结果进行的上层应用层封装
    struct RockResult
    {
        typedef std::shared_ptr<RockResult> ptr;
        RockResult(RockRequest::ptr req, RockResponse::ptr rsp,
                   uint32_t code, std::string rtstr, uint64_t time, std::string ser = "NULL")
            : request(req),
              response(rsp),
              resultCode(code),
              resultStr(rtstr),
              usedTime(time),
              server(ser)
        {
        }
        std::string toString() const
        {
            std::stringstream ss;
            ss << "req: " << (request ? request->ToString() : "NULL") << "\n"
               << "rsp: " << (response ? response->ToString() : "NULL") << "\n"
               << "resultCode: " << resultCode << "\n"
               << "resultStr: " << resultStr << "\n"
               << "usedTime: " << usedTime << "\n"
               << "Server: " << server << std::endl;
            return ss.str();
        }
        RockRequest::ptr request;   // 请求
        RockResponse::ptr response; // 响应
        uint32_t resultCode;        // 响应码
        std::string resultStr;      // 响应码描述
        uint64_t usedTime;          // 请求耗时(ms)
        std::string server;         // 请求服务器
    };
    // 进行分布式通信的rock流
    class RockStream : public AsyncSocketStream
    {
    public:
        typedef std::shared_ptr<RockStream> ptr;
        typedef std::function<bool(RockRequest::ptr request, RockResponse::ptr response,
                                   RockStream::ptr stream)>
            requestCb;
        typedef std::function<bool(RockNotify::ptr notify, RockStream::ptr stream)>
            notifyCb;

        RockStream(Socket::ptr socket, bool is_owner = true, bool auto_connect = false);
        virtual ~RockStream();

        struct RockSendCtx : public SendCtx
        {
            typedef std::shared_ptr<RockSendCtx> ptr;
            virtual ~RockSendCtx() = default;
            virtual bool doSend(AsyncSocketStream::ptr stream) override
            {
                auto rockstream = std::dynamic_pointer_cast<RockStream>(stream);
                if (rockstream)
                {
                    if (rockstream->_decoder->SerializeToStream(stream, msg) > 0)
                    {
                        return true;
                    }
                }
                return false;
            }

            Message::ptr msg; // 消息
        };
        // rock协议的请求上下文
        struct RockCtx : public Ctx
        {
            typedef std::shared_ptr<RockCtx> ptr;
            virtual ~RockCtx() = default;
            virtual bool doSend(AsyncSocketStream::ptr stream) override
            {
                auto rockstream = std::dynamic_pointer_cast<RockStream>(stream);
                if (rockstream)
                {
                    if (rockstream->_decoder->SerializeToStream(stream, req) > 0)
                    {
                        return true;
                    }
                }
                return false;
            }
            RockRequest::ptr req;  // 请求
            RockResponse::ptr rsp; // 响应
        };
        // 设置请求处理函数(由RockModule提供并注册)
        void SetRequestHandleCb(requestCb reqcb) { _requestHandleCb = reqcb; }
        // 设置通知处理函数(由RockModule提供并注册)
        void SetNotifyHandleCb(notifyCb ntycb) { _notifyHandleCb = ntycb; }
        // 获取请求处理函数
        requestCb GetRequestHandleCb() const { return _requestHandleCb; }
        // 获取通知处理函数
        notifyCb GetNotifyHandleCb() const { return _notifyHandleCb; }

        // 客户端请求函数(返回RockResult)
        RockResult::ptr Request(RockRequest::ptr request, uint64_t timeout_ms = 0);

    protected:
        // 接受数据的函数（只有正确收到服务端响应的时候才会返回Ctx）
        virtual Ctx::ptr doRecv() override;
        // 服务端发送函数
        int32_t sendMessage(Message::ptr msg);
        // 处理请求函数
        void handleRequest(RockStream::ptr self, RockRequest::ptr request);
        void handleNotify(RockStream::ptr self, RockNotify::ptr notify);
        RockMessageDecoder::ptr _decoder; // rock协议报文编解码器
        requestCb _requestHandleCb;       // 请求处理函数
        notifyCb _notifyHandleCb;         // 通知处理函数
    };
    // 分布式服务端的RockSession
    class RockSession : public RockStream
    {
    public:
        typedef std::shared_ptr<RockSession> ptr;
        RockSession(Socket::ptr socket);
        virtual ~RockSession() = default;
    };
    // 分布式客户端的RockConnection
    class RockConnection : public RockStream
    {
    public:
        typedef std::shared_ptr<RockConnection> ptr;
        RockConnection();
        // 与服务端建立连接
        bool Connect(Address::ptr addr);
        virtual ~RockConnection() = default;
    };
}
#endif

#endif
