#ifndef __XTEN_XFTP_SESSION_H__
#define __XTEN_XFTP_SESSION_H__
#include "../streams/socket_stream.h"
#include "xftp_protocol.h"
#include "../mutex.h"
#include <list>
namespace Xten
{
    namespace xftp
    {
        class XftpSession : public SocketStream, std::enable_shared_from_this<XftpSession>
        {
        public:
            typedef std::shared_ptr<XftpSession> ptr;
            typedef FiberMutex MutexType;
            typedef FiberCondition ConditionType;
            XftpSession(Socket::ptr socket, bool is_owner = true);
            // recv
            XftpRequest::ptr RecvRequest();
            // 由XftpWorker调用进行响应发送--入队列
            void pushResponse(uint32_t sn, XftpResponse::ptr rsp);

        private:
            // 真正发送响应--->启动协程将队列中的包发出去[发现空包则关闭连接]
            void doFlush(std::shared_ptr<XftpSession> self);

        private:
            // send queue
            std::list<std::pair<uint32_t, XftpResponse::ptr>> _sendQueue;
            // lock
            MutexType _mutex;
            // condition
            ConditionType _cond;
            // coder
            XftpMessageDecoder::ptr _decoder;
        };
    }
}
#endif