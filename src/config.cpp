#include "../include/config.h"
namespace Xten
{
    // 定义静态成员变量
    Config::ConfigVarMap Config::_configvars_map;
    Config::ConfigFileModifyTimeMap Config::_configfile_modifytimes;
    std::unordered_map<std::string, ConfigVarBase::ptr> &Config::GetDatas() // 获取到全局唯一的static类型的map结构
    {
        static std::unordered_map<std::string, ConfigVarBase::ptr> _configvars_map; // 第一次调用时初始化一次
        return _configvars_map;
    }
    std::unordered_map<std::string, uint64_t> &Config::GetFileModifyTimes() // 存储配置文件的修改时间
    {
        static std::unordered_map<std::string, uint64_t> _configfile_modifytimes;
        return _configfile_modifytimes;
    }
    RWMutex &Config::GetMutex()// 获取锁
    {
        return _mutex;
    }

    // // 查找指定name的配置项 不存在则创建并用val赋值 存在直接返回
    // template <class T> // 模板函数
    // typename ConfigVar<T>::ptr Config::LookUp(const std::string &name, const T &val, const std::string &desc)
    // {
    //     auto configvar_map = GetDatas(); // 获取到map
    //     // 1.先判断name是否存在
    //     auto iter = configvar_map.find(name);
    //     if (iter != configvar_map.end())
    //     {
    //         // name已经存在
    //         auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
    //         if (tmp)
    //         {
    //             // 基类转子类指针成功
    //             XTEN_LOG_INFO(XTEN_LOG_ROOT()) << "lookup name=" << name << " exists";
    //             return tmp;
    //         }
    //         else
    //         {
    //             // 转化失败 说明原先子类转基类 基类转到此子类 两个子类类型不同
    //             XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "lookup name=" << name << " exists but type not " << TypeUtil::TypeToName<T>() << "! real type=" << iter->second->GetTypeName();
    //             return nullptr;
    //         }
    //     }
    //     // 不存在指定name的var--创建
    //     if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos)
    //     {
    //         // name不合法
    //         XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "lookup name=" << name << " is invalid!";
    //         return nullptr;
    //     }
    //     auto tmp = std::make_shared<ConfigVar<T>>(name, val, desc);
    //     configvar_map.insert(std::make_pair(name, tmp)); // 放入map
    //     return tmp;
    // }
    // template <class T>                                                 // 模板函数
    // typename ConfigVar<T>::ptr Config::LookUp(const std::string &name) // 仅查找
    // {
    //     auto configvar_map = GetDatas(); // 获取到map
    //     // 1.先判断name是否存在
    //     auto iter = configvar_map.find(name);
    //     if (iter != configvar_map.end())
    //     {
    //         auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
    //         if (tmp)
    //         {
    //             // 基类转子类指针成功
    //             return tmp;
    //         }
    //         else
    //         {
    //             // 转化失败 说明原先子类转基类 基类转到此子类 两个子类类型不同
    //             XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "lookup name=" << name << " exists but type not " << TypeUtil::TypeToName<T>() << "! real type=" << iter->second->GetTypeName();
    //             return nullptr;
    //         }
    //     }
    //     XTEN_LOG_INFO(XTEN_LOG_ROOT()) << "lookup name=" << name << " not exists";
    //     return nullptr;
    // }
    typename ConfigVarBase::ptr Config::LookUpBase(const std::string &name) // 查找并返回基类指针
    {
        RWMutex::ReadLock lock(GetMutex()); //加读锁
        auto& configvar_map = GetDatas(); // 获取到map  必须引用接收
        std::cout << &configvar_map << std::endl;
        // 1.先判断name是否存在
        auto iter = configvar_map.find(name);
        if (iter != configvar_map.end())
        {
            // std::cout << "find" << std::endl;
            return iter->second;
        }
        // std::cout << "not find" << std::endl;
        XTEN_LOG_INFO(XTEN_LOG_ROOT()) << "lookup name=" << name << " not exists";
        return nullptr;
    }
    static void ListAllMembers(const std::string &prefix,
                               const YAML::Node &node,
                               std::list<std::pair<std::string, const YAML::Node>> &output) // 静态全局函数---仅当前cpp中可见
    {                                                                                       // 递归将一个yaml的node中的map的kv展开
        if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos)
        {
            XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        output.push_back(std::make_pair(prefix, node));
        if (node.IsMap())
        { // node的类型是map 需要递归展开
            for (auto it = node.begin();
                 it != node.end(); ++it)
            {
                ListAllMembers(prefix.empty() ? it->first.Scalar()
                                              : prefix + "." + it->first.Scalar(),
                               it->second, output);
            }
        }
    }
    void Config::LoadFromYaml(const YAML::Node &root) // 将一个node的内容解析到对应的configvar中
    {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMembers("", root, all_nodes); // 先列出有多少个模块

        for (auto &i : all_nodes)
        {                              // 一个yml文件可以配置多个模块
            std::string key = i.first; // 某个配置模块的name 比如日志模块 就是logs :
            if (key.empty())
            {
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookUpBase(key); // 返回的是基类指针
            // std::cout << key << std::endl;
            // std::cout << var.get() << std::endl;
            if (var)
            { // 找到了对应的配置项 进行赋值
                // std::cout << "走到LoadFromYaml" << std::endl;
                if (i.second.IsScalar())
                { // 配置项是纯量--string
                    var->FromString(i.second.Scalar());
                }
                else
                { // 是其他结构 先转成string再传入
                    std::stringstream ss;
                    ss << i.second;
                    // std::cout<<"配置"<<ss.str()<<std::endl;
                    // 多态调用 每个类型的配置模块有自己的实现
                    var->FromString(ss.str());
                }
            }
        }
    }
    // force的作用时决定是否强制加载 因为加载的时候先会判断文件是否被修改过
    void Config::LoadFromConFDir(const std::string &path, bool force) // 从config路径加载所有.yam配置文件到多个node中
    {
        std::vector<std::string> files;
        FileUtil::ListAllFile(files, path, ".yml");
        // std::cout << "进入 LoadFromConFDir" << std::endl;
        for (auto &i : files)
        {
            {
                struct stat st;
                lstat(i.c_str(), &st);
                if (!force && GetFileModifyTimes()[i] == (uint64_t)st.st_mtime)
                { // 非强制加载并且文件没有修改过
                    continue;
                }
                GetFileModifyTimes()[i] = st.st_mtime; // 更新存储的文件修改时间
            }
            try
            {
                // 每一个yml文件都生成对应的node节点
                YAML::Node root = YAML::LoadFile(i);
                // std::cout << "调用 LoadFromYaml" << std::endl;
                LoadFromYaml(root); // 进行node值加载到配置项
                //走到这里的时候 logger模块的实体更新已经完成 所有之后的日志使用的是新配置
                XTEN_LOG_INFO(XTEN_LOG_ROOT()) << "LoadConfFile file="
                                               << i << " ok";
            }
            catch (...)
            {
                XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "LoadConfFile file="
                                                << i << " failed";
            }
        }
    }
}