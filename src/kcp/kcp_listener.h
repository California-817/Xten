#ifndef __XTEN_KCP_LISTENER_H__
#define __XTEN_KCP_LISTENER_H__
#include <memory>
#include <unordered_map>
#include <list>
#include "kcp_session.h"
#include "third_part/ikcp.h"
#include "../socket.h"
#include "../mutex.h"
#include "../bytearray.h"
#include <atomic>
#include <thread>
namespace Xten
{
    namespace kcp
    {
        enum
        {
            // maximum packet size
            mtuLimit = 1500,
            // batch size
            maxBatchSize = 256,
        };
        class KcpListener : public std::enable_shared_from_this<KcpListener>
        {
        public:
            friend class KcpSession;

            typedef std::shared_ptr<KcpListener> ptr;
            typedef FiberMutex MutexType;
            typedef FiberCondition ConditionType;
            typedef FiberSemphore SemType;

            // 工厂方法创建listener
            static std::shared_ptr<KcpListener> CreateKcpListener(Address::ptr addr)
            {
                return std::shared_ptr<KcpListener>(new KcpListener(addr));
            }

            KcpListener(Address::ptr addr);
            ~KcpListener();
            // 设置listen状态---开启协程
            bool Listen(int backlog = 128);
            // 接受一个新的连接，返回nullptr表示没有新连接
            KcpSession::ptr Accept();
            // 设置accept超时时间
            void SetAcceptTimeout(uint64_t v) { _accept_timeout_ms = v; }
            // 获取accept超时时间
            uint64_t GetAcceptTimeout() const { return _accept_timeout_ms; }
            // 关闭
            void Close();
            // 获取localaddr
            Address::ptr GetLocalAddress() const;

        private:
            // 接收数据协程函数
            void doRecvLoop(KcpListener::ptr self, Socket::ptr udpChannel);
            // 原始udp包处理 [data ， fromaddr]
            void packetInput(const char *data, size_t len, Address::ptr from, Socket::ptr udpChannel);
            // 给session的关闭函数
            bool onSessionClose(const std::string &remote_addr);
            // 通知超时
            void notifyTimeout();
            // 通知accept
            void notifyAccept();
            // 通知读错误
            void notifyReadError(int code);

        private:
            std::atomic<uint32_t> _convid; // convid

            Address::ptr _local_address;           // 监听的udp socket
            std::vector<Socket::ptr> _udp_sockets; // udp套接字

            int _backlog_size;                                          // 全连接队列大小
            std::list<KcpSession::ptr> _accept_backlog;                 // 已经accept但还没有被用户拿走的session
            MutexType _backlog_mtx;                                     // 互斥锁
            ConditionType _backlog_cond;                                // 条件变量
            std::unordered_map<std::string, KcpSession::ptr> _sessions; // 这个listener管理的sessions
            MutexType _session_mtx;                                     // 互斥锁

            uint64_t _accept_timeout_ms; // accept超时时间

            std::once_flag _once_close;         // 一次关闭
            std::atomic<bool> _b_close = false; // 通知关闭

            std::once_flag _once_readError;          // 一次错误
            std::atomic<bool> _b_read_error = false; // 通知读错误

            SemType _wait_fiber;     // 等待所有协程退出
            uint32_t _coroutine_num; // 读协程数量

            // notify variables
            std::atomic<bool> _b_timeout = false; // 超时

            // read error msg
            std::atomic<int> _read_error_code;
        };
    } // namespace kcp

} // namespace Xten

#endif