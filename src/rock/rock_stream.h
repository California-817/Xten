#ifndef __XTEN_ROCK_STREAM_H__
#define __XTEN_ROCK_STREAM_H__
#include "../streams/async_socket_stream.h"
#include "rock_protocol.h"
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
        typedef std::function<bool(RockNotify::ptr request, RockStream::ptr stream)>
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
        notifyCb _notifyHandleCb;         // 响应处理函数
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