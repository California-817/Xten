#include "xftp_session.h"
#include "../iomanager.h"
namespace Xten
{
    namespace xftp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");

        XftpSession::XftpSession(Socket::ptr socket, bool is_owner)
            : SocketStream(socket, is_owner),
              _cond(_mutex),
              _decoder(std::make_shared<XftpMessageDecoder>())
        {
            // 不可以在构造函数内shared_from_this() 还没有被智能指针捕获
        }
        // 启动写协程
        void XftpSession::startWriter()
        {
            // 启动写协程不断从队列中获取消息并顺序发送
            auto self = shared_from_this();
            Xten::IOManager::GetThis()->Schedule(std::bind(&XftpSession::doFlush, this, self));
        }
        XftpRequest::ptr XftpSession::RecvRequest()
        {
            Message::ptr msg = _decoder->ParseFromStream(shared_from_this());
            if (msg)
            {
                XftpRequest::ptr req = std::dynamic_pointer_cast<XftpRequest>(msg);
                if (req)
                    return req;
            }
            return nullptr;
        }
        // 由XftpWorker调用协程进行响应发送
        void XftpSession::pushResponse(uint32_t sn, XftpResponse::ptr rsp)
        {
            // 连接还存在---发送响应
            {
                XftpSession::MutexType::Lock lock(_mutex);
                _sendQueue.push_back(std::make_pair(sn, rsp));
            }
            _cond.signal();
        }
        // 真正发送响应--->启动协程将队列中的包发出去
        void XftpSession::doFlush(std::shared_ptr<XftpSession> self)
        {
            XTEN_LOG_DEBUG(g_logger) << "write fiber begin";
            while (_socket->IsConnected())
            {
                // 1.connecting
                std::list<std::pair<uint32_t, XftpResponse::ptr>> rsps;
                {
                    MutexType::Lock lock(_mutex);
                    while (_sendQueue.empty() && _socket->IsConnected())
                    {
                        _cond.wait();
                    }
                    rsps.swap(_sendQueue);
                }
                // 2.发送
                bool isStop = false;
                for (auto &rsp : rsps)
                {
                    if (!rsp.second)
                    { // 上层发了空包---说明要关闭连接
                        isStop = true;
                        continue;
                    }
                    if (_decoder->SerializeToStream(self, rsp.second) < 0)
                        // error
                        break;
                }
                if (isStop)
                    break;
            }
            _socket->Close();
            XTEN_LOG_DEBUG(g_logger) << "XftpSession.writeFiber out";
        }
    }
}