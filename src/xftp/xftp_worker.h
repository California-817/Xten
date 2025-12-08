#ifndef __XFTP_WORKER_H__
#define __XFTP_WORKER_H__
#include "../singleton.hpp"
#include "../iomanager.h"
#include "../db/fox_thread.h"
#include "xftp_protocol.h"
#include "xftp_session.h"
#include <unordered_map>
namespace Xten
{
    namespace xftp
    {
        // Task类
        struct XftpTask
        {
            typedef std::shared_ptr<XftpTask> ptr;
            XftpRequest::ptr req;
            IOManager *iom = nullptr;
            std::weak_ptr<XftpSession> session;
        };
        // 单例的文件读取线程池---->专门进行文件io操作
        class XftpWorker : public singleton<XftpWorker>
        {
        public:
            typedef std::function<void(XftpTask::ptr)> Handler;
            XftpWorker();
            // 注册处理函数
            void registerHandler(uint32_t cmd, Handler handle);
            // 派发任务
            bool dispatch(XftpTask::ptr task, uint32_t cmd, uint32_t index);

        private:
            void handleTest(XftpTask::ptr task);

        private:
            std::unordered_map<uint32_t, Handler> _handles;         // 处理方法
            std::unordered_map<uint32_t, IFoxThread::ptr> _workers; // 所有工作线程
        };
    }
} // namespace Xten

#endif