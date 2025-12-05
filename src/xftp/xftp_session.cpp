#include "xftp_session.h"
namespace Xten
{
    XftpSession::XftpSession(Socket::ptr socket, bool is_owner = true)
        : SocketStream(socket, is_owner)
    {
    }
    // 由XftpWorker调用进行响应发送
    void XftpSession::pushResponse(uint32_t sn, XftpResponse::ptr rsp)
    {
        // 连接还存在---发送响应
        XftpSession::MutexType::Lock lock(_mutex);
        _sendQueue.push_back(std::make_pair(sn, rsp));
        if (!_flushing)
        {
            // 真正发送
            _flushing = true;
            doFlush();
        }
    }
    // 真正发送响应--->启动协程将队列中的包发出去
    void XftpSession::doFlush()
    {
    }
}