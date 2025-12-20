#include "kcp_server.h"
#include "log.h"
namespace Xten
{
    namespace kcp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        KcpServer::KcpServer(MsgHandler::ptr msghandler, IOManager *io_worker,
                             KcpServerConfig::ptr config)
            : _io_worker(io_worker),
              _msgHandler(msghandler),
              _isStop(true),
              _kcpConfig(config),
              _maxConnNum(10000),
              _coroutine_num(10),
              _name("Xten/kcp/1.0"),
              _recvTimeout(2000 * 60) // 默认2min
        {
            if (config)
            {
                _recvTimeout = config->recv_timeout_ms;
                _maxConnNum = config->max_conn_num;
                _coroutine_num = config->internal_coroutine_num;
                _name = config->name;
                if (config->timewheel)
                    _timewheel = std::make_shared<Xten::TimerWheelManager>();
            }
            //set default cbs
            _connectCb=KcpServer::defaultOnConnCb;
            _closeCb=KcpServer::defaultOnCloseCb;
            _timeoutCb=KcpServer::defaultOnTimeoutCb;
        }

        KcpServer::~KcpServer()
        {
            XTEN_ASSERT(_isStop); // 必须关闭
            _listener->Close();
        }
        // 停止服务器
        void KcpServer::Stop()
        {
            if (_isStop)
                return;
            _isStop = true;
            auto self = shared_from_this();
            _io_worker->Schedule([self, this]()
                                 {
                //stop listener
                for(auto& udpChannel : _listener->_udp_sockets)
                {
                    udpChannel->CancelAll(); //cancel event
                }
                _listener->Close(); });
        }
        bool KcpServer::Bind(Address::ptr addr)
        {
            XTEN_ASSERT(_io_worker);
            if (_kcpConfig)
                _listener = KcpListener::Create(addr, _maxConnNum, _io_worker, _coroutine_num,
                                                _kcpConfig->nodelay, _kcpConfig->interval, _kcpConfig->resend,
                                                _kcpConfig->nc, _kcpConfig->mtu_size, _kcpConfig->sndwnd, _kcpConfig->rcvwnd);
            else
                _listener = KcpListener::Create(addr, _maxConnNum, _io_worker, _coroutine_num);
            return true;
        }

        // 启动若干协程进行io操作
        void KcpServer::Start()
        {
            if (!_isStop)
                return; // running
            XTEN_ASSERT(_io_worker);
            // start listener to recv and send
            _listener->Listen();
            auto self = shared_from_this();
            // info
            _io_worker->addTimer(3000, [this]()
                                 { XTEN_LOG_DEBUG(g_logger) << _listener->ListenerInfo(); }, true);
            // accept
            _io_worker->Schedule(std::bind(&KcpServer::startAccept, this, self));
            _isStop = false;
        }

        void KcpServer::startAccept(std::shared_ptr<KcpServer> self)
        {
            if (_kcpConfig)
                _listener->SetAcceptTimeout(_kcpConfig->accept_timeout_ms);
            // 服务器未终止一直接受链接
            while (!_isStop)
            {
                KcpSession::ptr client = _listener->Accept();
                if (client)
                {
                    // 接受成功
                    client->SetReadTimeout(_recvTimeout);
                    // 将该client的处理交给_ioWorker
                    _io_worker->Schedule(std::bind(&KcpServer::handleClient,
                                                   this, self, client));
                }
                else
                {
                    XTEN_LOG_ERROR(g_logger) << "accept errno=" << errno
                                             << " errstr=" << strerror(errno);
                }
            }
            // once close
            _listener->Close();
        }
        const char *KcpServer::formSessionId(const KcpSession::ptr &session)
        {
            static thread_local char buffer[64] = {0};
            snprintf(buffer, 63, "%s-%s#%ld", _name.c_str(), _listener->_udp_sockets[0]->GetLocalAddress()->toString().c_str(), _sn++);
            session->SetInServerContainerId(buffer);
            return buffer;
        }

        void KcpServer::handleClient(std::shared_ptr<KcpServer> self, KcpSession::ptr session)
        {
            // 设置服务器
            session->SetKcpServer(self);
            // 加入map进行链接管理
            _connsMap.add(formSessionId(session), session);
            //  1.开启发送协程
            session->Start();
            // 2.启动连接开始函数
            if (_connectCb)
                _connectCb(session);
            do
            {
                KcpSession::READ_ERRNO error = KcpSession::READ_ERRNO::SUCCESS;
                auto msg = session->ReadMessage(error);
                if (msg && error == KcpSession::READ_ERRNO::SUCCESS)
                {
                    // 交给msghandler处理---todo
                    //=============test==============
                    auto req=std::dynamic_pointer_cast<KcpRequest>(msg);
                    auto rsp=req->CreateKcpResponse();
                    rsp->SetResult(0);
                    rsp->SetResultStr("success");
                    rsp->SetData(req->GetData()+"server");
                    session->SendMessage(rsp);
                    //==============================

                    if (_msgHandler)
                        _msgHandler->handleMessage(msg);
                }
                else
                {
                    if (error == KcpSession::READ_ERRNO::READ_TIMEOUT)
                    {
                        // timeout
                        // 1.执行超时回调函数
                        if (_timeoutCb)
                            _timeoutCb(session);
                        // 2.可能会发送链接关闭包文---todo

                        // 3.通知写协程退出
                        session->ForceClose();
                        break;
                    }
                    else if (error == KcpSession::READ_ERRNO::READ_ERROR ||
                             error == KcpSession::READ_ERRNO::SESSION_CLOSE)
                    {
                        // read error or session close
                        break;
                    }
                    else
                    {
                        XTEN_LOG_WARN(g_logger) << "recv msg error because of unknown reason";
                        break;
                    }
                }
            } while (true);
            // 执行onclose函数
            if (_closeCb)
                _closeCb(session);
            // 连接管理删除
            _connsMap.remove(session->GetInServerContainerId());
            // 退出了---开始关闭流程
            session->ForceClose();
            // 等发送协程退出
            session->WaitSender();
            // 关闭 once
            session->Close();
        }

         uint32_t KcpServer::defaultOnConnCb(KcpSession::ptr sess)
         {
            XTEN_LOG_INFO(g_logger)<<"on sess connectcb id="<<sess->GetInServerContainerId();
            return 0;
         }
         uint32_t KcpServer::defaultOnCloseCb(KcpSession::ptr sess)
         {
            XTEN_LOG_INFO(g_logger)<<"on sess closecb id="<<sess->GetInServerContainerId();
            return 0;
         }
         uint32_t KcpServer::defaultOnTimeoutCb(KcpSession::ptr sess)
         {
            XTEN_LOG_INFO(g_logger)<<"on sess recv timeoutcb id="<<sess->GetInServerContainerId();
            return 0;
         }
    }
}