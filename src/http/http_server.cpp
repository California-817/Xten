#include "http_server.h"
#include "log.h"
#include"servlets/status_servlet.h"
namespace Xten
{
    namespace http
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        HttpServer::HttpServer(IOManager *accept, IOManager *io, IOManager *process,
                               TcpServerConf::ptr config)
            : TcpServer(accept, io, process, config), _is_keepAlive(false)
        {
            if (config)
            {
                _is_keepAlive = config->keepalive;
            }
            _dispatch = std::make_shared<ServletDispatch>();
            _dispatch->setDefault(std::make_shared<NotFoundServlet>(_name));
            // 添加默认的servlet来获取服务器的状态信息
            _dispatch->addServlet("/_/status",std::make_shared<StatusServlet>());
        }

        void HttpServer::handleClient(TcpServer::ptr self, Socket::ptr client)
        {
            // 创建httpsession
            HttpSession::ptr session = std::make_shared<HttpSession>(client);
            do
            {
                HttpRequest::ptr req = session->RecvRequest();
                if (req == nullptr)
                {
                    //这里出现错误是常见的,一般是由于接受请求timeout超时触发了导致recv超时返回
                    XTEN_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                             << errno << " errstr=" << strerror(errno)
                                             << " cliet:" << *client
                                             << " keep_alive=" << _is_keepAlive;
                    break;
                }
                // std::cout<<req->toString()<<std::endl;
                HttpResponse::ptr rsp = req->createResponse();
                rsp->setClose(req->isClose() || !_is_keepAlive);
                rsp->setHeader("Server",GetName());
                rsp->setHeader("Content-Type","application/json;charset=utf8");
                // 开始处理请求(切换到process调度器执行该协程)
                {
                    SwitchScheduler sw(_processWorker);
                    _dispatch->handle(req,rsp,session);
                }
                int ret=session->SendResponse(rsp);
                if(ret<=0 || req->isClose() || !_is_keepAlive)
                {
                    break;
                }
            } while (true);
            //关闭连接
            session->Close();
        }

    }
}