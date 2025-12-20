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
#include "../config.h"
namespace Xten
{
    struct KcpServerConfig // 配置属性
    {
        /* data */
        typedef std::shared_ptr<KcpServerConfig> ptr;
        std::string type = "kcp";          // 服务器类型
        std::string name = "Xten/kcp/1.0"; // name
        std::string address;               // 地址

        uint16_t internal_coroutine_num = 10; // 协程数量
        uint64_t accept_timeout_ms = 0;       // accept超时时间
        int listen_backlog_size = 128;        // 连接队列长度

        uint32_t max_conn_num = 10000;            // 服务器最大连接数量
        uint64_t recv_timeout_ms = 2 * 60 * 1000; // 读超时时间

        bool timewheel = 0;    // 是否创建一个时间轮---[游戏业务逻辑定时器]
        std::string ioworker;  // io操作的调度器
        std::string msghanler; // 消息处理系统

        // kcp配置 fastest: ikcp_nodelay(kcp, 1, 20, 2, 1)
        int nodelay = 1;   // 0:disable(default), 1:enable  是否非延迟
        int interval = 20; // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
        int resend = 2;    // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
        int nc = 1;        // 0:normal congestion control(default), 1:disable congestion control 取消拥塞控制
        int mtu_size = 1400;
        int sndwnd = 32;
        int rcvwnd = 32;
        bool IsVaild() const
        {
            return !address.empty();
        }
        bool operator==(const KcpServerConfig &oth) const
        {
            return address == oth.address &&
                   recv_timeout_ms == oth.recv_timeout_ms &&
                   timewheel == oth.timewheel &&
                   type == oth.type &&
                   name == oth.name &&
                   ioworker == oth.ioworker &&
                   internal_coroutine_num == oth.internal_coroutine_num &&
                   accept_timeout_ms == oth.accept_timeout_ms &&
                   listen_backlog_size == oth.listen_backlog_size &&
                   max_conn_num == oth.max_conn_num &&
                   msghanler == oth.msghanler &&
                   nodelay == oth.nodelay &&
                   interval == oth.interval &&
                   resend == oth.resend &&
                   nc == oth.nc &&
                   mtu_size == oth.mtu_size &&
                   sndwnd == oth.sndwnd &&
                   rcvwnd == oth.rcvwnd;
        }
    };
    // 模板类全特化
    template <>
    class lexicalCast<std::string, KcpServerConfig>
    {
    public:
        KcpServerConfig operator()(const std::string &str)
        {

            YAML::Node node = YAML::Load(str);
            KcpServerConfig conf;
            conf.type = node["type"].as<std::string>(conf.type);
            conf.name = node["name"].as<std::string>(conf.name);
            conf.address = node["address"].as<std::string>();
            conf.internal_coroutine_num = node["internal_coroutine_num"].as<uint16_t>(conf.internal_coroutine_num);
            conf.accept_timeout_ms = node["accept_timeout_ms"].as<uint64_t>(conf.accept_timeout_ms);
            conf.listen_backlog_size = node["listen_backlog_size"].as<int>(conf.listen_backlog_size);
            conf.max_conn_num = node["max_conn_num"].as<uint32_t>(conf.max_conn_num);
            conf.recv_timeout_ms = node["recv_timeout_ms"].as<uint64_t>(conf.recv_timeout_ms);
            conf.timewheel = node["timewheel"].as<int>(conf.timewheel);
            conf.ioworker = node["io_worker"].as<std::string>();
            conf.msghanler = node["msghanler"].as<std::string>();

            conf.nodelay = node["nodelay"].as<int>(conf.nodelay);
            conf.interval = node["interval"].as<int>(conf.interval);
            conf.resend = node["resend"].as<int>(conf.resend);
            conf.nc = node["nc"].as<int>(conf.nc);
            conf.mtu_size = node["mtu_size"].as<int>(conf.mtu_size);
            conf.sndwnd = node["sndwnd"].as<int>(conf.sndwnd);
            conf.rcvwnd = node["rcvwnd"].as<int>(conf.rcvwnd);
            return conf;
        }
    };
    template <>
    class lexicalCast<KcpServerConfig, std::string>
    {
    public:
        std::string operator()(const KcpServerConfig &conf)
        {
            YAML::Node node;
            node["type"] = conf.type;
            node["name"] = conf.name;
            node["address"] = conf.address;
            node["internal_coroutine_num"] = conf.internal_coroutine_num;
            node["accept_timeout_ms"] = conf.accept_timeout_ms;
            node["listen_backlog_size"] = conf.listen_backlog_size;
            node["max_conn_num"] = conf.max_conn_num;
            node["recv_timeout_ms"] = conf.recv_timeout_ms;
            node["timewheel"] = conf.timewheel;
            node["io_worker"] = conf.ioworker;
            node["msghanler"] = conf.msghanler;

            node["nodelay"] = conf.nodelay;
            node["interval"] = conf.interval;
            node["resend"] = conf.resend;
            node["nc"] = conf.nc;
            node["mtu_size"] = conf.mtu_size;
            node["sndwnd"] = conf.sndwnd;
            node["rcvwnd"] = conf.rcvwnd;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    namespace kcp
    {
        // 连接管理
        class kcp_sessions_container :public NoCopyable
        {
        public:
            typedef FiberMutex MutexType;
            kcp_sessions_container() = default;
            ~kcp_sessions_container() = default;
            void add(const std::string &key, KcpSession::ptr session)
            {
                MutexType::Lock lock(_mtx);
                _connsMap[key] = session;
            }
            bool remove(const std::string &key)
            {
                MutexType::Lock lock(_mtx);
                auto iter = _connsMap.find(key);
                if (iter != _connsMap.end())
                {
                    _connsMap.erase(iter);
                    return true;
                }
                return false;
            }
            KcpSession::ptr get(const std::string &key)
            {
                MutexType::Lock lock(_mtx);
                auto iter = _connsMap.find(key);
                if (iter != _connsMap.end())
                {
                    return iter->second;
                }
                return nullptr;
            }
            void sendmsg(const std::string &key, Message::ptr msg)
            {
                MutexType::Lock lock(_mtx);
                auto iter = _connsMap.find(key);
                if (iter != _connsMap.end())
                {
                    auto temp = iter->second;
                    lock.unlock();
                    temp->SendMessage(msg);
                }
            }
            void broadcastmsg(Message::ptr msg)
            {
                std::unordered_map<std::string, KcpSession::ptr> temp;
                {
                    MutexType::Lock lock(_mtx);
                    temp = _connsMap;
                }
                for (auto &session : temp)
                {
                    session.second->SendMessage(msg);
                }
            }

        private:
            std::unordered_map<std::string, KcpSession::ptr> _connsMap;
            MutexType _mtx;
        };
        class KcpServer : public NoCopyable, public std::enable_shared_from_this<KcpServer>
        {
        public:
            typedef std::shared_ptr<KcpServer> ptr;
            typedef std::function<uint32_t(KcpSession::ptr)> onClientNoActiveCb;
            typedef std::function<uint32_t(KcpSession::ptr)> onConnectCb;
            typedef std::function<uint32_t(KcpSession::ptr)> onCloseCb;
            KcpServer(MsgHandler::ptr msghandler, IOManager *io_worker = IOManager::GetThis(),
                      KcpServerConfig::ptr config = nullptr);
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

            TimerWheelManager::ptr GetTimeWheel() const { return _timewheel; }

            uint64_t GetRecvTimeout() const { return _recvTimeout; }
            uint32_t GetMaxConnNum() const { return _maxConnNum; }

            //返回连接管理容器
            kcp_sessions_container& GetConnsMap(){return _connsMap;}
        protected:
            const char *formSessionId(const KcpSession::ptr &session);
            virtual void startAccept(std::shared_ptr<KcpServer> self);
            virtual void handleClient(std::shared_ptr<KcpServer> self, KcpSession::ptr client);

        private:
            IOManager *_io_worker; // io worker
            // handler --从配置选项中获取
            MsgHandler::ptr _msgHandler;
            std::atomic<bool> _isStop;
            KcpServerConfig::ptr _kcpConfig; // 配置属性

            kcp_sessions_container _connsMap; //连接管理

            KcpListener::ptr _listener; // 监听套接字
            // id
            //  cbs
            onClientNoActiveCb _timeoutCb; // 客户端无活动回调
            onConnectCb _connectCb;        // 连接建立回调
            onCloseCb _closeCb;            // 连接关闭回调

            uint64_t _recvTimeout; // 接收超时时间

            uint32_t _maxConnNum; // 最大连接数量

            uint32_t _coroutine_num; // 协程数量

            // timewheel
            TimerWheelManager::ptr _timewheel; // 时间轮

            std::string _name;
            std::atomic_uint64_t _sn=0;
        };
    } // namespace kcp

} // namespace Xten

#endif