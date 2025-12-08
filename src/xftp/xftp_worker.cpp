#include "xftp_worker.h"
#include "xftp_servlet.h"
namespace Xten
{
    namespace xftp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");

        XftpWorker::XftpWorker()
        {
            registerHandler(XftpOptCmd::UPLOAD_TEST, std::bind(&XftpWorker::handleTest, this, std::placeholders::_1));
            // 1.foxthead init
            _workers[0] = Xten::FoxThreadManager::GetInstance()->get("xftp_worker_upload"); //执行所有类型的上传任务的线程池
            _workers[1] = Xten::FoxThreadManager::GetInstance()->get("xftp_worker_download");//执行所有类型的下载任务的线程池
        }
        void XftpWorker::registerHandler(uint32_t cmd, Handler handle)
        {
            _handles[cmd] = handle;
        }
        // 派发任务
        bool XftpWorker::dispatch(XftpTask::ptr task, uint32_t cmd, uint32_t index)
        {
            auto func = _handles[cmd];
            if (func)
            {
                _workers[cmd%2]->dispatch(index, std::bind(func, task));
                return true;
            }
            return false;
        }
        // upload test handler
        void XftpWorker::handleTest(XftpTask::ptr task)
        {
            // 1.存储文件 todo
            XftpResponse::ptr rsp = task->req->CreateResponse();
            uint32_t sn = task->req->GetSn();
            do
            {
                std::string filePath = "./static/test.txt";
                std::cout << "file_path_str is " << filePath << std::endl;
                std::ofstream outfile;
                // 第一个包
                if (sn == 1)
                {
                    // 打开文件，如果存在则清空，不存在则创建
                    FileUtil::OpenForWrite(outfile, filePath, std::ios::binary | std::ios::trunc);
                }
                else
                {
                    // 保存为文件
                    FileUtil::OpenForWrite(outfile, filePath, std::ios::binary | std::ios::app);
                }

                if (!outfile)
                {
                    std::cerr << "无法打开文件进行写入。" << std::endl;
                    rsp->SetResult(1);
                    rsp->SetResultStr("failed open file");
                    break;
                }

                outfile.write(task->req->GetData().c_str(), task->req->GetData().size());
                if (!outfile)
                {
                    std::cerr << "写入文件失败。" << std::endl;
                    rsp->SetResult(2);
                    rsp->SetResultStr("failed write file");
                    break;
                }
                outfile.close();
                std::cout << "文件已成功保存为: " << task->req->GetFileName() << std::endl;
                rsp->SetResult(0);
                rsp->SetResultStr("success write file");
            } while (false);
            // 2.发送响应
            IOManager *iom = task->iom;
            XTEN_ASSERT(iom);
            auto session = task->session.lock();
            if (session)
            {
                // 这里不能直接调用pushMessage---因为发送逻辑要放到iomanager中
                // 并不能严格保证调用和执行的顺序完全一致！！！---->其实是没有很大的影响，因为客户端接受文件分片的响应不要严格与发送顺序一致
                iom->Schedule([session, sn, rsp]()
                              { session->pushResponse(sn, rsp); });
            }
        }
    }
}