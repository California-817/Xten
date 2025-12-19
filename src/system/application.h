#ifndef __XTEN_APPLICATION_H__
#define __XTEN_APPLICATION_H__
#include "../singleton.hpp"
#include "../tcp_server.h"
#include"../kcp/kcp_server.h"
namespace Xten
{
    class Application
    {
    public:
        Application();
        //获取单例对象指针(是在主线程main函数栈上创建)
        static Application* GetInstance();
        //初始化函数    
        bool Init(int argc,char** argv);
        //启动函数
        int Run();
        //按类型获取servers
        bool GetServersByType(const std::string& type,std::vector<TcpServer::ptr>& servs);
        //获取所有servers
        void GetAllServers(std::unordered_map<std::string,std::vector<TcpServer::ptr>>& servs); 
        void GetAllKcpServers(std::vector<kcp::KcpServer::ptr>& kcp_servs);
    private:
        int main(int argc,char** argv);
        void run_fiber();
        int _argc;
        char **_argv = nullptr;
        // 整个框架的所有服务器 [key->Type : value->Servers]
        std::unordered_map<std::string, std::vector<TcpServer::ptr>> _servers;
        std::vector<kcp::KcpServer::ptr> _kcp_servers; //kcpservers
        //主线程的执行调度器
        IOManager::ptr _mainIOManager;
        //单例指针
        static Application* _instance;
    };
}
#endif