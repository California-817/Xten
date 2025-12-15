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
        }
        KcpListener::~KcpListener() {}
        // 设置listen状态---开启协程
        bool KcpListener::Listen()
        {
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
                auto iom = Xten::IOManager::GetThis();
                XTEN_ASSERT(iom);
                // 调度recv协程
                iom->Schedule(std::bind(&KcpListener::doRecvLoop, this, shared_from_this(), _udp_sockets[i]));
                // todo 可能还要加上send协程
            }
            return true;
        }
        // 接受一个新的连接，返回nullptr表示没有新连接
        KcpSession::ptr KcpListener::Accept() {}
        // 关闭
        void KcpListener::Close() {}
        // 获取localaddr
        Address::ptr KcpListener::GetLocalAddress() const
        {
            return _local_address;
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
                    infos.resize(maxBatchSize, std::make_pair(std::make_shared<Address>(), 0));
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
                            packetInput((const char *)iovs[i].iov_base, infos[i].second, infos[i].first);
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
        void KcpListener::packetInput(const char *data, size_t len, Address::ptr from)
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
            if(!isExisted.first)
            {
                //1.判断是不是客户端发来的连接建立包
                if(!is_connect_packet(data,len)) 
                {
                    XTEN_LOG_WARN(g_logger) << "KcpListener recv a invalid kcp connect packet";
                    return;
                }
                //不存在这个连接
                MutexType::Lock lockq(_backlog_mtx);
                if(_accept_backlog.size()>=acceptBacklog) //连接队列已经满了
                    return;
                lockq.unlock();
                //没满--创建连接

            }
        }
        // 给session的关闭函数
        void KcpListener::onSessionClose(KcpSession::ptr session) {}
        // 通知超时
        void KcpListener::notifyTimeout() {}
        // 通知close
        void KcpListener::notifyClose() {}
        // 通知accept
        void KcpListener::notifyAccept() {}
        // 通知读错误
        void KcpListener::notifyReadError(int code) {}
    } // namespace kcp

} // namespace Xten
