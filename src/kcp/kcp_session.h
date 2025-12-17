#ifndef __XTEN_KCP_SESSION_H__
#define __XTEN_KCP_SESSION_H__
#include "third_part/ikcp.h"
#include "kcp_protocol.h"
#include "../iomanager.h"
#include "../socket.h"
#include <memory>
namespace Xten
{
    namespace kcp
    {
        class KcpListener;
        class KcpSession : public std::enable_shared_from_this<KcpSession>
        {
        protected:
            // 不允许外部随意创建
            KcpSession(std::weak_ptr<Socket> udp_channel, std::weak_ptr<KcpListener> listener, Address::ptr remote_addr, uint32_t convid,
                       int nodelay = 1,   // 0:disable(default), 1:enable  是否非延迟
                       int interval = 20, // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
                       int resend = 2,    // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
                       int nc = 1);       // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制

        public:
            friend class KcpListener;
            typedef std::shared_ptr<KcpSession> ptr;
            typedef FiberMutex MutexType;
            typedef FiberCondition CondType;
            void Start();
            ~KcpSession();
            void Close();
            // 读到一个message报文
            Message::ptr ReadMessage();

            void SendMessage(Message::ptr msg);

        private:
            // 获取kcpcb的convid
            uint32_t GetConvId() const { return _convid; }
            // 发送协程调用: [发送队列--->kcpcb发送缓冲区区]
            bool write();
            // 传给kcpcb的内部输出回调函数 [kcpcb缓冲区--->socket]
            static int kcp_output_func(const char *buf, int len, struct IKCPCB *kcp, void *user);
            // 原始udp包处理 [data]
            void packetInput(const char *data, size_t len);
            void update();
            // 通知read超时
            void notifyReadTimeout();
            // 通知write
            void notifyWriteTimeout();
            // 通知read就绪
            void notifyReadEvent();
            // 通知write就绪
            void notifyWriteEvent();
            // 通知读错误
            void notifyReadError(int code);
            // 通知写错误
            void notifyWriteError(int code);

        private:
            std::weak_ptr<Socket> _udp_channel;   // socket
            std::weak_ptr<KcpListener> _listener; // 监听er
            Address::ptr _remote_addr;            // 远端地址
            uint32_t _convid;                     // convid
            IKCPCB *_kcp_cb;                      // kcp控制块
            uint64_t _read_timeout_ms;            // 读超时时间
            uint64_t _write_timeout_ms;           // 写超时时间
            // std::list<
            MutexType _kcpcb_mtx; // 访问kcpcb的互斥锁
        };
    } // namespace kcp

} // namespace Xten

#endif