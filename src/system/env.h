#ifndef __XTEN_ENV_H__
#define __XTEN_ENV_H__
#include"../singleton.hpp"
#include"../mutex.h"
#include<vector>
#include<unordered_map>
#include<string>
namespace Xten
{
    class Env : public singleton<Env>
    {
        public:
            Env()=default;
            //解析命令行参数 ./program -key1 val -key2 -key3 .... 
            bool Init(int argc , char** argv);
            //添加命令行参数
            void Add(const std::string& key , const std::string& val);
            //获取命令行参数(有默认值)
            std::string Get(const std::string& key, const std::string& val="");
            //删除命令行参数
            void Del(const std::string& key);
            //判断是否有指定key
            bool Has(const std::string& key);
            //添加help
            void AddHelp(const std::string& key,const std::string& help);
            //获取所有help
            std::string PrintHelps();
            //删除某个help
            void DelHelp(const std::string& key);
            //获取exe
            std::string GetExe() const {return _exe;}
            //获取cwd
            std::string GetCwd() const {return _cwd;}
            //获取某个相对路径的绝对路径
            std::string GetAbsolutePath(const std::string& path);
            //获取配置文件的绝对路径(默认是当前exe目录的config路径)
            std::string GetConfigPath();
        private:
        RWMutex _mtx;
        std::string _program; //程序name
        std::unordered_map<std::string,std::string> _cmdLineArgs; //命令行参数
        std::vector<std::pair<std::string,std::string>> _helps; //参数的解释
        std::string _exe; //当前程序所在目录的绝对位置
        std::string _cwd; //绝对路径(_exe去除了程序name 仅目录)
    };
}
#endif