#pragma once
#include "const.h"
#include<sys/syscall.h>
//Xten协程库的工具类模块
namespace Xten
{
    class FileUtil
    {
    public:
        static bool OpenForWrite(std::ofstream& file_stream,const std::string& file_name,std::ios_base::openmode mode);
        static std::string DirName(const std::string & filename);
        static bool MakeDir(const std::string & dirname);
        static void ListAllFile(std::vector<std::string>& files,const std::string& path,const std::string& subfix);
    };
    class TimeUitl
    {
    public:
        static uint64_t NowTime_to_uint64();
    };
    class ThreadUtil
    {
        public:
            static pid_t GetThreadId();
    };
    class TypeUtil
    {
    public:
    //这个TypeToName函数也是一个模板函数 不能在cpp文件中进行实现 因为其他文件使用 在编译期间要看到其实现
        template<class T>
        static std::string TypeToName()
        {
            //这里使用static的原因
            //首先要知道这是一个模板函数---一个类型是一个函数
            //每一个类型的模板函数都只会执行一次abi::__cxa_demangle生成类型字符串 防止一个类型的函数调用多次abi::__cxa_demangle造成性能开销
            //static只会初始化一次 后续不会执行这个函数 直接返回第一次执行的结果
            static const char* ty_str=abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
            return ty_str;
        }
    };
}