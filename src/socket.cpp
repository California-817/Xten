#include "../include/socket.h"
#include "../include/log.h"
#include "../include/macro.h"
#include "../include/fdmanager.h"
#include "../include/hook.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    Socket::Socket(Socket::FAMILY family, Socket::TYPE type, int protocol)
        : _sockfd(-1),
          _family(family),
          _type(type),
          _protocol(protocol),
          _isConnect(false)
    {
    }
    // bind绑定
    bool Socket::Bind(Address::ptr addr)
    {
        if (!isValid())
        {
            // sockfd未设置--创建socket
            newSocket();
            if (XTEN_UNLIKELY(!isValid()))
            {
                return false;
            }
        }
        if (XTEN_UNLIKELY(addr->getFamily() != _family))
        {
            XTEN_LOG_ERROR(g_logger) << "bind sock.family("
                                     << _family << ") addr.family(" << addr->getFamily()
                                     << ") not equal, addr=" << addr->toString();
            return false;
        }
        // 看是否是unix地址
        UnixAddress::ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
        if (uaddr)
        {
            // 转化成功说明是unix地址
            // todo
        }
        // 普通ip地址
        int ret = ::bind(_sockfd, addr->getAddr(), addr->getAddrLen());
        if (ret)
        {
            XTEN_LOG_ERROR(g_logger) << "bind error errrno=" << errno
                                     << " errstr=" << strerror(errno);
            return false;
        }
        // 绑定成功 获取本地address
        GetLocalAddress();
        return true;
    }
    // 设置监听
    bool Socket::Listen(int size)
    {
        if (!isValid())
        {
            return false;
        }
        int ret = ::listen(_sockfd, size);
        if (ret == -1)
        {
            XTEN_LOG_ERROR(g_logger) << "listen error errno=" << errno
                                     << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }
    // connect发起连接
    bool Socket::Connect(Address::ptr addr, uint64_t timeout)
    {
        _peerAddress = addr;
        if (!isValid())
        {
            newSocket();
            if (XTEN_UNLIKELY(!isValid()))
            {
                return false;
            }
        }
        // 创建socket成功
        if (XTEN_UNLIKELY(addr->getFamily() != _family))
        {
            XTEN_LOG_ERROR(g_logger) << "connect sock.family("
                                     << _family << ") addr.family(" << addr->getFamily()
                                     << ") not equal, addr=" << addr->toString();
            return false;
        }
        if (timeout == (uint64_t)-1)
        {
            // 未设置超时时间
            int ret1 = ::connect(_sockfd, addr->getAddr(), addr->getAddrLen());
            if (ret1)
            {
                XTEN_LOG_ERROR(g_logger) << "sock=" << _sockfd << " connect(" << addr->toString()
                                         << ") error errno=" << errno << " errstr=" << strerror(errno);
                Close();
                return false;
            }
        }
        else
        {
            int ret2 = ::connect_with_timeout(_sockfd, addr->getAddr(), addr->getAddrLen(), timeout);
            if (ret2)
            {
                XTEN_LOG_ERROR(g_logger) << "sock=" << _sockfd << " connect(" << addr->toString()
                                         << ") error errno=" << errno << " errstr=" << strerror(errno);
                Close();
                return false;
            }
        }
        // 连接建立成功 ---获取本地address和远端address
        _isConnect = true;
        GetLocalAddress();
        GetPeerAddress();
        return true;
    }
    // 检查是否连接---系统层面检查
    bool Socket::CheckConnected()
    {
        if (!isValid())
        {
            return false;
        }
        struct tcp_info tcpinfo;
        socklen_t len = sizeof tcpinfo;
        ::getsockopt(_sockfd, IPPROTO_TCP, TCP_INFO, (void *)&tcpinfo, &len);
        _isConnect = (tcpinfo.tcpi_state == TCP_ESTABLISHED);
        return _isConnect;
    }
    // 返回是否连接
    bool Socket::IsConnected()
    {
        return _isConnect;
    }
    // 关闭socket连接
    bool Socket::Close()
    {
        if (!_isConnect && _sockfd == -1)
        {
            return true;
        }
        _isConnect = false;
        if (_sockfd != -1)
        {
            // 不会多次关闭
            ::close(_sockfd);
            _sockfd = -1;
        }
        return true;
    }
    // 接收连接
    std::shared_ptr<Socket> Socket::Accept()
    {
        Socket::ptr newsocket = std::make_shared<Socket>(_family, _type);
        if (XTEN_UNLIKELY(!isValid()))
        {
            return nullptr;
        }
        // 系统创建好socket结构并返回socketfd
        int newsockfd = ::accept(_sockfd, nullptr, nullptr);
        if (newsockfd < 0)
        {
            XTEN_LOG_ERROR(g_logger) << "accept(" << _sockfd << ") errno="
                                     << errno << " errstr=" << strerror(errno);
            return nullptr;
        }
        if (newsocket->init(newsockfd))
        {
            return newsocket;
        }
        return nullptr;
    }
    // 获取本地address
    Address::ptr Socket::GetLocalAddress()
    {
        if (_localAddress)
        {
            return _localAddress;
        }
    }
    // 获取对端address
    Address::ptr Socket::GetPeerAddress()
    {
    }
    Socket::~Socket()
    {
        Close();
    }
    // 通过sockfd进行初始化
    bool Socket::init(int sockfd)
    {
        FdCtx::ptr fdctx = FdCtxMgr::GetInstance()->Get(sockfd);
        if (fdctx && fdctx->IsSocket() && !fdctx->IsClose())
        {
            _sockfd = sockfd;
            _isConnect = true;
            initSocket();
            GetLocalAddress();
            GetPeerAddress();
            return true;
        }
        return false;
    }
    bool Socket::newSocket()
    {
        int sockfd = ::socket(_family, _type, _protocol);
        if (XTEN_UNLIKELY(sockfd < 0))
        {
            // failed
            XTEN_LOG_ERROR(g_logger) << "socket(" << _family
                                     << ", " << _type << ", " << _protocol << ") errno="
                                     << errno << " errstr=" << strerror(errno);
        }
        _sockfd = sockfd;
        initSocket();
        return true;
    }
    bool Socket::Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        int ret = ::setsockopt(sockfd, level, optname, optval, optlen);
        if (ret != 0)
        {
            XTEN_LOG_DEBUG(g_logger) << "setOption sock=" << _sockfd
                                     << " level=" << level << " option=" << optname
                                     << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }
    bool Socket::Getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
    {
        int ret = ::getsockopt(sockfd, level, optname, optval, optlen);
        if (ret != 0)
        {
            XTEN_LOG_DEBUG(g_logger) << "getOption sock=" << _sockfd
                                     << " level=" << level << " option=" << optname
                                     << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        return true;
    }
    bool Socket::initSocket()
    {
        if (!isValid())
        {
            return false;
        }
        // 设置端口复用
        int val = 0;
        bool ret = Setsockopt(SOL_SOCKET, SO_REUSEADDR, val);
        // tcp --->设置禁用Nagle算法 降低延迟
        if (_type == TYPE::TCP)
        {
            ret &= Setsockopt(IPPROTO_TCP, TCP_NODELAY, val);
        }
        return ret;
    }
    bool Socket::isValid()
    {
        return _sockfd != -1;
    }
}