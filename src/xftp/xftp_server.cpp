#include "xftp_server.h"
#include "xftp_session.h"
#include "xftp_worker.h"
namespace Xten
{
    void XftpServer::handleClient(TcpServer::ptr self, Socket::ptr client)
    {
        XftpSession::ptr session(std::make_shared<XftpSession>(client));
        do
        {
            // session->read;
            XftpRequest::ptr req;
            XftpTask::ptr task;
            uint32_t id;
            XftpWorker::GetInstance()->dispatch(task,req->GetCmd(),req->GetSn());
        } while (session->IsConnected());
    }
}