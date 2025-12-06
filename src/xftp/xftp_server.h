#ifndef __XTEN_XFTP_SERVER_H__
#define __XTEN_XFTP_SERVER_H__
#include "../tcp_server.h"
#include "xftp_protocol.h"
#include"xftp_servlet.h"
namespace Xten
{
    namespace xftp
    {

        class XftpServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<XftpServer> ptr;
            XftpServer(Xten::IOManager *accept = IOManager::GetThis(), Xten::IOManager *io = IOManager::GetThis(),
                       Xten::IOManager *process = IOManager::GetThis(), TcpServerConf::ptr config = nullptr);
            ~XftpServer() = default;
            XftpServletDispatch::ptr GetDispatcher() {return _dispatcher;}
        protected:
            void handleClient(TcpServer::ptr self, Socket::ptr client) override;

        private:
            XftpServletDispatch::ptr _dispatcher;
        };
    }
} // namespace Xten

#endif