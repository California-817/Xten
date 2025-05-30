#pragma once
#include "const.h"
#include "util.h"
#include "log.h"
// 协程库的配置模块
namespace Xten
{
    // 基类的配置单元
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &desc) : _name(name), _desc(desc)
        {
            // 将name进行转小写操作
            std::transform(_name.begin(), _name.end(), _name.begin(), ::tolower);
        }
        std::string GetName() // 获取var名字
        {
            return _name;
        }
        std::string GetDesc() // 描述
        {
            return _desc;
        }
        virtual std::string ToString() = 0;                      // 将val转成string
        virtual bool FromString(const std::string &val_str) = 0; // 从yaml的string转成val并设置值
        virtual std::string GetTypeName() = 0;                   // 获取val的类型名称
        virtual ~ConfigVarBase() {};

    protected:
        std::string _name; // 配置单元名称 如 log
        std::string _desc; // 描述
    };
    // 进行类型转换的仿函数类型   F--来源类型  T--转目标类型
    template <class F, class T>
    class lexicalCast
    {
    public:
        T operator()(const F &from)
        {                                        // 最普通的情况  string int double之间的转化
            return boost::lexical_cast<T>(from); // 这里转化失败会抛出异常
        }
    };
    // 模板偏特化 有一些特殊的类型 用boost::lexical_cast<T>(from)无法解决 需要特殊处理 ---模板特化
    // 1.string转vector<T>  底层还是化成普通类型处理
    template <class T>
    class lexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &yaml_str)
        { // yaml格式string转vector
            //[1,2,3,4,5]
            // 1.将yaml字符串解析到node中
            YAML::Node node = YAML::Load(yaml_str);
            typename std::vector<T> vec;
            std::stringstream ss;
            for (int i = 0; i < node.size(); i++)
            {
                ss.str("");    // 清空
                ss << node[i]; // 拿到每一个值
                // 普通情况可以处理
                vec.push_back(lexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };
    // 2.vector转string
    template <class T>
    class lexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &from_vec)
        {
            YAML::Node node;
            for (auto &ele : from_vec)
            {
                node.push_back(YAML::Load(lexicalCast<T, std::string>()(ele)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    // 3.string转unordered_set
    template <class T>
    class lexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string &from_str)
        {
            YAML::Node node = YAML::Load(from_str);
            std::unordered_set<T> set;
            std::stringstream ss;
            for (int i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss = node[i];
                set.insert(lexicalCast<std::string, T>()(ss.str()));
            }
            return set;
        }
    };
    // 4.unordered_set转string
    template <class T>
    class lexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(lexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
     */
    template <class T>
    class lexicalCast<std::string, std::map<std::string, T>>
    {
    public:
        std::map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, T> vec;
            std::stringstream ss;
            for (auto it = node.begin();
                 it != node.end(); ++it)
            {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(),
                                          lexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };

    /**
     * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
     */
    template <class T>
    class lexicalCast<std::map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::map<std::string, T> &v)
        {
            YAML::Node node(YAML::NodeType::Map);
            for (auto &i : v)
            {
                node[i.first] = YAML::Load(lexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
     */
    template <class T>
    class lexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string, T> vec;
            std::stringstream ss;
            for (auto it = node.begin();
                 it != node.end(); ++it)
            {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(),
                                          lexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };

    /**
     * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
     */
    template <class T>
    class lexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &v)
        {
            YAML::Node node(YAML::NodeType::Map);
            for (auto &i : v)
            {
                node[i.first] = YAML::Load(lexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 类型转换模板类片特化(YAML String 转换成 std::list<T>)
    template <class T>
    class lexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(lexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
     * @brief 类型转换模板类片特化(std::list<T> 转换成 YAML String)
     */
    template <class T>
    class lexicalCast<std::list<T>, std::string>
    {
    public:
        std::string operator()(const std::list<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(lexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
     * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
     */
    template <class T>
    class lexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::set<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.insert(lexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
     * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
     */
    template <class T>
    class lexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(lexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    // 具体的configvar配置单元  模板类  class FromStr,class ToStr 表示T转string string转T的 [仿函数] 类型
    template <class T, class FromStr = lexicalCast<std::string, T>, class ToStr = lexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        typedef std::function<void(const T &old_val, const T &new_val)> on_change_cb; // 配置val变更 修改LoggerMgr实体的函数
        ConfigVar(const std::string &name, const T &val, const std::string &desc)
            : ConfigVarBase(name, desc), _val(val) {}
        virtual std::string ToString() override // 将val转成string
        {
            try
            {
                return ToStr()(_val);
            }
            catch (std::exception &e)
            {
                std::cout << "configvalue to string error: " << e.what();
            }
            return "";
        }
        virtual bool FromString(const std::string &val_str) override // 从yaml的string转成val 并且内部会进行对val赋值
        {
            // std::cout << "走到 FromString" << std::endl;
            try
            {
                // 这个setval除了会对值进行重新设置 还会通过变更回调函数进行修改真正的配置实体
                SetValue(FromStr()(val_str));
                return true;
            }
            catch (const std::exception &e)
            {
                std::cout << "string to configvalue error: " << e.what();
            }
            return false;
        }
        virtual std::string GetTypeName() override // 获取val的类型名称
        {
            // 获取类型名
            return TypeUtil::TypeToName<T>();
        }
        // 子类增加对子类特有成员的操作函数
        T GetValue()
        { // 获取
            return _val;
        }
        void SetValue(const T &val) // 设置value的配置值   并且发现值不一样的时候会调用变更函数进行配置实体的更改
        {
            // std::cout << "SetValue" <<ToStr()(val)<< std::endl;
            if (_val == val)
            {
                return;
            }
            // 不相等
            for (auto &cb : _change_cbs)
            {
                // std::cout << "走到调用变更函数" << std::endl;
                cb.second(_val, val); // 调用变更函数---在main函数之间就完成了注册
            }
            _val = val;
        }
        uint64_t AddListener(on_change_cb cb) // 添加
        {
            // config<T>是相同类型不同对象 对应的这个cb_count只有一份 因为函数一个
            // config<T>是不同类型的不同对象 对应的这个cb_count有多份 因为类不同 函数不同
            static uint64_t cb_count = 0; // 第一次调用初始化 初始化一次 生命周期到程序结束
            _change_cbs[++cb_count] = cb;
            return cb_count;
        }
        void DelListener(uint64_t cb_key) // 删除
        {
            _change_cbs.erase(cb_key);
        }
        on_change_cb GetListener(uint64_t cb_key) // 获取
        {
            auto iter = _change_cbs.find(cb_key);
            if (iter == _change_cbs.end())
            {
                return nullptr;
            }
            return iter->second;
        }
        void ClearListener() // 清除所有回调
        {
            _change_cbs.clear();
        }

    private:
        // lock锁 todo
        T _val;                                            // 具体类型的配置值
        std::unordered_map<int, on_change_cb> _change_cbs; // 变更配置回调函数组
    };
    // configvar的管理类
    class Config
    {
    public:
        // ConfigVarMap用来保存所有配置项的 name:ConfigVar<T>ptr
        typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
        typedef std::unordered_map<std::string, uint64_t> ConfigFileModifyTimeMap;
        static ConfigVarMap _configvars_map;
        static std::unordered_map<std::string, uint64_t> _configfile_modifytimes;
        static ConfigVarMap &GetDatas();                      // 获取到全局唯一的static类型的map结构
        static ConfigFileModifyTimeMap &GetFileModifyTimes(); // 存储配置文件的修改时间

        //  预处理 编译 汇编 链接
        //  模板函数必须在编译期可见其实现，因为编译器要根据具体类型生成代码。
        //  如果只在 .h 中声明而在 .cpp 中定义，其他使用该模板的 .cpp 文件无法看到实现（在不同翻译单元），会导致 链接错误 或 编译失败
        /**
         *  @brief 将模板函数定义写在头文件中（推荐）这是最常见、最安全的做法。
         */
        // 查找指定name的配置项 不存在则创建并用val赋值 存在直接返回
        template <class T> // 模板函数
        static typename ConfigVar<T>::ptr LookUp(const std::string &name, const T &val, const std::string &desc)
        {
            // 引用返回 + 引用接收 == 操作原始对象
            // 值接收 这个函数返回的是原变量 但是由于是值接收还会进行一次拷贝
            auto &configvar_map = GetDatas(); // 获取到map
            // 1.先判断name是否存在
            auto iter = configvar_map.find(name);
            if (iter != configvar_map.end())
            {
                // name已经存在
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
                if (tmp)
                {
                    // 基类转子类指针成功
                    XTEN_LOG_INFO(XTEN_LOG_ROOT()) << "lookup name=" << name << " exists";
                    return tmp;
                }
                else
                {
                    // 转化失败 说明原先子类转基类 基类转到此子类 两个子类类型不同
                    XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "lookup name=" << name << " exists but type not " << TypeUtil::TypeToName<T>() << "! real type=" << iter->second->GetTypeName();
                    return nullptr;
                }
            }
            // 不存在指定name的var--创建
            if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos)
            {
                // name不合法
                XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "lookup name=" << name << " is invalid!";
                return nullptr;
            }
            // std::cout << name << "create" << std::endl;
            // std::cout << &GetDatas() << std::endl;
            auto tmp = std::make_shared<ConfigVar<T>>(name, val, desc);
            configvar_map.insert(std::make_pair(name, tmp)); // 放入map
            return tmp;
        }
        template <class T>                                                // 模板函数
        static typename ConfigVar<T>::ptr LookUp(const std::string &name) // 仅查找
        {
            // 引用返回 + 引用接收 == 操作原始对象
            auto &configvar_map = GetDatas(); // 获取到map
            // 1.先判断name是否存在
            auto iter = configvar_map.find(name);
            if (iter != configvar_map.end())
            {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
                if (tmp)
                {
                    // 基类转子类指针成功
                    return tmp;
                }
                else
                {
                    // 转化失败 说明原先子类转基类 基类转到此子类 两个子类类型不同
                    XTEN_LOG_ERROR(XTEN_LOG_ROOT()) << "lookup name=" << name << " exists but type not " << TypeUtil::TypeToName<T>() << "! real type=" << iter->second->GetTypeName();
                    return nullptr;
                }
            }
            XTEN_LOG_INFO(XTEN_LOG_ROOT()) << "lookup name=" << name << " not exists";
            return nullptr;
        }
        static typename ConfigVarBase::ptr LookUpBase(const std::string &name);   // 查找并返回基类指针
        static void LoadFromYaml(const YAML::Node &node);                         // 将一个yam文件的内容解析
        static void LoadFromConFDir(const std::string &path, bool force = false); // 从config路径加载所有.yam配置文件
    private:
        Config() = default; // 防止实例化
    };
}