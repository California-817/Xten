#ifndef __XTEN_MODULE_H__
#define __XTEN_MODULE_H__
#include <memory>
#include "../singleton.hpp"
#include "../streams/socket_stream.h"
#include "../streams/async_socket_stream.h"
#include "../rock/rock_stream.h"
#include <string>
namespace Xten
{
    // 普通业务module(http1.0 http1.1 websocket)
    class Module
    {
    public:
        typedef std::shared_ptr<Module> ptr;
        enum ModuleType
        {
            MODULE = 0, // 普通业务module
            ROCK = 1    // rock分布式服务module
            // 后面可能还会有 http2的module 或者其他类型Server的module
        };
        Module(const std::string &name,
               const std::string &filename,
               const std::string &version,
               ModuleType type = ModuleType::MODULE);
        virtual ~Module() = default;
        std::string GetName() const { return _name; }
        std::string GetFileName() const { return _filename; }
        std::string GetVersion() const { return _version; }
        std::string GetId() const { return _id; }
        ModuleType GetType() const { return _type; }
        void SetFileName(const std::string &file) { _filename = file; }
        // 命令行参数解析前后执行
        virtual void OnBeforeArgsParse(int argc, char **argv) = 0;
        virtual void OnAfterArgsParse(int argc, char **argv) = 0;
        // 加载前后执行
        virtual bool OnLoad() = 0;
        virtual bool OnUnload() = 0;
        // Server准备启动前执行(进行servlet的注册)
        virtual bool OnServerReady() = 0;
        // Server启动后执行
        virtual bool OnServerUp() = 0;

        virtual std::string StatusString();

    protected:
        std::string _name;     // module名称
        std::string _filename; // 动态库name
        std::string _version;  // module版本
        std::string _id;       // module的标识id
        ModuleType _type;      // 服务类型
    };
    // 与Rock分布式服务有关的module(RockServer)
    class RockModule : public Module
    {
    public:
        typedef std::shared_ptr<RockModule> ptr;
        RockModule(const std::string &name,
                   const std::string &filename,
                   const std::string &version,
                   ModuleType type = ModuleType::ROCK);
        // 链接建立的函数
        virtual bool OnConnect(Stream::ptr stream) = 0;
        // 连接断开函数
        virtual bool OnDisConnect(Stream::ptr stream) = 0;
        // 处理Rock请求函数
        virtual bool OnHandleRockRequest(RockRequest::ptr req, RockResponse::ptr rsp, RockStream::ptr stream) = 0;
        // 处理Rock通知函数(对服务端基本无意义)
        virtual bool OnHandleRockNotify(RockNotify::ptr notify, RockStream::ptr stream) = 0;
    };
    // 管理所有的module
    class ModuleMgr : public singleton<ModuleMgr>
    {
    public:
        ModuleMgr() = default;
        // 初始化,加载所有module
        bool Init();
        // 添加module
        void Add(Module::ptr module);
        // 删除module
        void Del(const std::string &id);
        // 删除所有module
        void DelAll();
        // 获取module
        Module::ptr Get(const std::string &id);
        // 列出所有module
        void ListAll(std::vector<Module::ptr> &vec);
        // 按照类型列出所有同类型module
        void ListAllByType(Module::ModuleType type, std::vector<Module::ptr> &vec);
        // 让所有Type类型的Module都执行指定函数
        bool Foreach(Module::ModuleType type , std::function<bool(Module::ptr)> cb);
    private:
        // 根据.so路径加载Module
        bool initWithPath(const std::string &path);
        RWMutex _mtx;
        // key->id  value->Module
        std::unordered_map<std::string, Module::ptr> _modules;
        std::unordered_map<Module::ModuleType,
                           std::unordered_map<std::string, Module::ptr>>
                                                    _type2Module; // 将module及其类型进行存储
    };
}

#endif