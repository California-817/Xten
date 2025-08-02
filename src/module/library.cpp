#include "library.h"
#include <dlfcn.h>
#include "../log.h"
#include "../config.h"
#include "../system/env.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    typedef Module *(*create_module)();
    typedef void (*destory_module)(Module *);
    // 模块卸载器(智能指针自定义删除器)
    class ModuleCloser
    {
    public:
        ModuleCloser(void *handle, destory_module destory)
            : _destory(destory), _handle(handle)
        {
        }
        void operator()(Module *ptr)
        {
            // 卸载模块
            std::string name = ptr->GetName();
            std::string version = ptr->GetVersion();
            std::string id = ptr->GetId();
            std::string path = ptr->GetFileName();
            // 1.删除模块对象（位于堆上）
            _destory(ptr);
            // 2.卸载动态库
            int ret = dlclose(_handle);
            if (ret)
            {
                // 卸载失败
                XTEN_LOG_ERROR(g_logger) << "dlclose a module failed, name=" << name
                                         << " version=" << version << " id=" << id << " path" << path
                                         << " error" << dlerror();
            }
            else
            {
                // 卸载成功
                XTEN_LOG_INFO(g_logger) << "dlclose a module success, name=" << name
                                        << " version=" << version << " id=" << id << " path" << path;
            }
        }

    private:
        destory_module _destory; // lib中调用的用户删除module函数
        void *_handle;           // lib.so的动态符号表handle
    };
    Module::ptr Library::GetModule(const std::string &lib_path)
    {
        if (lib_path.empty())
        {
            return nullptr;
        }
        // 运行时加载动态库
        void *handle = dlopen(lib_path.c_str(), RTLD_NOW);
        if (!handle)
        {
            // 打开失败
            XTEN_LOG_ERROR(g_logger) << "Cannot open lib.so path=" << lib_path
                                     << ", error=" << dlerror();
            return nullptr;
        }
        // 打开成功,通过handle获取符号表中函数地址
        create_module create = (create_module)dlsym(handle, "CreateModule");
        if (!create)
        {
            XTEN_LOG_ERROR(g_logger) << "Cannot load symbol CreateModule in" << lib_path
                                     << ", error=" << dlerror();
            return nullptr;
        }
        destory_module destory = (destory_module)dlsym(handle, "DestoryModule");
        if (!destory)
        {
            XTEN_LOG_ERROR(g_logger) << "Cannot load symbol DestoryModule in" << lib_path
                                     << ", error=" << dlerror();
            return nullptr;
        }
        // 调用create函数创建module并交给智能指针管理
        Module::ptr libmodule = std::shared_ptr<Module>(create(), ModuleCloser(handle, destory));
        XTEN_LOG_INFO(g_logger) << "Success Create Module in lib.so, path=" << lib_path
                                << " Name=" << libmodule->GetName()
                                << " Version=" << libmodule->GetVersion()
                                << " id=" << libmodule->GetId(); // 模块的一些属性

        Config::LoadFromConFDir(Xten::Env::GetInstance()->GetConfigPath());
        return libmodule;
    }
}
