#ifndef __XTEN_KCP_SERVER_H__
#define __XTEN_KCP_SERVER_H__
#include <memory>
#include <functional>
#include "../nocopyable.hpp"
#include "../socket.h"
#include "kcp_session.h"
#include "../iomanager.h"
#include "../msghandle.h"
#include "kcp_listener.h"
#include "third_part/ikcp.h"
#include "../address.h"
namespace Xten
{
    namespace kcp
    {
        struct KcpConfig // 配置属性
        {
            /* data */
            typedef std::shared_ptr<KcpConfig> ptr;
            std::string type = "kcp";          // 服务器类型
            std::string name = "Xten/kcp/1.0"; // name
            std::string address;               // 地址
            uint16_t internal_coroutine_num;   // 协程数量
            uint64_t accept_timeout_ms;        // accept超时时间
            int listen_backlog_size;           // 连接队列长度
            uint32_t max_conn_num;             // 服务器最大连接数量
            uint64_t recv_timeout_ms;          // 超时时间
            bool timewheel;                    // 是否创建一个时间轮---[游戏业务逻辑定时器]
            std::string ioworker;              // io操作的调度器
            std::string msghanler;             // 消息处理系统

            // kcp配置 fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
            int nodelay;  // 0:disable(default), 1:enable  是否非延迟
            int interval; // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
            int resend;   // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
            int nc;       // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制
        };

        class KcpServer : public NoCopyable, public std::enable_shared_from_this<KcpServer>
        {
        public:
            typedef std::shared_ptr<KcpServer> ptr;
            typedef std::function<uint32_t(KcpSession::ptr)> onClientNoActiveCb;
            typedef std::function<uint32_t(KcpSession::ptr)> onConnectCb;
            typedef std::function<uint32_t(KcpSession::ptr)> onCloseCb;
            KcpServer(MsgHandler::ptr msghandler,IOManager *io_worker = IOManager::GetThis(),
                      KcpConfig::ptr config = nullptr);
            ~KcpServer();
            // bind
            bool Bind(Address::ptr addr);
            // 启动若干协程进行io操作
            void Start();
            // 停止服务器
            void Stop();

            void SetOnClientNoActiveCb(onClientNoActiveCb cb) { _timeoutCb = cb; }
            onClientNoActiveCb GetOnClientNoActiveCb() const { return _timeoutCb; }

            void SetOnConnectCb(onConnectCb cb) { _connectCb = cb; }
            onConnectCb GetOnConnectCb() const { return _connectCb; }

            void SetOnCloseCb(onCloseCb cb) { _closeCb = cb; }
            onCloseCb GetOnCloseCb() const { return _closeCb; }

            uint64_t GetRecvTimeout() const { return _recvTimeout; }

        private:
        private:
            IOManager *_io_worker; // io worker
            // handler --从配置选项中获取
            MsgHandler::ptr _msgHandler;
            bool _isStop;
            KcpConfig::ptr m_kcpConfig;           // 配置属性
            std::vector<Socket::ptr> _udpSockets; // udp套接字---每个读协程都有一个socket【即使在同一个端口】

            KcpListener::ptr _listener; // 监听套接字

            // cbs
            onClientNoActiveCb _timeoutCb; // 客户端无活动回调
            onConnectCb _connectCb;        // 连接建立回调
            onCloseCb _closeCb;            // 连接关闭回调

            uint64_t _recvTimeout; // 接收超时时间

            uint32_t _maxConnNum; // 最大连接数量

            KcpConfig::ptr _config; // config
        };
    } // namespace kcp

} // namespace Xten

#endif