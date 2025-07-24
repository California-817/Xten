#include "ws_server.h"
#include "log.h"
namespace Xten
{
    namespace http
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        WSServer::WSServer(IOManager *accept, IOManager *io,
                           IOManager *process, TcpServerConf::ptr config)
            : TcpServer(accept, io, process, config)
        {
            _dispatch = std::make_shared<WSServletDispatch>();
        }
        void WSServer::handleClient(TcpServer::ptr self, Socket::ptr client)
        {
            XTEN_LOG_DEBUG(g_logger) << "handle ws client: " << *client;
            WSSession::ptr session = std::make_shared<WSSession>(client, true);
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
                WSServlet::ptr servlet = _dispatch->getWSServlet(shake_req->getPath());
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
                while (true)
                {
                    // 在这个循环内部进行全双工通信
                    WSFrameMessage::ptr msg = session->RecvMessage();
                    if (!msg)
                    {
                        break;
                    }
                    // 在handle函数内部进行逻辑处理和发送响应
                    int handleret = servlet->handle(shake_req, msg, session);
                    if (handleret) //不像http的sevlet，这个websocket的dispatch没有default的servlet
                    {
                        XTEN_LOG_DEBUG(g_logger) << "handle ret: " << handleret;
                        break;
                    }
                }
                // 链接到了尾声
                servlet->onClose(shake_req, session);
            } while (false);
            session->Close();
        }
    }
}