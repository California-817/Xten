#include "xftp_worker.h"
namespace Xten
{
    XftpWorker::XftpWorker()
    {
        registerHandler(0,std::bind(&XftpWorker::handleUpLoad,this,std::placeholders::_1));
        registerHandler(1,std::bind(&XftpWorker::handleUpLoad,this,std::placeholders::_1));
    }
    void XftpWorker::registerHandler(uint32_t cmd, Handler handle)
    {
        _handles[cmd] = handle;
    }
    // 派发任务
    void XftpWorker::dispatch(XftpTask::ptr task, uint32_t cmd, uint32_t index)
    {
        auto func = _handles[cmd];
        _workers[cmd]->dispatch(index, std::bind(func, task));
    }
    //upload handler
    void XftpWorker::handleUpLoad(XftpTask::ptr task)
    {
        //1.存储文件 todo
        uint32_t sn=task->req->GetSn();
        XftpResponse::ptr rsp;

        
        //2.发送响应
        IOManager* iom=task->iom;
        XTEN_ASSERT(iom);
        auto session=task->session.lock();
        if(session)
        {
           //这里不能直接调用pushMessage---因为发送逻辑要放到iomanager中
            iom->Schedule([session,sn,rsp](){
                session->pushResponse(sn,rsp);
            });
        }
    }
    //download handler
    void XftpWorker::handleDownLoad(XftpTask::ptr task)
    {
        //todo
    }
}