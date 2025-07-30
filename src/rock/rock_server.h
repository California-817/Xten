#ifndef __XTEN_ROCK_SERVER_H__
#define __XTEN_ROCK_SERVER_H__
#include "../tcp_server.h"
namespace Xten
{
    class RockServer : public TcpServer
    {
    public:
        typedef std::shared_ptr<RockServer> ptr;
        RockServer(Xten::IOManager *accept_worker = Xten::IOManager::GetThis(),
                   Xten::IOManager *io_worker = Xten::IOManager::GetThis(),
                   Xten::IOManager *process_worker = Xten::IOManager::GetThis(),
                   TcpServerConf::ptr conf = nullptr);

    protected:
        // 处理一个session的函数->由_ioWorker调度器执行网络io（and 逻辑处理）
        virtual void handleClient(TcpServer::ptr self, Socket::ptr client) override;
    };
}
#endif