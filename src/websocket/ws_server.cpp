#include "ws_server.h"
#include "log.h"
namespace Xten
{
    namespace http
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");

        WSSession::ptr Session_container::get(const std::string &key)
        {
            FiberMutex::Lock lock(_mtx);
            auto iter = _connsMap.find(key);
            return iter != _connsMap.end() ? iter->second : nullptr;
        }
        void Session_container::remove(const std::string &key)
        {
            FiberMutex::Lock lock(_mtx);
            _connsMap.erase(key);
        }
        void Session_container::add(const std::string &key, WSSession::ptr se)
        {
            FiberMutex::Lock lock(_mtx);
            _connsMap[key] = se;
        }
        // 发送消息
        bool Session_container::sendmsg(const std::string &key, const std::string &data,
                                        int32_t opcode, bool fin)
        {
            FiberMutex::Lock lock(_mtx);
            auto iter = _connsMap.find(key);
            if (iter != _connsMap.end())
            {
                iter->second->SendMessage(data, opcode, fin);
                return true;
            }
            return false;
        }
        // 发送消息给部分session
        void Session_container::sengmsg(const std::vector<std::string> &keys, const std::string &data,
                                        int32_t opcode, bool fin)
        {
            for (auto &key : keys)
            {
                sendmsg(key, data, opcode, fin);
            }
        }
        // 广播消息
        void Session_container::broadcastmsg(const std::string &data,
                                             int32_t opcode, bool fin)
        {
            FiberMutex::Lock lock(_mtx);
            for (auto &conn : _connsMap)
            {
                conn.second->SendMessage(data, opcode, fin);
            }
        }

        WSServer::WSServer(IOManager *accept, IOManager *io,
                           IOManager *process, TcpServerConf::ptr config)
            : TcpServer(accept, io, process, config),
              _sn(1)
        {
            _dispatch = std::make_shared<WSServletDispatch>();
            _dispatch->addServlet("/_/test",Testhandle,TestonConnect,TestonClose);
        }
        const char *WSServer::formSessionId(const WSSession::ptr &session)
        {
            static thread_local char buffer[64] = {0};
            snprintf(buffer, 63, "%s-%s#%ld", _name.c_str(), session->GetLocalAddrString().c_str(), _sn++);
            std::cout<<buffer<<std::endl;
            session->setId(buffer);
            return buffer;
        }
        void WSServer::handleClient(TcpServer::ptr self, Socket::ptr client)
        {
            client->SetRecvTimeOut(999999999); //超时不由这里决定，由定时器决定  
            XTEN_LOG_DEBUG(g_logger) << "handle ws client: " << *client;
            WSSession::ptr session = std::make_shared<WSSession>(client, 
                std::dynamic_pointer_cast<WSServer>(shared_from_this()), true);
            do
            {
                // 先进行协议升级握手
                HttpRequest::ptr shake_req = session->HandleShake();
                if (!shake_req)
                {
                    XTEN_LOG_DEBUG(g_logger) << "handle shake failed";
                    break;
                }
                // 握手成功---获取servlet（根据握手请求的uri来获取servlet）
                WSServlet::ptr servlet = _dispatch->getWSServlet(shake_req->getPath()); // 后面会修改整个wsServer的收，处理，发包的架构 todo....
                if (!servlet)
                {
                    XTEN_LOG_DEBUG(g_logger) << "no match Servlet";
                    break;
                }
                // 执行servlet的三个函数
                // 1.onconnect
                int conret = servlet->onConnect(shake_req, session);
                if (conret)
                {
                    XTEN_LOG_DEBUG(g_logger) << "onConnect ret: " << conret;
                    break;
                }
                // 连接加入map
                _connsMap.add(formSessionId(session), session);
                // 2.启动写协程
                session->StartSender();
                // 3.启动读超时
                session->setTimeout(_recvTimeout);
                session->StartTimer();
                while (true)
                {
                    // 在这个循环内部进行全双工通信
                    WSFrameMessage::ptr msg = session->RecvMessage();
                    if (!msg)
                    {
                        break;
                    }
                    // 在handle函数内部进行逻辑处理和发送响应
                    int handleret = 0;
                    {
                        SwitchScheduler sw(_processWorker);
                        handleret = servlet->handle(shake_req, msg, session);
                    }
                    if (handleret) // 不像http的sevlet，这个websocket的dispatch没有default的servlet
                    {
                        XTEN_LOG_DEBUG(g_logger) << "handle ret: " << handleret;
                        break;
                    }
                    session->StartTimer(); // 重新设置超时
                }
                // 链接到了尾声
                servlet->onClose(shake_req, session);
                // 删除管理的连接
                _connsMap.remove(session->getId());
                // 通知写协程退出
                session->ForceClose();
                // 等待退出
                session->waitSender();
            } while (false);
            // 关闭socket连接
            session->Close();
        }
    }
}