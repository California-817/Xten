#ifndef __XTEN_WS_SERVER_H__
#define __XTEN_WS_SERVER_H__
#include "../tcp_server.h"
#include "ws_servlet.h"
#include "ws_session.h"
namespace Xten
{
    namespace http
    {
        class WSServer : public TcpServer
        {
        public:
            typedef std::shared_ptr<WSServer> ptr;
            WSServer(IOManager *accept = IOManager::GetThis(), IOManager *io = IOManager::GetThis(),
                     IOManager *process = IOManager::GetThis(), TcpServerConf::ptr config = nullptr);
            ~WSServer() = default;
            //获取servlet分发器
            WSServletDispatch::ptr GetWSServletDispatch() const {return _dispatch;}
            //获取process逻辑处理调度器(在servlet中可能要进行切换调度器)
            IOManager* GetProcessIOManager() const { return _processWorker;}
            protected:
            void handleClient(TcpServer::ptr self, Socket::ptr client) override;
        private:
            WSServletDispatch::ptr _dispatch;
        };
    }

}

#endif