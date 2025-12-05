#ifndef __XTEN_XFTP_SERVER_H__
#define __XTEN_XFTP_SERVER_H__
#include "../tcp_server.h"
#include"xftp_protocol.h"
namespace Xten
{
    //    namespace Xftp

    class XftpServer : public TcpServer
    {
    public:
        typedef std::shared_ptr<XftpServer> ptr;
        XftpServer(IOManager *accept = IOManager::GetThis(), IOManager *io = IOManager::GetThis(),
                   IOManager *process = IOManager::GetThis(), TcpServerConf::ptr config = nullptr);
        ~XftpServer() = default;

    protected:
        void handleClient(TcpServer::ptr self, Socket::ptr client) override;
    private:
        //文件信息等--redis
        XftpMessageDecoder::ptr _decoder;
    };
} // namespace Xten

#endif