#include "xftp_server.h"
#include "xftp_session.h"
#include "xftp_worker.h"
namespace Xten
{
    namespace xftp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        XftpServer::XftpServer(IOManager *accept, IOManager *io,
                               IOManager *process, TcpServerConf::ptr config)
            : TcpServer(accept, io, process, config),
              _dispatcher(std::make_shared<XftpServletDispatch>())
        {
            _dispatcher->addXftpServlet(XftpOptCmd::UPLOAD_TEST, std::make_shared<Xten::xftp::TestServlet>());
        }
        void XftpServer::handleClient(TcpServer::ptr self, Socket::ptr client)
        {
            XftpSession::ptr session = std::make_shared<XftpSession>(client);
            session->startWriter(); //启动写协程
            do
            {
                // 1.recv
                XftpRequest::ptr req = session->RecvRequest();
                if (req)
                    XTEN_LOG_DEBUG(g_logger) << "req:" << req->GetData() << "cmd" << req->ToString();
                if (!req)
                    break;
                // 2.servlet handle
                auto servlet = _dispatcher->getXftpServlet(req->GetCmd());
                if (!servlet)
                    break;
                int ret = servlet->handle(req, nullptr, session);
                if (ret != 0)
                {
                    XTEN_LOG_ERROR(g_logger) << "handle XftpSession error";
                    break;
                }
            } while (session->IsConnected());
            XTEN_LOG_DEBUG(g_logger) << "XftpSession:" << session->GetPeerAddrString() << " close";
            // 及时通知写协程退出---防止内存泄露
            session->pushResponse(-1, nullptr);
            session->Close();
        }
    }
}