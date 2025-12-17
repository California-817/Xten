#include "kcp_server.h"
#include "log.h"
namespace Xten
{
    namespace kcp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        KcpServer::KcpServer(IOManager *io_worker,
                             KcpConfig::ptr config)
            : _io_worker(io_worker),
              _isStop(true),
              m_kcpConfig(config),
              _recvTimeout(2000 * 60) // 默认2min
        {
            if (config)
            {
                // _recvTimeout = config->_port;
            }
        }
        bool KcpServer::Bind(Address::ptr addr)
        {
            // udp绑定
            // auto num = m_kcpConfig->_read_coroutine_num;
            int opt = 1;
            int num=1;
            for (uint16_t i = 0; i < num; i++)
            {
                Socket::ptr udp_socket = Socket::CreateUDPSocket();
                // 设置port复用
                udp_socket->Setsockopt(SOL_SOCKET, SO_REUSEPORT, opt);
                // bind
                if (!udp_socket->Bind(addr))
                {
                    XTEN_LOG_ERROR(g_logger) << "KcpServer bind fail errno="
                                             << errno << " errstr=" << strerror(errno)
                                             << " addr=[" << addr->toString() << "]";
                    return false;
                }
                // success
                _udpSockets.push_back(udp_socket);
            }
            return true;
        }

        // 启动若干协程进行io操作
        void KcpServer::Start()
        {
            if (_isStop == false)
                return; // running
            XTEN_ASSERT(_io_worker);
            auto self = shared_from_this();
            for (auto &udp_channel : _udpSockets)
            {
                // 调度读协程
                _io_worker->Schedule(std::bind(&KcpServer::doRead, this, udp_channel, self));
            }
            _isStop = false;
        }

        void KcpServer::doRead(Socket::ptr udp_socket, KcpServer::ptr self)
        {
            XTEN_LOG_INFO(g_logger) << "KcpServer doRead start! udp_socket=" << *udp_socket;
            while (!_isStop)
            {
                //1. udpsocket中读取报文
                // udp_socket->RecvFromV()
                //2.判断包文类型
                //3.1创建新连接
                //3.2交给逻辑层处理
            }
            XTEN_LOG_INFO(g_logger) << "KcpServer doRead end! udp_socket=" << *udp_socket;
        }
    }
}