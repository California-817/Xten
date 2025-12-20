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
#include <chrono>
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
        private:
            KcpListener(Address::ptr addr, uint32_t maxConnNum, IOManager *iom, int coroutine_num, int nodelay, // 0:disable(default), 1:enable  是否非延迟
                        int interval,                                                                           // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
                        int resend,                                                                             // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
                        int nc, int mtusize, int sndwnd, int rcvwnd);                                           // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制
        public:
            friend class KcpSession;
            friend class KcpServer;
            typedef std::shared_ptr<KcpListener> ptr;
            typedef FiberMutex MutexType;
            typedef FiberCondition ConditionType;
            typedef FiberSemphore SemType;

            // 工厂方法创建listener
            static std::shared_ptr<KcpListener> Create(Address::ptr addr, uint32_t maxConnNum, IOManager *iom = IOManager::GetThis(), int coroutine_num = 10,
                                                       int nodelay = 1, int interval = 20, int resend = 2, int nc = 1, int mtusize = 1400, int sndwnd = 32, int rcvwnd = 32)
            {
                return std::shared_ptr<KcpListener>(new KcpListener(addr, maxConnNum, iom, coroutine_num, nodelay, interval, resend, nc, mtusize, sndwnd, rcvwnd));
            }

            ~KcpListener();
            // 设置listen状态---开启内部协程
            bool Listen(int backlog = 128);
            // 接受一个新的连接，返回nullptr表示没有新连接
            KcpSession::ptr Accept();
            // 设置accept超时时间[0表示不超时]
            void SetAcceptTimeout(uint64_t v_ms = 0) { _accept_timeout_ms = v_ms; }
            // 获取accept超时时间
            uint64_t GetAcceptTimeout() const { return _accept_timeout_ms; }
            // 关闭
            void Close();
            // 获取localaddr
            Address::ptr GetLocalAddress() const;

            // 信息
            std::string ListenerInfo() const
            {
                std::stringstream ss;
                if (_sessions.empty())
                {
                    ss << "[KcpServer No Connection]";
                    return ss.str();
                }
                ss<<"\n======KcpConnectionsInfo======\n";
                for (auto &sess : _sessions)
                {
                    ss << "[fd=" << sess.first << "]==>{ ";
                    for (auto &e : sess.second)
                    {
                        ss << "[convid=" << e.second->GetConvId() << "] ";
                    }
                    ss << "}\n";
                }
                ss << "=======BlackListInfo=======\n{ ";
                for (auto &entry : _blacklist)
                {
                    ss << "[ip=" << entry << "] ";
                }
                ss << "}\n";
                return ss.str();
            }

        private:
            // 接收数据协程函数
            void doRecvLoop(KcpListener::ptr self, Socket::ptr udpChannel);
            // 发送数据协程函数
            void doSendLoop(KcpListener::ptr self, Socket::ptr udpChannel);
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

            // 黑名单方法
            bool isBlacklisted(const std::string &addr);
            void reportInvalidPacket(const std::string &addr);
            void cleanupBlacklist();

        private:
            // kcp config
            int _nodelay;
            int _interval;
            int _resend;
            int _nc;
            int _mtxsize;
            int _sndwnd;
            int _rcvwnd;

            IOManager *_iom;

            std::atomic<uint32_t> _convid;         // convid
            uint32_t _max_conn_num;                // 服务器最大连接数量
            Address::ptr _local_address;           // 监听的udp socket
            std::vector<Socket::ptr> _udp_sockets; // udp套接字

            int _backlog_size;                                                                   // 全连接队列大小
            std::list<KcpSession::ptr> _accept_backlog;                                          // 已经accept但还没有被用户拿走的session
            MutexType _backlog_mtx;                                                              // 互斥锁
            ConditionType _backlog_cond;                                                         // 条件变量
            std::unordered_map<int, std::unordered_map<std::string, KcpSession::ptr>> _sessions; // 这个listener管理的sessions
            MutexType _session_mtx;                                                              // 互斥锁

            uint64_t _accept_timeout_ms; // accept超时时间

            std::once_flag _once_close;         // 一次关闭
            std::atomic<bool> _b_close = false; // 通知关闭

            std::once_flag _once_readError;          // 一次错误
            std::atomic<bool> _b_read_error = false; // 通知读错误

            uint32_t _coroutine_num; // 读写协程数量

            // notify variables
            std::atomic<bool> _b_timeout = false; // 超时

            // read error msg
            std::atomic<int> _read_error_code;

            // 黑名单
            std::unordered_set<std::string> _blacklist; // key: remote addr string
            MutexType _blacklist_mtx;

            // 黑名单策略参数（可调整）
            uint32_t _blacklist_threshold = 5;                 // 触发封禁的非法包计数阈值
            uint64_t _blacklist_base_ttl_ms = 60 * 1000;       // 初始封禁时长 60s
            uint64_t _blacklist_max_ttl_ms = 24 * 3600 * 1000; // 最大封禁时长 24h
        };
    } // namespace kcp

} // namespace Xten

#endif