#ifndef __XTEN_MYMODULE_H__
#define __XTEN_MYMODULE_H__
#include "Xten/include/Xten.h"
namespace name_space
{
    class MyModule : public Xten::Module
    {
    public:
        typedef std::shared_ptr<MyModule> ptr;
        MyModule();
        // 命令行参数解析前后执行
        virtual void OnBeforeArgsParse(int argc, char **argv) override;
        virtual void OnAfterArgsParse(int argc, char **argv) override;
        // 加载前后执行
        virtual bool OnLoad() override;
        virtual bool OnUnload() override;
        // Server准备启动前执行(进行servlet的注册)
        virtual bool OnServerReady() override;
        // Server启动后执行
        virtual bool OnServerUp() override;
    };
}
#endif
