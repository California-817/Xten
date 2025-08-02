#include "module.h"
#include "../util.h"
#include "../log.h"
#include "../config.h"
#include "../system/env.h"
#include "library.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    static ConfigVar<std::string>::ptr g_module_path =
        Config::LookUp("module.path", std::string("./module"), "module load path");
    Module::Module(const std::string &name,
                   const std::string &filename,
                   const std::string &version,
                   ModuleType type)
        : _name(name), _filename(filename), _version(version), _type(type)
    {
        _id = name + "/" + version;
    }
    std::string Module::StatusString()
    {
        std::stringstream ss;
        ss << "Module name=" << GetName()
           << " version=" << GetVersion()
           << " filename=" << GetFileName()
           << std::endl;
        return ss.str();
    }
    RockModule::RockModule(const std::string &name,
                           const std::string &filename,
                           const std::string &version,
                           ModuleType type)
        : Module(name, filename, version, type)
    {
    }
    bool ModuleMgr::Init()
    {
        // 获取.so的路径
        std::string path = g_module_path->GetValue();
        std::string absPath = Env::GetInstance()->GetAbsolutePath(path);
        return initWithPath(absPath);
    }
    // 添加module
    void ModuleMgr::Add(Module::ptr module)
    {
        RWMutex::WriteLock wlock(_mtx);
        _modules.insert(std::make_pair(module->GetId(), module));
        _type2Module[module->GetType()].insert(std::make_pair(module->GetId(), module));
    }
    // 删除module
    void ModuleMgr::Del(const std::string &id)
    {
        RWMutex::WriteLock wlock(_mtx);
        auto iter = _modules.find(id);
        if (iter != _modules.end())
        {
            Module::ModuleType type = iter->second->GetType();
            _modules.erase(id);
            _type2Module[type].erase(id);
        }
    }
    // 删除所有module
    void ModuleMgr::DelAll()
    {
        RWMutex::WriteLock wlock(_mtx);
        _modules.clear();
        _type2Module.clear();
    }
    // 获取module
    Module::ptr ModuleMgr::Get(const std::string &id)
    {
        RWMutex::ReadLock rlock(_mtx);
        auto iter = _modules.find(id);
        return iter == _modules.end() ? nullptr : iter->second;
    }
    // 列出所有module
    void ModuleMgr::ListAll(std::vector<Module::ptr> &vec)
    {
        RWMutex::ReadLock rlock(_mtx);
        for (auto &modu : _modules)
        {
            vec.push_back(modu.second);
        }
    }
    // 按照类型列出所有同类型module
    void ModuleMgr::ListAllByType(Module::ModuleType type, std::vector<Module::ptr> &vec)
    {
        RWMutex::ReadLock rlock(_mtx);
        auto iter = _type2Module.find(type);
        if (iter != _type2Module.end())
        {
            for (auto &mod : _type2Module[type])
            {
                vec.push_back(mod.second);
            }
        }
    }
    // 根据.so路径加载Module
    bool ModuleMgr::initWithPath(const std::string &path)
    {
        std::vector<std::string> libs;
        Xten::FileUtil::ListAllFile(libs, path, ".so");
        std::sort(libs.begin(), libs.end());
        for (auto &lib : libs)
        {
            Module::ptr mod = Library::GetModule(lib);
            if (!mod)
            {
                // 该路径下的module加载失败
                return false;
            }
            // 加载成功
            Add(mod);
        }
        return true;
    }
    // 让所有Type类型的Module都执行指定函数
    bool ModuleMgr::Foreach(Module::ModuleType type, std::function<bool(Module::ptr)> cb)
    {
        std::vector<Module::ptr> mods;
        ListAllByType(type,mods);
        bool ret=true;
        for(auto& mod : mods)
        {
            ret &=cb(mod);
        }
        return ret;
    }
}