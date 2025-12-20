#include "kcp_listener.h"
#include "../log.h"
#include "../config.h"
#include "../macro.h"
#include "../iomanager.h"
#include "kcp_util.hpp"
#include <ctime>
#include <chrono>
#include <algorithm>
namespace Xten
{
    namespace kcp
    {
        // 最好是把配置全部放到kcpserver中统一管理----todo
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");

        KcpListener::KcpListener(Address::ptr addr, uint32_t maxConnNum, IOManager *iom, int coroutine_num, int nodelay, // 0:disable(default), 1:enable  是否非延迟
                                 int interval,                                                                           // internal update timer interval in millisec, default is 100ms  内部刷新数据间隔时间
                                 int resend,                                                                             // 0:disable fast resend(default), 1:enable fast resend 快速重传次数
                                 int nc, int mtxsize, int sndwnd, int rcvwnd)
            : _accept_timeout_ms(0),
              _coroutine_num(coroutine_num),
              _b_timeout(false),
              _b_close(false),
              _b_read_error(false),
              _read_error_code(0),
              _local_address(addr),
              _max_conn_num(maxConnNum),
              _backlog_cond(_backlog_mtx),
              _nodelay(nodelay),
              _interval(interval),
              _resend(resend),
              _nc(nc),
              _iom(iom),
              _mtxsize(mtxsize),
              _sndwnd(sndwnd),
              _rcvwnd(rcvwnd)
        {
            // 创建udpsocket并设置端口复用
            for (int i = 0; i < _coroutine_num; i++)
            {
                Socket::ptr udp_socket;
                udp_socket = Socket::CreateUDPSocket();
                XTEN_ASSERT(udp_socket);
                int opt = 1;
                XTEN_ASSERT(udp_socket->Setsockopt(SOL_SOCKET, SO_REUSEPORT, opt));
                _udp_sockets.push_back(udp_socket);
            }
            // 随机生成起始convid [10000-100999]
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            _convid = std::rand() % 100000 + 10000;
        }
        KcpListener::~KcpListener()
        {
            // close
            Close();
        }
        // 设置listen状态---开启协程
        bool KcpListener::Listen(int backlog)
        {
            if (backlog > 0)
                _backlog_size = backlog;
            for (int i = 0; i < _coroutine_num; ++i)
            {
                // bind
                if (!_udp_sockets[i]->Bind(_local_address))
                {
                    XTEN_LOG_ERROR(g_logger) << "KcpListener bind fail errno="
                                             << errno << " errstr=" << strerror(errno)
                                             << " addr=[" << _local_address->toString() << "]";
                    return false;
                }
                // 调度recv协程
                _iom->Schedule(std::bind(&KcpListener::doRecvLoop, this, shared_from_this(), _udp_sockets[i]));
                // 调度send协程
                _iom->Schedule(std::bind(&KcpListener::doSendLoop, this, shared_from_this(), _udp_sockets[i]));
            }
            return true;
        }

        // 关闭
        void KcpListener::Close()
        {
            std::call_once(_once_close, [this]()
                           {
                _b_close=true;
                //close all listen udp socket
                for(auto& channel : _udp_sockets)
                {
                    channel->Close();
                }
                _backlog_cond.signal(); });
        }
        // 获取localaddr
        Address::ptr KcpListener::GetLocalAddress() const
        {
            return _local_address;
        }
        // 接受一个新的连接，返回nullptr表示没有新连接
        KcpSession::ptr KcpListener::Accept()
        {
            // 启动超时定时器
            Xten::Timer::ptr timer;
            if (_accept_timeout_ms > 0)
                timer = Xten::IOManager::GetThis()->addTimer(_accept_timeout_ms, std::bind(&KcpListener::notifyTimeout, this));
            {
                MutexType::Lock lock(_backlog_mtx);
                while (_accept_backlog.empty() &&
                       !_b_timeout && !_b_read_error && !_b_close)
                {
                    _backlog_cond.wait();
                }
                // wake up
                do
                {
                    if (_b_timeout)
                    {
                        _b_timeout = false;
                        XTEN_LOG_DEBUG(g_logger) << "Accept time out";
                        break;
                    }
                    else
                    {
                        if (timer)
                            timer->cancel();
                        if (_b_read_error)
                        {
                            XTEN_LOG_DEBUG(g_logger) << "Accept read error [errorcode=" << _read_error_code << "]errstr=["
                                                     << strerror((int)_read_error_code);
                            break;
                        }
                        else if (_b_close)
                        {
                            XTEN_LOG_DEBUG(g_logger) << "Accept error listener close";
                            break;
                        }
                        else
                        {
                            // 有连接到来
                            auto newss = _accept_backlog.front();
                            _accept_backlog.pop_front();
                            return newss;
                        }
                    }
                } while (false);
            }
            return nullptr;
        }
        // 接收数据协程函数
        void KcpListener::doRecvLoop(KcpListener::ptr self, Socket::ptr udpChannel)
        {
            XTEN_LOG_INFO(g_logger) << "KcpListener doRecvLoop fiber begin";
            try
            {
                // 输出缓冲区
                ByteArray::ptr ba = std::make_shared<ByteArray>(1024 + 512); // mtu大小大一点 内存对齐
                // 循环读取
                for (;;)
                {
                    // XTEN_LOG_INFO(g_logger) << "running";
                    ba->SetPosition(0);
                    // // 读取裸数据包 -----> 批量读取提高性能
                    std::vector<iovec> iovs;                                // 缓冲区指针+len
                    ba->GetWriteBuffers(iovs, maxBatchSize * (1024 + 512)); // 拿到batchsize个的缓冲区
                    std::vector<std::pair<Address::ptr, size_t>> infos;
                    infos.resize(maxBatchSize);
                    int ret = udpChannel->RecvFromBatch(iovs, maxBatchSize, infos);
                    // if (ret >= 2)
                    // {
                    //     XTEN_LOG_INFO(g_logger) << "recv,ret=" << ret;
                    //     for (int i = 0; i < ret; i++)
                    //     {
                    //         std::cout << "addr=" << i << infos[i].first->toString() << std::endl;
                    //     }
                    // }
                    // Xten::Address::ptr addr=std::make_shared<IPv4Address>();
                    // int ret=udpChannel->RecvFrom(ba->GetBeginNodePtr(),ba->GetNodeSize(),addr);
                    // 注意：ba里面的读写位置并没有改变
                    if (ret <= 0)
                    {
                        // 错误----通知读错误
                        self->notifyReadError(errno);
                        break;
                    }
                    else
                    {
                        // 将真正返回的一条条数据放入内部处理
                        for (int i = 0; i < ret; i++)
                        {
                            packetInput((const char *)iovs[i].iov_base, infos[i].second, infos[i].first, udpChannel);
                        }
                    }
                }
            }
            catch (...)
            {
                XTEN_LOG_ERROR(g_logger) << "KcpListener doRecvLoop fiber catch exception";
            }
            XTEN_LOG_INFO(g_logger) << "KcpListener doRecvLoop fiber exit";
        }

        // 获取当前时间（毫秒）
        static uint64_t now_ms()
        {
            using namespace std::chrono;
            return (uint64_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        }

        bool KcpListener::isBlacklisted(const std::string &addr)
        {
            MutexType::Lock lock(_blacklist_mtx);
            auto it = _blacklist.find(addr);
            if (it == _blacklist.end())
                return false;
            uint64_t now = now_ms();
            if (it->second.expiry_ms > now)
            {
                return true;
            }
            // 已过期，移除
            _blacklist.erase(it);
            return false;
        }

        void KcpListener::reportInvalidPacket(const std::string &addr)
        {
            uint64_t now = now_ms();
            MutexType::Lock lock(_blacklist_mtx);
            auto &entry = _blacklist[addr];
            entry.expiry_ms = now + 60 * 2 * 1000; // 加入黑名单2min
            XTEN_LOG_WARN(g_logger) << "Blacklisted " << addr << " ttl=2000*2*60" << "ms";
        }

        void KcpListener::cleanupBlacklist()
        {
            MutexType::Lock lock(_blacklist_mtx);
            _blacklist.clear();
        }

        // 发送数据协程函数
        void KcpListener::doSendLoop(KcpListener::ptr self, Socket::ptr udpChannel)
        {
            // 只发送属于当前socket的session的数据
            XTEN_LOG_INFO(g_logger) << "KcpListener doSendLoop fiber begin";
            auto sockfd = udpChannel->GetSockFd();
            try
            {
                while (!_b_close && !_b_read_error)
                {
                    std::unordered_map<std::string, KcpSession::ptr> senders;
                    // 找到要发送的sessions
                    {
                        MutexType::Lock lock(_session_mtx);
                        auto iter = _sessions.find(sockfd);
                        if (iter != _sessions.end())
                            senders = iter->second;
                    }
                    // 对当前的所有sessions副本进行数据发送
                    for (auto &session : senders)
                    {
                        session.second->update();
                    }
                    // 间隔一段时间--每10ms调用一次
                    usleep(1000 * _interval / 2);
                    // XTEN_LOG_DEBUG(g_logger)<<"update"<<TimeUitl::GetCurrentMS();
                }
            }
            catch (...)
            {
                XTEN_LOG_ERROR(g_logger) << "KcpListener doSendLoop fiber catch exception";
            }
            XTEN_LOG_INFO(g_logger) << "KcpListener doSendLoop fiber exit";
        }

        // 原始udp包处理 [data ， fromaddr]
        void KcpListener::packetInput(const char *data, size_t len, Address::ptr from, Socket::ptr udpChannel)
        {
            std::string remote = from->toString();
            if (isBlacklisted(remote))
            {
                // XTEN_LOG_INFO(g_logger) << "Drop packet from blacklisted " << remote;
                // 对方已经在黑名单中了
                return;
            }
            // 1.看连接是否已经存在
            std::pair<bool, KcpSession::ptr> isExisted;
            {
                MutexType::Lock lock(_session_mtx);
                auto iter = _sessions[udpChannel->GetSockFd()].find(from->toString());
                if (iter != _sessions[udpChannel->GetSockFd()].end())
                {
                    isExisted.first = true;
                    isExisted.second = iter->second;
                }
            }
            if (!isExisted.first)
            {
                // 1.判断是不是客户端发来的连接建立包
                if (!KcpUtil::is_connect_packet(data, len))
                {
                    // 解答疑问：kcp服务端[当前]将这个连接关闭了,不再发送响应和其他kcp内部的包给客户端，而客户端认为连接
                    // 仍然建立，会认为是网络问题而重传没有收到响应的包，并且客户端也在不停发包，就会导致服务端频繁得收到
                    // 客户端的包文，因此这个接口会频繁触发------>客户端发送包文数量是远小于kcp内部处理后发送的包的[实际包数量]
                    XTEN_LOG_WARN(g_logger) << "KcpListener recv a invalid kcp connect packet from " << remote;
                    // 把这个连接上报到黑名单计数器
                    reportInvalidPacket(remote);
                    return;
                }
                // 不存在这个连接
                {
                    MutexType::Lock lockq(_backlog_mtx);
                    if (_accept_backlog.size() >= _backlog_size) // 连接队列已经满了
                        return;
                    lockq.unlock();
                    {
                        MutexType::Lock lock(_session_mtx);
                        size_t sz = 0;
                        for (auto &sesss : _sessions)
                        {
                            sz += sesss.second.size();
                        }
                        if (sz >= _max_conn_num) // 服务器的连接数量达到上限
                            return;
                    }
                    // 没满--创建连接
                    // 1.给客户端发送连接响应包文
                    uint32_t local_convid = _convid++;
                    std::string backpacket = KcpUtil::making_connect_backpacket() + std::to_string(local_convid);
                    XTEN_LOG_DEBUG(g_logger) << "from=" << from->toString() << "convid" << local_convid;
                    udpChannel->SendTo(backpacket.c_str(), backpacket.length(), from);
                    // 2.创建并保存连接
                    // KcpSession::ptr newss = std::make_shared<KcpSession>(udpChannel,shared_from_this(),from,local_convid) ; // todo 参数
                    KcpSession::ptr newss = Xten::protected_make_shared<KcpSession>(udpChannel, shared_from_this(), from, local_convid,
                                                                                    _nodelay, _interval, _resend, _nc, _mtxsize, _sndwnd, _rcvwnd);
                    {
                        MutexType::Lock lock(_session_mtx);
                        _sessions[udpChannel->GetSockFd()][from->toString()] = newss;
                    }
                    lockq.lock();
                    _accept_backlog.push_back(newss);
                }
                notifyAccept();
            }
            else
            {
                // exist
                auto session = isExisted.second;
                do
                {
                    if (len < 4)
                    {
                        // len 不足4字节 convid读不到
                        XTEN_LOG_INFO(g_logger) << "" << "len=" << len;
                        break;
                    }
                    uint32_t id = ikcp_getconv(data);
                    if (id == session->GetConvId())
                    {
                        // equal
                        session->packetInput(data, len);
                        return;
                    }
                    else
                    {
                        XTEN_LOG_INFO(g_logger) << "id" << id << "serverid=" << session->GetConvId();
                        std::cout << "from" << from->toString() << std::endl;
                        for (auto &session : _sessions[udpChannel->GetSockFd()])
                        {
                            std::cout << "idkey=" << session.first << "id=" << session.second->GetConvId() << "from=" << session.second->_remote_addr->toString() << std::endl;
                        }
                        break;
                    }
                } while (false);
                session->Close();
            }
        }
        // 给session的关闭函数---->session关闭后通知listener
        bool KcpListener::onSessionClose(const std::string &remote_addr)
        {
            MutexType::Lock lock(_session_mtx);
            for (auto &sessions : _sessions)
            {
                auto iter = sessions.second.find(remote_addr);
                if (iter != sessions.second.end())
                {
                    sessions.second.erase(iter);
                    return true;
                }
            }
            return false;
        }
        // 通知超时
        void KcpListener::notifyTimeout()
        {
            _b_timeout = true;
            _backlog_cond.signal(); // signal
        }
        // 通知accept
        void KcpListener::notifyAccept()
        {
            _backlog_cond.signal();
        }
        // 通知读错误
        void KcpListener::notifyReadError(int code)
        {
            std::call_once(_once_readError, [this, code]()
                           {
                            _read_error_code=code;
                            _b_read_error=true;
                            //notify all session that udp socket is read error
                            {
                             MutexType::Lock lock(_session_mtx) ;
                            for(auto& sessions:_sessions){
                            for(auto& session:sessions.second)
                            {
                                session.second->notifyReadError(code);
                            }
                            }}
                            _backlog_cond.signal(); });
        }
    } // namespace kcp

} // namespace Xten
