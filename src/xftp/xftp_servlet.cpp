#include "xftp_servlet.h"
#include "../db/redis.h"
#include "xftp_worker.h"
#include "../util.h"
#include "../log.h"
namespace Xten
{
    namespace xftp
    {
        static Logger::ptr g_logger = XTEN_LOG_NAME("system");
        FunctionXftpServlet::FunctionXftpServlet(callback cb, std::string cmd)
            : XftpServlet(cmd), m_cb(cb)
        {
        }

        FunctionXftpServlet::FunctionXftpServlet(callback cb)
            : XftpServlet("FunctionXftpServlet"), m_cb(cb)
        {
        }

        int32_t FunctionXftpServlet::handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response, Xten::SocketStream::ptr session)
        {
            return m_cb(request, response, session);
        }

        XftpServletDispatch::XftpServletDispatch()
            : XftpServlet("XftpServletDispatch")
        {
        }

        int32_t XftpServletDispatch::handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response, Xten::SocketStream::ptr session)
        {
            auto slt = getMatchedXftpServlet(request->GetCmd());
            if (slt)
            {
                return slt->handle(request, response, session);
            }
            return 0;
        }

        void XftpServletDispatch::addXftpServlet(const uint32_t &uri, XftpServlet::ptr slt)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_datas[uri] = std::make_shared<HoldXftpServletCreator>(slt);
        }

        void XftpServletDispatch::addXftpServletCreator(const uint32_t &uri, IXftpServletCreator::ptr creator)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_datas[uri] = creator;
        }

        void XftpServletDispatch::addXftpServlet(const uint32_t &uri, FunctionXftpServlet::callback cb)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_datas[uri] = std::make_shared<HoldXftpServletCreator>(
                std::make_shared<FunctionXftpServlet>(cb));
        }


        void XftpServletDispatch::delXftpServlet(const uint32_t &uri)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_datas.erase(uri);
        }

        XftpServlet::ptr XftpServletDispatch::getXftpServlet(const uint32_t &uri)
        {
            RWMutexType::ReadLock lock(m_mutex);
            auto it = m_datas.find(uri);
            return it == m_datas.end() ? nullptr : it->second->get();
        }

        XftpServlet::ptr XftpServletDispatch::getMatchedXftpServlet(const uint32_t &uri)
        {
            RWMutexType::ReadLock lock(m_mutex);
            // 精确查找
            auto mit = m_datas.find(uri);
            if (mit != m_datas.end())
            {
                return mit->second->get();
            }
            return nullptr;
        }

        void XftpServletDispatch::listAllXftpServletCreator(std::map<uint32_t, IXftpServletCreator::ptr> &infos)
        {
            RWMutexType::ReadLock lock(m_mutex);
            for (auto &i : m_datas)
            {
                infos[i.first] = i.second;
            }
        }

        int32_t TestServlet::handle(Xten::xftp::XftpRequest::ptr request, Xten::xftp::XftpResponse::ptr response,
                                    Xten::SocketStream::ptr session)
        {
            (void *)response.get();
            return DispatchReq2Worker(request,session);
        }


        int32_t DispatchReq2Worker(Xten::xftp::XftpRequest::ptr request, Xten::SocketStream::ptr session)
        {
            // 存储文件方法
            auto xftpsession = std::dynamic_pointer_cast<XftpSession>(session);
            if (!xftpsession)
                return -1;
            XftpTask::ptr task = std::make_shared<XftpTask>();
            task->iom = Xten::IOManager::GetThis();
            task->req = request;
            task->session = std::weak_ptr<XftpSession>(xftpsession);
            uint32_t index = uint32_from_string(request->GetFileName().c_str());
            Xten::xftp::XftpWorker::GetInstance()->dispatch(task, request->GetCmd(), index);
            return 0;
        }
    }
}