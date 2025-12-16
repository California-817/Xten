#include "kcp_listener.h"
#include "../log.h"
#include "../config.h"
#include "../macro.h"
#include "../iomanager.h"
#include "kcp_util.hpp"
namespace Xten
{
    namespace kcp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        static Xten::ConfigVar<uint64_t>::ptr g_kcp_accept_timeout =
            Xten::Config::LookUp<uint64_t>("kcp.listener.accept_timeout", 2000, "kcp accept timeout ms");
        static Xten::ConfigVar<uint32_t>::ptr g_kcp_fiber_num =
            Xten::Config::LookUp<uint32_t>("kcp.listener.internal_fibernum", 10, "kcp internal fiber num");

        KcpListener::KcpListener(Address::ptr addr)
            : _accept_timeout_ms(g_kcp_accept_timeout->GetValue()),
              _wait_fiber(g_kcp_fiber_num->GetValue()),
              _coroutine_num(g_kcp_fiber_num->GetValue()),
              _b_timeout(false),
              _b_close(false),
              _b_read_error(false),
              _read_error_code(0),
              _local_address(addr),
              _backlog_cond(_backlog_mtx)
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
            // 随机生成起始convid [0-9999]
            _convid = std::rand() % 10000;
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
            auto iom = Xten::IOManager::GetThis();
            XTEN_ASSERT(iom);
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
                iom->Schedule(std::bind(&KcpListener::doRecvLoop, this, shared_from_this(), _udp_sockets[i]));
                // todo 可能还要加上send协程
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
            // sem--
            _wait_fiber.wait();
            try
            {
                // 输出缓冲区
                ByteArray::ptr ba = std::make_shared<ByteArray>(1024 + 512); // mtu大小大一点 内存对齐
                // 循环读取
                for (;;)
                {
                    ba->SetPosition(0);
                    // 读取裸数据包 -----> 批量读取提高性能
                    std::vector<iovec> iovs;                                // 缓冲区指针+len
                    ba->GetWriteBuffers(iovs, maxBatchSize * (1024 + 512)); // 拿到batchsize个的缓冲区
                    std::vector<std::pair<Address::ptr, size_t>> infos;
                    infos.resize(maxBatchSize, std::make_pair(std::make_shared<IPv4Address>(), 0));
                    int ret = udpChannel->RecvFromBatch(iovs, maxBatchSize, infos);
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
            _wait_fiber.post();
            XTEN_LOG_INFO(g_logger) << "KcpListener doRecvLoop fiber exit";
        }

        // 原始udp包处理 [data ， fromaddr]
        void KcpListener::packetInput(const char *data, size_t len, Address::ptr from, Socket::ptr udpChannel)
        {
            // 1.看连接是否已经存在
            std::pair<bool, KcpSession::ptr> isExisted;
            {
                MutexType::Lock lock(_session_mtx);
                auto iter = _sessions.find(from->toString());
                if (iter != _sessions.end())
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
                    XTEN_LOG_WARN(g_logger) << "KcpListener recv a invalid kcp connect packet";
                    return;
                }
                // 不存在这个连接
                {
                    MutexType::Lock lockq(_backlog_mtx);
                    if (_accept_backlog.size() >= _backlog_size) // 连接队列已经满了
                        return;
                    lockq.unlock();
                    // 没满--创建连接
                    // 1.给客户端发送连接响应包文
                    uint32_t local_convid=_convid++;
                    std::string backpacket = KcpUtil::making_connect_backpacket() + std::to_string(local_convid);
                    udpChannel->SendTo(backpacket.c_str(), backpacket.length(), from);
                    // 2.创建并保存连接
                    KcpSession::ptr newss = std::make_shared<KcpSession>(); // todo 参数
                    {
                        MutexType::Lock lock(_session_mtx);
                        _sessions[from->toString()] = newss;
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
                        break;
                    }
                    uint32_t id = ikcp_getconv(data);
                    if (id == session->GetConvId())
                    {
                        // equal
                        session->packetInput(data, len);
                        return;
                    }
                } while (false);
                session->Close();
            }
        }
        // 给session的关闭函数---->session关闭后通知listener
        bool KcpListener::onSessionClose(const std::string &remote_addr)
        {
            MutexType::Lock lock(_session_mtx);
            auto iter = _sessions.find(remote_addr);
            if (iter != _sessions.end())
            {
                _sessions.erase(iter);
                return true;
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
                            for(auto& session:_sessions)
                            {
                                session.second->notifyReadError(code);
                            }
                            }
                            _backlog_cond.signal(); });
        }
    } // namespace kcp

} // namespace Xten
