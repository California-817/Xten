#pragma once
#include "const.h"
#include "util.h"
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
        virtual bool FromString(const std::string &val_str) = 0; // 从yaml的string转成val
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
            try
            {
                SetValue(FromStr()(val_str));
                return true;
            }
            catch(const std::exception& e)
            {
                std::cout << "string to configvalue error: " << e.what();
            }
            return false;
            
        }
        virtual std::string GetTypeName() override// 获取val的类型名称
        {
            //获取类型名
            return TypeUtil::TypeToName<T>();
        }
        // 子类增加对子类特有成员的操作函数
        T GetValue()
        { // 获取
            return _val;
        }
        void SetValue(const T& val) // 设置value的配置值   并且发现值不一样的时候会调用变更函数进行配置实体的更改
        {
            if(_val==val){
                return;
            }
            //不相等
            for(auto& cb:_change_cbs)
            {
                cb.second(_val,val); //调用变更函数---在main函数之间就完成了注册
            }
            _val=val;
        }
        uint64_t AddListener(on_change_cb cb) // 添加
        {
            //config<T>是相同类型不同对象 对应的这个cb_count只有一份 因为函数一个
            //config<T>是不同类型的不同对象 对应的这个cb_count有多份 因为类不同 函数不同
            static uint64_t cb_count=0; //第一次调用初始化 初始化一次 生命周期到程序结束
            _change_cbs[++cb_count]=cb;
            return cb_count;
        }
        void DelListener(uint64_t cb_key) // 删除
        {
            _change_cbs.erase(cb_key);
        }
        on_change_cb GetListener(uint64_t cb_key) // 获取
        {
            auto iter=_change_cbs.find(cb_key);
            if(iter==_change_cbs.end()){
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

    private:
    };
}