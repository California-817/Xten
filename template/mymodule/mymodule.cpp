#include "mymodule.h"
namespace name_space
{
    static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("root");
    MyModule::MyModule()
        : Module("Mymodule", "libMymodule.so", "1.0.0")
    {
    }
    // 命令行参数解析前后执行
    void MyModule::OnBeforeArgsParse(int argc, char **argv)
    {
        XTEN_LOG_INFO(g_logger) << "MyModule OnBeforeArgsParse";
    }
    void MyModule::OnAfterArgsParse(int argc, char **argv)
    {
        XTEN_LOG_INFO(g_logger) << "MyModule OnAfterArgsParse";
    }
    // 加载前后执行
    bool MyModule::OnLoad()
    {
        XTEN_LOG_INFO(g_logger) << "MyModule OnLoad";
        return true;
    }
    bool MyModule::OnUnload()
    {
        XTEN_LOG_INFO(g_logger) << "MyModule OnUnLoad";
        return true;
    }
    // Server准备启动前执行(进行servlet的注册)
    bool MyModule::OnServerReady()
    {
        XTEN_LOG_INFO(g_logger) << "MyModule OnServerReady";
        std::vector<Xten::TcpServer::ptr> servs;
        Xten::Application::GetInstance()->GetServersByType("http", servs);
        for (auto &serv : servs)
        {
            auto hs = std::dynamic_pointer_cast<Xten::http::HttpServer>(serv);
            if (hs)
            {
                // 添加Servlet处理函数
                //  hs->GetServletDispatch()->addServlet();
            }
        }
        return true;
    }
    // Server启动后执行
    static int count = 0;
    bool MyModule::OnServerUp()
    {
        XTEN_LOG_INFO(g_logger) << "MyModule OnServerUp";
        std::vector<Xten::TcpServer::ptr> servs;
        Xten::Application::GetInstance()->GetServersByType("http", servs);
        for (auto &serv : servs)
        {
            auto hs = std::dynamic_pointer_cast<Xten::http::HttpServer>(serv);
            if (hs)
            {
                if (hs->GetTimer())
                {
                    hs->GetTimer()->AddTimer(1000, []()
                                             { XTEN_LOG_DEBUG(g_logger) << "测试On Timer" << "count:" << count++; }, true);
                }
            }
        }
        return true;
    }
};
// 创建Module的函数-防止函数名修饰
extern "C"
{
    Xten::Module *CreateModule()
    {
        Xten::Module *mod = new name_space::MyModule();
        XTEN_LOG_INFO(name_space::g_logger) << "CreateModule: " << mod;
        return mod;
    }
    void DestoryModule(Xten::Module *ptr)
    {
        delete ptr;
        XTEN_LOG_INFO(name_space::g_logger) << "DestoryModule: " << ptr;
    }
}