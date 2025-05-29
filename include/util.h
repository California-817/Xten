#pragma once
#include "const.h"
//Xten协程库的工具类模块
namespace Xten
{
    class FileUtil
    {
    public:
        static bool OpenForWrite(std::ofstream& file_stream,const std::string& file_name,std::ios_base::openmode mode);
        static std::string DirName(const std::string & filename);
        static bool MakeDir(const std::string & dirname);
    };
    class TimeUitl
    {
    public:
        static uint64_t NowTime_to_uint64();
    };
    class TypeUtil
    {
    public:
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