#include "../include/tcp_server.h"
#include "../include/log.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    TcpServer::TcpServer(Xten::IOManager *accept_worker,
                         Xten::IOManager *io_worker,
                         Xten::IOManager *process_worker,
                         TcpServerConf::ptr conf)
        : _acceptWorker(accept_worker),
          _ioWorker(io_worker),
          _processWorker(process_worker),
          _conf(conf),
          _isStop(true),
          _timeWheelMgr(nullptr)
    {
        if (conf)
        {
            _recvTimeout = conf->timeout;
            _name = conf->name;
            _type = conf->type;
            _isSSL = conf->ssl;
            _timeWheel = conf->timewheel;
        }
        if (_timeWheel)
        {
            // 创建时间轮定时器，由_processWorker处理定时事件
            _timeWheelMgr = std::make_shared<TimerWheelManager>(_processWorker);
        }
    }
    // 绑定一个地址
    bool TcpServer::Bind(Address::ptr addr)
    {
        std::vector<Address::ptr> addrs;
        std::vector<Address::ptr> fails;
        addrs.push_back(addr);
        return Bind(addrs, fails);
    }
    // 绑定多组地址并返回绑定失败的地址
    bool TcpServer::Bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails)
    {
        // 1.创建socket
        for (auto &addr : addrs)
        {
            Socket::ptr newsocket = _isSSL ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
            if (!newsocket->Bind(addr))
            {
                // 绑定失败
                XTEN_LOG_ERROR(g_logger) << "Bind fail errno="
                                         << errno << " errstr=" << strerror(errno)
                                         << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            // 设置监听状态
            if (!newsocket->Listen())
            {
                XTEN_LOG_ERROR(g_logger) << "Listen fail errno="
                                         << errno << " errstr=" << strerror(errno)
                                         << " addr=[" << addr->toString() << "]";
                fails.push_back(addr);
                continue;
            }
            // 都成功将套接字放入数组
            _listenSockets.push_back(newsocket);
        }
        // 有一个地址绑定失败-->所有地址重新绑定
        if (!fails.empty())
        {
            _listenSockets.clear();
            return false;
        }
        // 都绑定成功
        for (auto &i : _listenSockets)
        {
            XTEN_LOG_INFO(g_logger) << "type=" << _type
                                    << " name=" << _name
                                    << " ssl=" << _isSSL
                                    << " server bind success: " << *i;
        }
        return true;
    }
    // 加载证书
    bool TcpServer::LoadCertificates(const std::string &cert_file, const std::string &key_file)
    {
        if (!_isSSL)
        {
            return true;
        }
        for (auto &socket : _listenSockets)
        {
            SSLSocket::ptr sslsocket = std::dynamic_pointer_cast<SSLSocket>(socket);
            if (sslsocket)
            {
                if (!sslsocket->LoadCertificates(cert_file, key_file))
                {
                    return false;
                }
            }
        }
        return true;
    }
    // 启动服务器
    bool TcpServer::Start()
    {
        if (!_isStop)
        {
            return false;
        }
        _isStop = false;
        for (auto &listenSocket : _listenSockets)
        {
            // 由_acceptWorker这个调度器执行接受链接操作
            _acceptWorker->Schedule(std::bind(&TcpServer::startAccept,
                                              this, shared_from_this(), listenSocket));
        }
        return true;
    }
    // 终止服务器
    void TcpServer::Stop()
    {
        if (_isStop)
        {
            return;
        }
        _isStop = true;
        // 保证当调度器执行函数的时候这个server还存在
        auto self = shared_from_this();
        _acceptWorker->Schedule([self, this]()
                                {
            for(auto& listensocket:_listenSockets)
            {
                //取消该socket的事件（取消调度器中保存的该fd的事件）
                listensocket->CancelAll();
                //关闭listen套接字
                listensocket->Close();
            }
            _listenSockets.clear(); });
    }
    // 这个tcpServer的析构是在整个程序的主线程中进行
    TcpServer::~TcpServer()
    {
        for (auto &i : _listenSockets)
        {
            i->Close();
        }
        _listenSockets.clear();
    }
    // 内部函数-->由_acceptWorker调度器执行接受链接函数
    void TcpServer::startAccept(TcpServer::ptr self, Socket::ptr listensocket)
    {
        // 服务器未终止一直接受链接
        while (!_isStop)
        {
            Socket::ptr client = listensocket->Accept();
            if (client)
            {
                // 接受成功
                client->SetRecvTimeOut(_recvTimeout);
                // 将该client的处理交给_ioWorker

                // std::bind 在绑定成员函数时，会在运行时根据对象的实际类型进行动态绑定
                // 如果子类继承自 TcpServer 并重写了 handleClient 虚函数，则绑定的函数是子类函数
                _ioWorker->Schedule(std::bind(&TcpServer::handleClient,
                                              this, self, client));
            }
            else
            {
                XTEN_LOG_ERROR(g_logger) << "accept errno=" << errno
                                         << " errstr=" << strerror(errno);
            }
        }
    }
    // 处理一个session的函数->由_ioWorker调度器执行网络io（or and 逻辑处理)
    void TcpServer::handleClient(TcpServer::ptr self, Socket::ptr client)
    {
        XTEN_LOG_INFO(g_logger) << "handleClient: " << *client;
    }
    // server信息转化成字符串
    std::string TcpServer::ToString(const std::string &prefix) const
    {
        std::stringstream ss;
        ss << prefix << "[type=" << _type
           << " name=" << _name << " ssl=" << _isSSL
           << " worker=" << (_processWorker ? _processWorker->GetName() : "")
           << " accept=" << (_acceptWorker ? _acceptWorker->GetName() : "")
           << " recv_timeout=" << _recvTimeout << "]" << std::endl;
        std::string pfx = prefix.empty() ? "    " : prefix;
        for (auto &i : _listenSockets)
        {
            ss << pfx << pfx << *i << std::endl;
        }
        return ss.str();
    }
}