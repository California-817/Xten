#ifndef __XTEN_HTTP_SERVER_H__
#define __XTEN_HTTP_SERVER_H__
#include "http_session.h"
#include "http.h"
#include "../tcp_server.h"
#include "servlet.h"
namespace Xten
{
    namespace http
    {
        class HttpServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<HttpServer> ptr;
            HttpServer(IOManager *accept = IOManager::GetThis(), IOManager *io = IOManager::GetThis(),
                       IOManager *process = IOManager::GetThis(), TcpServerConf::ptr config = nullptr);
            ~HttpServer()=default;
            //  获取servlet分发器来注册servlet
            ServletDispatch::ptr GetServletDispatch() const {return _dispatch;}

        protected:
            // 重写tcpserver的处理客户端连接的函数
            virtual void handleClient(TcpServer::ptr self, Socket::ptr client) override;

        private:
            bool _is_keepAlive;             // 是否长连接
            ServletDispatch::ptr _dispatch; // servlet分发器
        };
    }
}
#endif