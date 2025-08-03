#include "status_servlet.h"
#include "../../Xten.h"
#include <iomanip>
#include <ios>
#ifndef FIBER_TYPE
#define FIBER_TYPE FIBER_FCONTEXT
#endif
#ifndef OPTIMIZE
#define OPTIMIZE ON
#endif
namespace Xten
{
    namespace http
    {
        static const char *getFiberType()
        {
            if (FIBER_TYPE == FIBER_UCONTEXT)
            {
                return "FIBER_UCONTEXT";
            }
            else if (FIBER_TYPE == FIBER_FCONTEXT)
            {
                return "FIBER_FCONTEXT";
            }
            else if (FIBER_TYPE == FIBER_COCTX)
            {
                return "FIBER_COCTX";
            }
            else
            {
                return "UNKNOW FIBER TYPE";
            }
        }
        static std::string format_run_time(int64_t ts)
        {
            std::stringstream ss;
            bool v = false;
            if (ts >= 3600 * 24)
            {
                ss << (ts / 3600 / 24) << "d ";
                ts = ts % (3600 * 24);
                v = true;
            }
            if (ts >= 3600)
            {
                ss << (ts / 3600) << "h ";
                ts = ts % 3600;
                v = true;
            }
            else if (v)
            {
                ss << "0h ";
            }
            if (ts >= 60)
            {
                ss << (ts / 60) << "m ";
                ts = ts % 60;
            }
            else if (v)
            {
                ss << "0m ";
            }
            ss << ts << "s";
            return ss.str();
        }
        StatusServlet::StatusServlet()
            : Servlet(std::string("StatusServlet"))
        {
        }
        int32_t StatusServlet::handle(Xten::http::HttpRequest::ptr request, Xten::http::HttpResponse::ptr response,
                                      Xten::SocketStream::ptr session)
        {
            response->setHeader("Content-Type", "text/text; charset=utf-8");
#define XX(key) \
    ss << std::setw(30) << std::right << key << ": "
            std::stringstream ss;
            ss << "======================================================" << std::endl;
            XX("Server_Version") << "Xten/1.0.0" << std::endl;
            std::vector<Module::ptr> ms;
            ModuleMgr::GetInstance()->ListAll(ms);
            XX("Modules");
            for (int i = 0; i < ms.size(); i++)
            {
                if (ms.empty())
                {
                    ss << "Have not Load Any Module";
                }
                if (i)
                {
                    ss << ",";
                }
                ss << ms[i]->GetId();
            }
            ss << std::endl;
            XX("Host") << GetHostName() << std::endl;
            XX("Daemon_Id") << ProcessInfo::GetInstance()->parent_id << std::endl;
            XX("Main_Id") << ProcessInfo::GetInstance()->main_id << std::endl;
            XX("Daemon_Start") << Time2Str(ProcessInfo::GetInstance()->parent_start_time, "%a, %d %b %Y %H:%M:%S") << std::endl;
            XX("Main_Start") << Time2Str(ProcessInfo::GetInstance()->main_start_time, "%a, %d %b %Y %H:%M:%S") << std::endl;
            XX("Main_Restart_Count") << ProcessInfo::GetInstance()->restart_main_count << std::endl;

            XX("Daemon_RunTime") << format_run_time(time(0) - ProcessInfo::GetInstance()->parent_start_time) << std::endl;
            XX("Main_RunTime") << format_run_time(time(0) - ProcessInfo::GetInstance()->main_start_time) << std::endl;
            ss << "======================================================" << std::endl;
            XX("Optimize_Queue") << ((OPTIMIZE == ON) ? "ON" : "OFF");
            XX("Fiber_Type") << getFiberType() << std::endl;
            XX("Fiber_Count") << Fiber::GetTotalFiberNums() << std::endl;
            ss << "======================================================" << std::endl;
#undef XX
            ss << "<<Logger>>" << std::endl;
            ss << LoggerManager::GetInstance()->toYamlString() << std::endl;
            ss << "======================================================" << std::endl;
            ss << "<<Worker>>" << std::endl;
            WorkerManager::GetInstance()->dump(ss) << std::endl;
            ss << "======================================================" << std::endl;

            std::unordered_map<std::string, std::vector<TcpServer::ptr>> servers;
            Application::GetInstance()->GetAllServers(servers);
            for (auto iter = servers.begin(); iter != servers.end(); iter++)
            {
                if (iter != servers.begin())
                {
                    ss << "************************************************" << std::endl;
                }
                ss << "<<Servers=> [" << iter->first << "]>>" << std::endl;
                for (auto iiter = iter->second.begin(); iiter != iter->second.end(); iiter++)
                {
                    if (iiter != iter->second.begin())
                    {
                        ss << "--------------------------------------------" << std::endl;
                    }
                    ss << (*iiter)->ToString() << std::endl;
                    HttpServer::ptr hs;
                    hs = std::dynamic_pointer_cast<HttpServer>((*iiter));
                    if (hs)
                    {
                        // http服务器
                        auto sd = hs->GetServletDispatch();
                        if (sd)
                        {
                            std::map<std::string, IServletCreator::ptr> infos;
                            sd->listAllServletCreator(infos);
                            if (!infos.empty())
                            {
                                ss << "[Servlets]" << std::endl;
#define XX2(key) \
    ss << std::setw(30) << std::right << key << ": "
                                for (auto &i : infos)
                                {
                                    XX2(i.first) << i.second->getName() << std::endl;
                                }
                                infos.clear();
                            }
                            sd->listAllGlobServletCreator(infos);
                            if (!infos.empty())
                            {
                                ss << "[Servlets.Globs]" << std::endl;
                                for (auto &i : infos)
                                {
                                    XX2(i.first) << i.second->getName() << std::endl;
                                }
                                infos.clear();
                            }
                        }
                    }
                }
            }
            ss<<"========================================================"<<std::endl;
            for(int i=0;i<ms.size();i++)
            {
                if(i)
                {
                    ss<<"************************************************"<<std::endl;
                }
                ss<<ms[i]->StatusString()<<std::endl;
            }
            ss<<"========================================================"<<std::endl;
            response->setBody(ss.str());
            return 0;
        }
    }
}