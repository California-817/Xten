#ifndef __XTEN_XFTP_SESSION_H__
#define __XTEN_XFTP_SESSION_H__
#include "../streams/socket_stream.h"
#include "xftp_protocol.h"
#include "../mutex.h"
#include <list>
namespace Xten
{
    class XftpSession : public SocketStream
    {
    public:
        friend class XftpWorker;
        typedef std::shared_ptr<XftpSession> ptr;
        typedef FiberMutex MutexType;
        XftpSession(Socket::ptr socket, bool is_owner = true);
    private:
        //由XftpWorker调用进行响应发送
        void pushResponse(uint32_t sn,XftpResponse::ptr rsp);
        //真正发送响应--->启动协程将队列中的包发出去
        void doFlush();
    private:
        // send queue
        std::list<std::pair<uint32_t, XftpResponse::ptr>> _sendQueue;
        // lock
        MutexType _mutex;
        // whether real send
        bool _flushing=false;
    };
}
#endif