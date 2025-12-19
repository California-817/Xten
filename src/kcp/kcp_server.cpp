#include "kcp_server.h"
#include "log.h"
namespace Xten
{
    namespace kcp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        KcpServer::KcpServer(MsgHandler::ptr msghandler, IOManager *io_worker,
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
            XTEN_ASSERT(_io_worker);
            _listener = KcpListener::Create(addr, 10000, _io_worker, 1);
            return true;
        }

        // 启动若干协程进行io操作
        void KcpServer::Start()
        {
            if (_isStop == false)
                return; // running
            XTEN_ASSERT(_io_worker);
            _listener->Listen();
            auto self = shared_from_this();
            _io_worker->addTimer(3000, [this]()
                                 { XTEN_LOG_DEBUG(g_logger) << _listener->ListenerInfo(); }, true);
            _io_worker->Schedule(std::bind(&KcpServer::startAccept, this, self));
            _isStop = false;
        }
        void KcpServer::startAccept(std::shared_ptr<KcpServer> self)
        {
            // 服务器未终止一直接受链接
            while (!_isStop)
            {
                // _listener->SetAcceptTimeout(1000);
                KcpSession::ptr client = _listener->Accept();
                if (client)
                {
                    // 接受成功
                    client->SetReadTimeout(10 * 1000);
                    // 将该client的处理交给_ioWorker

                    // std::bind 在绑定成员函数时，会在运行时根据对象的实际类型进行动态绑定
                    // 如果子类继承自 TcpServer 并重写了 handleClient 虚函数，则绑定的函数是子类函数
                    _io_worker->Schedule(std::bind(&KcpServer::handleClient,
                                                   this, self, client));
                }
                else
                {
                    XTEN_LOG_ERROR(g_logger) << "accept errno=" << errno
                                             << " errstr=" << strerror(errno);
                }
            }
            _listener->Close();
        }
        void KcpServer::handleClient(std::shared_ptr<KcpServer> self, KcpSession::ptr session)
        {
            session->Start();
            // sleep(2);
            // session->ForceClose();
            // XTEN_LOG_DEBUG(g_logger) << "waitsender begin";
            // session->WaitSender();
            // XTEN_LOG_DEBUG(g_logger) << "waitsender success";
            // session->Close();
            // client->
            while (true)
            {
                KcpSession::READ_ERRNO error;
                auto req = session->ReadMessage(error);
                if (req)
                {
                    req->ToString();
                    // XTEN_LOG_DEBUG(g_logger) << "req=" << req->ToString();
                    auto rsp = req->CreateKcpResponse();
                    rsp->SetResult(0);
                    rsp->SetResultStr("success");
                    rsp->SetData(req->GetData() + "server");
                    session->SendMessage(rsp);
                }
                else
                {
                    if(error==KcpSession::READ_ERRNO::READ_TIMEOUT)
                    {
                        //timeout
                    }
                    else if(error==KcpSession::READ_ERRNO::READ_ERROR)
                    {
                        //read error
                    }
                    else if(error==KcpSession::READ_ERRNO::SESSION_CLOSE)
                    {
                        //close
                    }
                    else
                    {
                        XTEN_LOG_DEBUG(g_logger) << "recv msg error";
                        break;
                    }
                    
                }
            }
            session->ForceClose();
            XTEN_LOG_DEBUG(g_logger) << "waitsender begin";
            session->WaitSender();
            XTEN_LOG_DEBUG(g_logger) << "waitsender success";
            session->Close();
            // todo
        }
        // void KcpServer::doRead(Socket::ptr udp_socket, KcpServer::ptr self)
        // {
        //     XTEN_LOG_INFO(g_logger) << "KcpServer doRead start! udp_socket=" << *udp_socket;
        //     while (!_isStop)
        //     {
        //         //1. udpsocket中读取报文
        //         // udp_socket->RecvFromV()
        //         //2.判断包文类型
        //         //3.1创建新连接
        //         //3.2交给逻辑层处理
        //     }
        //     XTEN_LOG_INFO(g_logger) << "KcpServer doRead end! udp_socket=" << *udp_socket;
        // }
        // 停止服务器
        void KcpServer::Stop()
        {
            _isStop = true;
        }
    }
}