#ifndef __XTEN_KCP_SESSION_H__
#define __XTEN_KCP_SESSION_H__
#include "third_part/ikcp.h"
#include <memory>
namespace Xten
{
    namespace kcp
    {
        class KcpSession
        {
        public:
            friend class KcpListener;
            typedef std::shared_ptr<KcpSession> ptr;
            void Close();
            uint32_t GetConvId()
            {
                if (_kcp_cb)
                    return _kcp_cb->conv;
                return -1;
            }

        private:
            // 原始udp包处理 [data ， fromaddr]
            void packetInput(const char *data, size_t len);
            // 通知超时
            void notifyTimeout();
            // 通知close
            void notifyClose();
            // 通知accept
            void notifyAccept();
            // 通知读错误
            void notifyReadError(int code);
            IKCPCB *_kcp_cb; // kcp控制块
        };
    } // namespace kcp

} // namespace Xten

#endif