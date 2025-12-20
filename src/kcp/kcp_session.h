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
        class KcpServer;
        class KcpSession : public std::enable_shared_from_this<KcpSession>
        {
        protected:
            // 不允许外部随意创建
            KcpSession(std::weak_ptr<Socket> udp_channel, std::weak_ptr<KcpListener> listener, Address::ptr remote_addr, uint32_t convid,
                       int nodelay = 1,                                                   // 0:disable(default), 1:enable  是否非延迟
                       int interval = 20,                                                 // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
                       int resend = 2,                                                    // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
                       int nc = 1, int mtxsize = 1400, int sndwnd = 32, int rcvwnd = 32); // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制

        public:
            enum READ_ERRNO
            {
                SUCCESS = 0,              // 成功
                READ_TIMEOUT = (1 << 0),  // 读超时
                READ_ERROR = (1 << 1),    // 读错误
                SESSION_CLOSE = (1 << 2), // 连接关闭
            };

            friend class KcpListener;
            typedef std::shared_ptr<KcpSession> ptr;
            typedef FiberMutex MutexType;
            typedef FiberCondition CondType;
            typedef FiberSemphore SemType;
            // 启动发送协程 [发送队列-->kcpcb发送缓冲区]
            void Start();
            ~KcpSession();
            void Close();
            // 读到一个message报文
            Message::ptr ReadMessage(KcpSession::READ_ERRNO &error);
            // 发送一个包文--->into queue [注意：包文大小最大不能超过1024+512字节]
            void SendMessage(Message::ptr msg);
            // 等待写协程退出
            void WaitSender() { _sem.wait(); }
            // 强制关闭连接
            void ForceClose();
            // 设置read超时时间[0表示不超时]
            void SetReadTimeout(uint64_t v = 0) { _read_timeout_ms = v; }
            // 获取accept超时时间
            uint64_t GetReadTimeout() const { return _read_timeout_ms; }
            // 设置当前服务器
            void SetKcpServer(std::shared_ptr<KcpServer> serv)
            {
                _server = std::weak_ptr<KcpServer>(serv);
            }
            // 获取当前服务器
            std::shared_ptr<KcpServer> GetKcpServer()
            {
                auto serv = _server.lock();
                if (serv)
                    return serv;
                return nullptr;
            }
            void SetInServerContainerId(const char *id) { _in_server_container_id = id; }
            std::string GetInServerContainerId() const { return _in_server_container_id; }

        private:
            // 获取kcpcb的convid
            uint32_t GetConvId() const { return _convid; }
            // 发送协程
            void dopacketOutput(std::shared_ptr<KcpSession> self);
            // 发送协程调用: [发送队列--->kcpcb发送缓冲区区]
            bool write(Message::ptr rsp);
            // 传给kcpcb的内部输出回调函数 [kcpcb缓冲区--->socket]
            static int kcp_output_func(const char *buf, int len, struct IKCPCB *kcp, void *user);
            // 原始udp包处理 [data]
            void packetInput(const char *data, size_t len);
            void update();
            // 通知read超时
            void notifyReadTimeout();
            // 通知read就绪
            void notifyReadEvent();
            // 通知读错误
            void notifyReadError(int code);
            // 通知写错误
            void notifyWriteError(int code);

        private:
            std::weak_ptr<KcpServer> _server;     // server
            std::weak_ptr<Socket> _udp_channel;   // socket
            std::weak_ptr<KcpListener> _listener; // 监听er
            Address::ptr _remote_addr;            // 远端地址

            uint32_t _convid;          // convid
            IKCPCB *_kcp_cb = nullptr; // kcp控制块

            std::string _in_server_container_id; // 在server管理中的id

            uint64_t _read_timeout_ms;                 // 读超时时间
            std::atomic<bool> _b_read_timeout = false; // 超时

            MutexType _kcpcb_mtx; // 访问kcpcb的互斥锁
            CondType _kcpcb_cond; // 接收kcpcb数据条件变量

            std::list<Message::ptr> _sendque; // 发送队列
            MutexType _sendque_mtx;           // 发送队列锁
            CondType _sendque_cond;           // 发送队列条件变量

            SemType _sem; // 等待写协程退出

            std::once_flag _once_close;         // 一次关闭
            std::atomic<bool> _b_close = false; // 通知关闭

            std::once_flag _once_readError;          // 一次错误
            std::atomic<bool> _b_read_error = false; // 通知读错误
            std::atomic<int> _read_error_code;       // 读错误码

            std::once_flag _once_writeError;          // 一次错误
            std::atomic<bool> _b_write_error = false; // 通知读错误
            std::atomic<int> _write_error_code;       // 读错误码
        };
    } // namespace kcp

} // namespace Xten

#endif