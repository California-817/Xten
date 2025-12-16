#include "socket.h"
#include "log.h"
#include "macro.h"
#include "fdmanager.h"
#include "iomanager.h"
#include "hook.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    Socket::ptr Socket::CreateTCP(Address::ptr addr)
    {
        return std::make_shared<Socket>(addr->getFamily(), TYPE::TCP);
    }
    Socket::ptr Socket::CreateUDP(Address::ptr addr)
    {
        Socket::ptr socket = std::make_shared<Socket>(addr->getFamily(), TYPE::UDP);
        // 创建udp的socket后要显式的在系统层面构造出真正的socket结构
        // 因为tcp的socket结构会在bind和connect时自动在函数内部创建真正socket结构
        // 而客户端的udp的socket不会进行bind和connect 而是直接读写操作---->因此需要在创建的时候直接真正创建socket结构
        socket->newSocket();
        socket->_isConnect = true;
        return socket;
    }
    Socket::ptr Socket::CreateTCPSocket()
    {
        return std::make_shared<Socket>(FAMILY::IPv4, TYPE::TCP);
    }
    Socket::ptr Socket::CreateUDPSocket()
    {
        Socket::ptr socket = std::make_shared<Socket>(FAMILY::IPv4, TYPE::UDP);
        socket->newSocket();
        socket->_isConnect = true;
        return socket;
    }
    Socket::ptr Socket::CreateTCPSocketIPv6()
    {
        return std::make_shared<Socket>(FAMILY::IPv6, TYPE::TCP);
    }
    Socket::ptr Socket::CreateUDPSocketIPv6()
    {
        Socket::ptr socket = std::make_shared<Socket>(FAMILY::IPv6, TYPE::UDP);
        socket->newSocket();
        socket->_isConnect = true;
        return socket;
    }
    Socket::ptr Socket::CreateUnixTCPSocket()
    {
        return std::make_shared<Socket>(FAMILY::UNIX, TYPE::TCP);
    }
    Socket::ptr Socket::CreateUnixUDPSocket()
    {
        std::shared_ptr<Socket> socket = std::make_shared<Socket>(FAMILY::UNIX, TYPE::UDP);
        socket->newSocket();
        socket->_isConnect = true;
        return socket;
    }
    Socket::Socket(int family, int type, int protocol)
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
            // 转化成功说明是unix地址--->绑定之前判断是否已经该路径已经在使用或者有残留
            Socket::ptr usock = CreateUnixTCPSocket();
            if (usock->Connect(uaddr)) // 以客户端方式连接
            {
                // success
                return false; // 这个路径的套接字正在被使用
            }
            // 连接失败 未被使用--->去除残留防止bind失败
            else
            {
                Xten::FileUtil::UnLink(uaddr->getPath(), true);
            }
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
    bool Socket::ReConnect(uint64_t timeout)
    {
        if (!_peerAddress.get())
        {
            XTEN_LOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
            return false;
        }
        // 重连后 本地绑定的address将失效 由系统自动重新分配
        _localAddress.reset();
        return Connect(_peerAddress, timeout);
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
            XTEN_LOG_DEBUG(g_logger) << "close: " << _sockfd;
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
    // tcp读函数-单缓冲区
    ssize_t Socket::Recv(void *buf, size_t len, int flags)
    {
        if (!IsConnected())
        {
            return -1;
        }
        ssize_t ret = ::recv(_sockfd, buf, len, flags);
        return ret;
    }
    // tcp读函数-多缓冲区
    ssize_t Socket::RecvV(struct iovec *iov, int iovcnt, int flags)
    {
        if (!IsConnected())
        {
            return -1;
        }
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = iov;
        msg.msg_iovlen = iovcnt;
        ssize_t ret = ::recvmsg(_sockfd, &msg, flags);
        return ret;
    }
    // udp读函数-单缓冲区
    ssize_t Socket::RecvFrom(void *buf, size_t len, Address::ptr from, int flags)
    {
        if (!IsConnected())
        {
            return -1;
        }
        socklen_t sock_len = from->getAddrLen();
        ssize_t ret = ::recvfrom(_sockfd, buf, len, flags, from->getAddr(), &sock_len);
        return ret;
    }
    // udp读函数-多缓冲区
    ssize_t Socket::RecvFromV(struct iovec *iov, int iovcnt, Address::ptr from, int flags)
    {
        if (!IsConnected())
        {
            return -1;
        }
        sockaddr *peer = from->getAddr();
        socklen_t socklen = from->getAddrLen();
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = (void *)peer;
        msg.msg_namelen = socklen;
        msg.msg_iov = iov;
        msg.msg_iovlen = iovcnt;
        ssize_t ret = ::recvmsg(_sockfd, &msg, flags);
        return ret;
    }
    // 批量读取
    int Socket::RecvFromBatch(std::vector<iovec> &iov, int batch_size, std::vector<std::pair<Address::ptr, size_t>> &info,
                              int flags)
    {
        if (!IsConnected())
        {
            return -1;
        }
        //struct mmsghdr {
        //        struct msghdr msg_hdr;  /* Message header */
        //        unsigned int  msg_len;  /* Number of received bytes for header */};
        struct mmsghdr msgs[batch_size];
        memset(&msgs, 0, sizeof(msgs));
        // 设置值
        for (int i = 0; i < batch_size; i++)
        {
            msgs[i].msg_hdr.msg_iov = &iov[i];
            msgs[i].msg_hdr.msg_iovlen = 1;
            msgs->msg_hdr.msg_name = info[i].first->getAddr();
            msgs->msg_hdr.msg_namelen = info[i].first->getAddrLen();
        }
        int ret = ::recvmmsg(_sockfd, msgs, batch_size, flags, nullptr);
        // 设置info中的每条数据大小
        if (ret > 0)
        {
            for (int i = 0; i < ret; i++)
            {
                info[i].second = msgs[i].msg_len;
            }
        }
        return ret;
    }
    // tcp写函数-单缓冲区
    ssize_t Socket::Send(const void *msg, size_t len, int flags)
    {
        if (IsConnected())
        {
            return ::send(_sockfd, msg, len, flags);
        }
        return -1;
    }
    // tcp写函数-多缓冲区
    ssize_t Socket::SendV(const struct iovec *iov, int iovcnt, int flags)
    {
        if (IsConnected())
        {
            struct msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)iov;
            msg.msg_iovlen = iovcnt;
            return ::sendmsg(_sockfd, &msg, flags);
        }
        return -1;
    }
    // udp写函数-单缓冲区
    ssize_t Socket::SendTo(const void *msg, size_t len, Address::ptr to, int flags)
    {
        if (IsConnected())
        {
            socklen_t socklen = to->getAddrLen();
            return ::sendto(_sockfd, msg, len, flags, to->getAddr(), socklen);
        }
        return -1;
    }
    // udp写函数-多缓冲区
    ssize_t Socket::SendToV(const struct iovec *iov, int iovcnt, Address::ptr to, int flags)
    {
        if (IsConnected())
        {
            struct msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)iov;
            msg.msg_iovlen = iovcnt;
            msg.msg_name = (void *)to->getAddr();
            msg.msg_namelen = to->getAddrLen();
            return ::sendmsg(_sockfd, &msg, flags);
        }
        return -1;
    }
    // 获取本地addres
    Address::ptr Socket::GetLocalAddress()
    {
        if (_localAddress)
        {
            return _localAddress;
        }
        Address::ptr result;
        switch (_family)
        {
        case AF_INET:
            result = std::make_shared<IPv4Address>();
            break;
        case AF_INET6:
            result = std::make_shared<IPv6Address>();
            break;
        case AF_UNIX:
            result = std::make_shared<UnixAddress>();
            break;
        default:
            result = std::make_shared<UnknownAddress>(_family);
        }
        socklen_t len = result->getAddrLen();
        // 获取sockfd对应的本地的地址
        if (getsockname(_sockfd, result->getAddr(), &len)) // 内核会对真实长度进行赋值
        {
            XTEN_LOG_ERROR(g_logger) << "getsockname error sock=" << _sockfd
                                     << " errno=" << errno << " errstr=" << strerror(errno);
            return std::make_shared<UnknownAddress>(_family);
        }
        // 域间套接字重新赋值长度（默认构造时内部的_length不是真实长度 而是 2 + 108）
        if (_family == AF_UNIX)
        {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(len); // 赋值真实路径的长度
        }
        _localAddress = result;
        return _localAddress;
    }
    // 获取对端address
    Address::ptr Socket::GetPeerAddress()
    {
        if (_peerAddress)
        {
            return _peerAddress;
        }
        Address::ptr result;
        switch (_family)
        {
        case AF_INET:
            result = std::make_shared<IPv4Address>();
            break;
        case AF_INET6:
            result = std::make_shared<IPv6Address>();
            break;
        case AF_UNIX:
            result = std::make_shared<UnixAddress>();
            break;
        default:
            result = std::make_shared<UnknownAddress>(_family);
        }
        socklen_t len = result->getAddrLen();
        if (getpeername(_sockfd, result->getAddr(), &len))
        {
            XTEN_LOG_ERROR(g_logger) << "getpeername error sock=" << _sockfd
                                     << " errno=" << errno << " errstr=" << strerror(errno);
            return std::make_shared<UnknownAddress>(_family);
        }
        // 域间套接字重新赋值长度
        if (_family == AF_UNIX)
        {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(len);
        }
        _peerAddress = result;
        return _peerAddress;
    }
    Socket::~Socket()
    {
        Close();
    }
    // 通过sockfd进行初始化---服务端通过accept获取连接之后
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
    // 获取读超时时间
    int64_t Socket::GetRecvTimeOut()
    {
        FdCtx::ptr fdctx = FdCtxMgr::GetInstance()->Get(_sockfd);
        if (fdctx)
        {
            return fdctx->GetTimeOut(SO_RCVTIMEO);
        }
        return -1;
    }
    // 设置读超时时间
    bool Socket::SetRecvTimeOut(int64_t timeout)
    {
        struct timeval val{int(timeout / 1000), int(timeout % 1000 * 1000)};
        return Setsockopt(SOL_SOCKET, SO_RCVTIMEO, val);
    }
    // 获取写超时时间
    int64_t Socket::GetSendTimeOut()
    {

        FdCtx::ptr fdctx = FdCtxMgr::GetInstance()->Get(_sockfd);
        if (fdctx)
        {
            return fdctx->GetTimeOut(SO_SNDTIMEO);
        }
        return -1;
    }
    // 设置写超时时间
    bool Socket::SetSendTimeOut(int64_t timeout)
    {
        struct timeval val{timeout / 1000, timeout % 1000 * 1000};
        return Setsockopt(SOL_SOCKET, SO_SNDTIMEO, val);
    }
    int Socket::GetError()
    {
        int error = 0;
        if (!Getsockopt(SOL_SOCKET, SO_ERROR, error))
        {
            error = errno;
        }
        return error;
    }
    // 取消读事件
    bool Socket::CancelRead()
    {
        return IOManager::GetThis()->CancelEvent(_sockfd, Xten::IOManager::Event::READ);
    }
    // 取消写事件
    bool Socket::CancelWrite()
    {
        return IOManager::GetThis()->CancelEvent(_sockfd, Xten::IOManager::Event::WRITE);
    }
    // 取消所有事件
    bool Socket::CancelAll()
    {
        return IOManager::GetThis()->CancelAll(_sockfd);
    }
    // 取消accept
    bool Socket::CancelAccept()
    {
        return IOManager::GetThis()->CancelEvent(_sockfd, Xten::IOManager::Event::READ);
    }
    // 输出信息
    std::ostream &Socket::dump(std::ostream &os) const
    {
        os << "[Socket sock=" << _sockfd
           << " is_connected=" << _isConnect
           << " family=" << _family
           << " type=" << _type
           << " protocol=" << _protocol;
        if (_localAddress)
        {
            os << " local_address=" << _localAddress->toString();
        }
        if (_peerAddress)
        {
            os << " remote_address=" << _peerAddress->toString();
        }
        os << "]";
        return os;
    }
    std::string Socket::tostring() const
    {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }
    bool Socket::isValid()
    {
        return _sockfd != -1;
    }

    // SSL安全的socket
    namespace
    {
        class _SSLInit
        {
        public:
            _SSLInit()
            {
                // 完成Openssl环境的初始化工作
                SSL_library_init();
                SSL_load_error_strings();
                OpenSSL_add_all_algorithms();
            }
        };
    }
    // 在main函数之前，编译成静态库由运行时库在__libc_start_main函数中执行该构造函数
    static _SSLInit s_sslinit;
    //  创建加密ipv4tcp套接字
    SSLSocket::ptr SSLSocket::CreateTCP(Address::ptr addr)
    {
        return std::make_shared<SSLSocket>(addr->getFamily(), TYPE::TCP);
    }
    // 创建加密ipv4tcp套接字
    SSLSocket::ptr SSLSocket::CreateTCPSocket()
    {
        return std::make_shared<SSLSocket>(FAMILY::IPv4, TYPE::TCP);
    }
    // 创建加密ipv6tcp套接字
    SSLSocket::ptr SSLSocket::CreateTCPSocketIPv6()
    {
        return std::make_shared<SSLSocket>(FAMILY::IPv6, TYPE::TCP);
    }
    SSLSocket::SSLSocket(int family, int type, int protocol)
        : Socket(family, type, protocol)
    {
    }
    // connect发起连接
    bool SSLSocket::Connect(Address::ptr addr, uint64_t timeout)
    {
        if (Socket::Connect(addr, timeout))
        {
            // 网络层连接建立成功 作为客户端开始进行TLS握手
            // 1.创建TLS上下文并用智能指针管理
            _ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
            // 2.根据上下文创建SSL结构
            _ssl.reset(SSL_new(_ctx.get()), SSL_free);
            // 3.ssl结构绑定一个socket
            SSL_set_fd(_ssl.get(), _sockfd);
            // 4.发起TLS握手
            int ret = SSL_connect(_ssl.get());
            if (ret == 1)
            {
                return true;
            }
        }
        return false;
    }
    // 接收连接
    std::shared_ptr<Socket> SSLSocket::Accept()
    {
        SSLSocket::ptr newsocket = std::make_shared<SSLSocket>(_family, _type, _protocol);
        int socketfd = ::accept(_sockfd, nullptr, nullptr);
        if (socketfd < 0)
        {
            XTEN_LOG_ERROR(g_logger) << "accept(" << _sockfd << ") errno="
                                     << errno << " errstr=" << strerror(errno);
            return nullptr;
        }
        // 将listen套接字的ssl上下文给到新的通信socket
        newsocket->_ctx = _ctx;
        if (newsocket->init(socketfd))
        {
            return newsocket;
        }
        return nullptr;
    }
    // tcp读函数-单缓冲区
    ssize_t SSLSocket::Recv(void *buf, size_t len, int flags)
    {
        if (_ssl)
        {
            return SSL_read(_ssl.get(), buf, len);
        }
        return -1;
    }
    // tcp读函数-多缓冲区(iovec *iov为iov数组的指针,iovcnt为数组的大小)
    ssize_t SSLSocket::RecvV(struct iovec *iov, int iovcnt, int flags)
    {
        if (!_ssl)
        {
            return -1;
        }
        int total = 0;
        for (int i = 0; i < iovcnt; i++)
        {
            int tmp = SSL_read(_ssl.get(), iov[i].iov_base, iov[i].iov_len);
            if (tmp <= 0)
            {
                return tmp;
            }
            total += tmp;
            if (tmp != (int)iov[i].iov_len)
            {
                break;
            }
        }
        return total;
    }
    // udp读函数-单缓冲区
    ssize_t SSLSocket::RecvFrom(void *buf, size_t len, Address::ptr from, int flags)
    {
        XTEN_ASSERT(false);
        return -1;
    }
    // udp读函数-多缓冲区(iovec *iov为iov数组的指针,iovcnt为数组的大小)
    ssize_t SSLSocket::RecvFromV(struct iovec *iov, int iovcnt, Address::ptr from, int flags)
    {
        XTEN_ASSERT(false);
        return -1;
    }
    // tcp写函数-单缓冲区
    ssize_t SSLSocket::Send(const void *msg, size_t len, int flags)
    {
        if (_ssl)
        {
            return SSL_write(_ssl.get(), msg, len);
        }
        return -1;
    }
    // tcp写函数-多缓冲区
    ssize_t SSLSocket::SendV(const struct iovec *iov, int iovcnt, int flags)
    {
        if (!_ssl)
        {
            return -1;
        }
        int total = 0;
        for (int i = 0; i < iovcnt; i++)
        {
            int tmp = SSL_write(_ssl.get(), iov[i].iov_base, iov[i].iov_len);
            if (tmp <= 0)
            {
                return tmp;
            }
            total += tmp;
            if (tmp != (int)iov[i].iov_len)
            {
                break;
            }
        }
        return total;
    }
    // udp写函数-单缓冲区
    ssize_t SSLSocket::SendTo(const void *msg, size_t len, Address::ptr to, int flags)
    {
        XTEN_ASSERT(false);
        return -1;
    }
    // udp写函数-多缓冲区
    ssize_t SSLSocket::SendToV(const struct iovec *iov, int iovcnt, Address::ptr to, int flags)
    {
        XTEN_ASSERT(false);
        return -1;
    }
    // 通过sockfd进行初始化
    bool SSLSocket::init(int sockfd)
    {
        if (Socket::init(sockfd))
        {
            // socket层面的初始化完成后，进行与客户端的TLS握手
            _ssl.reset(SSL_new(_ctx.get()), SSL_free); // 新链接的ssl上下文来自listen套接字
            SSL_set_fd(_ssl.get(), _sockfd);
            int ret = SSL_accept(_ssl.get());
            if (ret == 1)
            {
                return true;
            }
        }
        return false;
    }
    // 加载证书和私钥文件(服务端调用给listen套接字加载)
    bool SSLSocket::LoadCertificates(const std::string &cert_file, const std::string &key_file)
    {
        _ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
        if (SSL_CTX_use_certificate_chain_file(_ctx.get(), cert_file.c_str()) != 1)
        {
            // 加载证书失败
            XTEN_LOG_ERROR(g_logger) << "SSL_CTX_use_certificate_chain_file("
                                     << cert_file << ") error";
            return false;
        }
        if (SSL_CTX_use_PrivateKey_file(_ctx.get(), key_file.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            // 加载密钥对文件失败
            XTEN_LOG_ERROR(g_logger) << "SSL_CTX_use_PrivateKey_file("
                                     << key_file << ") error";
            return false;
        }
        if (SSL_CTX_check_private_key(_ctx.get()) != 1)
        {
            // 证书中的公钥和私钥配对失败
            XTEN_LOG_ERROR(g_logger) << "SSL_CTX_check_private_key cert_file="
                                     << cert_file << " key_file=" << key_file;
            return false;
        }
        return true;
    }
    // 输出信息
    std::ostream &SSLSocket::dump(std::ostream &os) const
    {
        os << "[SSLSocket sock=" << _sockfd
           << " is_connected=" << _isConnect
           << " family=" << _family
           << " type=" << _type
           << " protocol=" << _protocol;
        if (_localAddress)
        {
            os << " local_address=" << _localAddress->toString();
        }
        if (_peerAddress)
        {
            os << " remote_address=" << _peerAddress->toString();
        }
        os << "]";
        return os;
    }
    std::string SSLSocket::tostring() const
    {
        std::stringstream ss;
        dump(ss);
        return ss.str();
    }
    std::ostream &operator<<(std::ostream &os, const Xten::Socket &socket)
    {
        return socket.dump(os);
    }
    // 流式输出SSLsocket内容
    std::ostream &operator<<(std::ostream &os, const Xten::SSLSocket &socket)
    {
        return socket.dump(os);
    }
}
