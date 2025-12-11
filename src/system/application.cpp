#include "application.h"
#include "daemon.h"
#include "env.h"
#include "../worker.h"
#include "../log.h"
#include "../config.h"
#include "../http/http_server.h"
#include "../websocket/ws_server.h"
#include "../rock/rock_server.h"
#include "../module/module.h"
#include "../db/fox_thread.h"
#include "../db/redis.h"
#include "../util.h"
#include "../db/mysql.h"
#include"../xftp/xftp_server.h"
#include"../xftp/xftp_worker.h"
#include <signal.h>
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    static ConfigVar<std::vector<TcpServerConf>>::ptr g_servers_conf =
        Config::LookUp("servers", std::vector<TcpServerConf>(), "Servers config");
    Application::Application()
    {
        _instance = this;
    }
    Application *Application::GetInstance()
    {
        return _instance;
    }
    // 初始化函数
    bool Application::Init(int argc, char **argv)
    {
        _argc = argc;
        _argv = argv;
        Env::GetInstance()->AddHelp("s", "start with the terminal");
        Env::GetInstance()->AddHelp("d", "run as a daemon");
        Env::GetInstance()->AddHelp("c", "config path [default path=./conf]");
        Env::GetInstance()->AddHelp("p", "print helps");

        bool is_printHelp = false;
        if (!Env::GetInstance()->Init(argc, argv))
        {
            // 解析命令行失败
            is_printHelp = true;
        }
        if (Env::GetInstance()->Has("p"))
        {
            is_printHelp = true;
        }
        if (!Env::GetInstance()->Has("s") && !Env::GetInstance()->Has("d"))
        {
            is_printHelp = true;
        }
        if (Env::GetInstance()->Has("s") && Env::GetInstance()->Has("d"))
        {
            is_printHelp = true;
        }
        if (is_printHelp)
        {
            std::cout << Env::GetInstance()->PrintHelps();
            return false;
        }
        // 加载配置
        std::string config_path = Env::GetInstance()->GetConfigPath();
        XTEN_LOG_INFO(g_logger) << "Load Config path=" << config_path;
        Xten::Config::LoadFromConFDir(config_path);

        // 将module加载进来
        bool load = Xten::ModuleMgr::GetInstance()->Init();
        if (!load)
        {
            return false;
        }
        // 获取所有module调用其指定位置调用的函数
        std::vector<Module::ptr> modules;
        Xten::ModuleMgr::GetInstance()->ListAll(modules);
        for (auto &mod : modules)
        {
            mod->OnBeforeArgsParse(argc, argv);
        }
        for (auto &mod : modules)
        {
            mod->OnAfterArgsParse(argc, argv);
        }

        return true;
    }
    // 启动函数
    int Application::Run()
    {
        bool is_daemon = Env::GetInstance()->Has("d");
        return xten_start(_argc, _argv,
                          std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2), is_daemon);
    }
    // 子进程真正执行函数
    int Application::main(int argc, char **argv)
    {
        // 忽略Pipe信号
        signal(SIGPIPE, SIG_IGN);
        std::string config_path = Env::GetInstance()->GetConfigPath();
        Xten::Config::LoadFromConFDir(config_path, true);

        // 1.创建主线程的IOmanager
        _mainIOManager = std::make_shared<Xten::IOManager>(1, true, "Main_IOManager");

        // 2.进行Server的创建(在调度器中执行,因为Worker可能没有配置调度器,默认调度器就应该是main调度器)
        _mainIOManager->Schedule(std::bind(&Application::run_fiber, this));
        // 3.主线程执行循环定时器,热更新配置
        _mainIOManager->addTimer(2000, [config_path]()
                                 { 
            //主线程循环进行配置文件热更新工作(防止主线程退出)
            Xten::Config::LoadFromConFDir(config_path); }, true);
        // 主线程会阻塞在这里一直处理循环定时任务
        _mainIOManager->Stop();
        return 0;
    }
    void Application::run_fiber()
    {
        // 根据配置文件初始化Worker 创建所有调度器
        Xten::WorkerManager::GetInstance()->Init();
        //初始化foxthread
        Xten::FoxThreadManager::GetInstance()->init();
        Xten::FoxThreadManager::GetInstance()->start();

        //初始化所有redis连接
        Xten::RedisManager::GetInstance();

        //初始化mysql库
        Xten::MySQLManager::GetInstance();
        //初始化xftpWorker
        Xten::xftp::XftpWorker::GetInstance();
        // Xten::MySQLUtil::Query("default","SELECT 1"); //test
        
        // 获取所有Module
        std::vector<Module::ptr> modules;
        Xten::ModuleMgr::GetInstance()->ListAll(modules);
        bool has_error = false;
        for (auto &mod : modules)
        {
            has_error |= !mod->OnLoad();
        }
        if (has_error)
        {
            _exit(0);
        }
        // 读取Server配置创建Server
        std::vector<TcpServerConf> serversConf = g_servers_conf->GetValue();
        std::vector<TcpServer::ptr> servers;
        for (auto &servConf : serversConf)
        {
            XTEN_LOG_DEBUG(g_logger) << std::endl
                                     << lexicalCast<TcpServerConf, std::string>()(servConf);
            // 解析出一个Server的所有Address
            //  address: ["0.0.0.0:8090", "127.0.0.1:0", "/tmp/test.sock"]
            std::vector<Address::ptr> address;
            for (auto &i : servConf.address)
            {
                size_t pos = i.find(":");
                if (pos == std::string::npos)
                {
                    // 没找到-->是一个Unix地址
                    address.push_back(std::make_shared<UnixAddress>(i));
                    continue;
                }
                // 找到了-->IP地址
                uint16_t port = atoi(i.substr(pos + 1).c_str());
                auto addr1 = IPAddress::Create(i.substr(0, pos).c_str(), port);
                if (addr1)
                {
                    address.push_back(addr1);
                    continue;
                }
                // 网卡地址 eth0
                std::vector<std::pair<Address::ptr, uint32_t>> result;
                if (Address::GetInterfaceAddresses(result, i.substr(0, pos)))
                {
                    for (auto &ip : result)
                    {
                        IPAddress::ptr ipaddr = std::dynamic_pointer_cast<IPAddress>(ip.first);
                        if (ipaddr)
                        {
                            ipaddr->setPort(atoi(i.substr(pos + 1).c_str()));
                        }
                        address.push_back(ipaddr);
                    }
                    continue;
                }
                // 域名地址
                auto addr2 = Address::LookupAny(i);
                if (addr2)
                {
                    address.push_back(addr2);
                    continue;
                }
                // 非法地址
                XTEN_LOG_ERROR(g_logger) << "Invaild Address: " << i;
                _exit(0);
            }
            // 得到了所有address
            IOManager *accept_w = IOManager::GetThis();
            IOManager *io_w = IOManager::GetThis();
            IOManager *process_w = IOManager::GetThis();
            if (!servConf.accept_worker.empty())
            {
                // 指定了Worker
                accept_w = WorkerManager::GetInstance()->GetAsIOManager(servConf.accept_worker).get();
                if (!accept_w)
                {
                    XTEN_LOG_ERROR(g_logger) << "accept_worker: " << servConf.accept_worker
                                             << " is not exist";
                    _exit(0);
                }
            }
            if (!servConf.io_worker.empty())
            {
                io_w = WorkerManager::GetInstance()->GetAsIOManager(servConf.io_worker).get();
                if (!io_w)
                {
                    XTEN_LOG_ERROR(g_logger) << "io_worker: " << servConf.io_worker
                                             << " is not exist";
                    _exit(0);
                }
            }
            if (!servConf.process_worker.empty())
            {
                process_w = WorkerManager::GetInstance()->GetAsIOManager(servConf.process_worker).get();
                if (!process_w)
                {
                    XTEN_LOG_ERROR(g_logger) << "process_worker: " << servConf.process_worker
                                             << " is not exist";
                    _exit(0);
                }
            }
            // 确定了Server的三类Worker--->创建Server
            TcpServer::ptr server;
            TcpServerConf::ptr p_servConf=std::make_shared<TcpServerConf>(servConf);
            if (servConf.type == "http")
            {
                server = std::make_shared<Xten::http::HttpServer>(accept_w, io_w, process_w, p_servConf);
                server->GetTimer()->AddTimer(20,[](){XTEN_LOG_DEBUG(g_logger)<<"test on timer"<<TimeUitl::GetCurrentMS()<<std::endl;},true);
            }
            else if (servConf.type == "websocket")
            {
                server = std::make_shared<Xten::http::WSServer>(accept_w, io_w, process_w, p_servConf);
            }
            else if (servConf.type == "rock")
            {
                server = std::make_shared<Xten::RockServer>(accept_w, io_w, process_w, p_servConf);
            }
            else if(servConf.type == "xftp")
            {
                server= std::make_shared<Xten::xftp::XftpServer>(accept_w, io_w, process_w, p_servConf);
            }
            else
            {
                XTEN_LOG_ERROR(g_logger) << "Invaild Server Type: " << servConf.type;
                _exit(0);
            }
            // 为Server绑定ip
            std::vector<Address::ptr> fails;
            while (!server->Bind(address, fails))
            {
                // 失败
                for (auto &fail : fails)
                {
                    XTEN_LOG_ERROR(g_logger) << "Bind Address failed: " << fail->toString();
                }
                sleep(1);
            }
            //直到绑定成功
            if(servConf.ssl)
            {
                //Server采用TLS加密通信
                bool ret=server->LoadCertificates(servConf.cert_file,servConf.key_file);
                if(!ret)
                {
                    XTEN_LOG_ERROR(g_logger)<<"LoadCertificates failed, "<<
                    "cert_file="<<servConf.cert_file<<" key_file="<<servConf.key_file;
                    _exit(0);
                }
            }
            servers.push_back(server);
            _servers[server->GetServerConf()->type].push_back(server);
        }
        //创建出了所有Server
        for(auto& i : modules)
        {
            //进行servle的注册
            i->OnServerReady();
        }
        //启动Server
        for(auto& i : servers)
        {
            i->Start();
        }
        for(auto& i : modules)
        {
            i->OnServerUp();
        }
        modules.clear();
        XTEN_LOG_INFO(g_logger)<<"run in fiber end";
        return;
    }
    // 按类型获取servers
    bool Application::GetServersByType(const std::string &type, std::vector<TcpServer::ptr> &servs)
    {
        auto iter = _servers.find(type);
        if (iter != _servers.end())
        {
            servs = iter->second;
            return true;
        }
        return false;
    }
    // 获取所有servers
    void Application::GetAllServers(std::unordered_map<std::string, std::vector<TcpServer::ptr>> &servs)
    {
        servs = _servers;
    }
    Application *Application::_instance = nullptr;
}