#ifndef __XTEN_UTIL_H__
#define __XTEN_UTIL_H__
#include "const.h"
#include <sys/syscall.h>
namespace Xten
{
    class FileUtil
    {
    public:
        //以读方式打开文件
        static bool OpenForWrite(std::ofstream &file_stream, const std::string &file_name, std::ios_base::openmode mode);
        //获取路径名
        static std::string DirName(const std::string &filename);
        //递归创建目录
        static bool MakeDir(const std::string &dirname);
        //列出指定路径下的所有符合后缀的文件
        static void ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix);
        //删除指定的文件
        static bool UnLink(const std::string& name,bool exist=false);    
    };
    class BackTraceUtil
    {
    public:
        // 查看到当前的整个函数调用链的栈帧情况---便于快速查找错误
        static void backtrace(std::vector<std::string> &bt, int depth, int skip);
        static std::string backtraceTostring(int depth, int skip = 2, const std::string &prefix = "     ");
    };
    class TimeUitl
    {
    public:
        static uint64_t NowTime_to_uint64();
        //获取当前的毫秒数
        static  uint64_t GetCurrentMS();
        //获取当前微秒数
        static uint64_t GetCurrentUS();
    };
    class ThreadUtil
    {
    public:
        static pid_t GetThreadId();
    };
    class FiberUtil
    {
    public:
        static int64_t GetFiberId();
    };
    class TypeUtil
    {
    public:
        // 这个TypeToName函数也是一个模板函数 不能在cpp文件中进行实现 因为其他文件使用 在编译期间要看到其实现
        template <class T>
        static std::string TypeToName() // TypeToName<T>()
        {
            // 这里使用static的原因
            // 首先要知道这是一个模板函数---一个类型是一个函数
            // 每一个类型的模板函数都只会执行一次abi::__cxa_demangle生成类型字符串 防止一个类型的函数调用多次abi::__cxa_demangle造成性能开销
            // static只会初始化一次 后续不会执行这个函数 直接返回第一次执行的结果
            static const char *ty_str = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
            return ty_str;
        }
    };
    //T类的构造函数是protected保护的 通过这个函数绕过保护实现创建shared_ptr智能指针 <模板函数定义内联在调用处直接展开，防止多重定义>
    template<class T ,class ...Args>
    inline std::shared_ptr<T> protected_make_shared(Args&&... args)
    {
        //子类
        struct Helper: public T
        {
            Helper(Args&&... args) 
            :T(std::forward<Args>(args)...) //对子类而言 父类protected函数可访问
            {}
        };
        return std::make_shared<Helper>(std::forward<Args>(args)...);
    }
    class StringUtil
    {
        public:
        //进行url编码
        static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
        //对浏览器的url编码后的字符串进行解码
        static std::string UrlDecode(const std::string& str, bool space_as_plus = true);
        static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    };
    //时间转化成字符串
    std::string Time2Str(time_t ts, const std::string& format);
    template<class Iter>
    inline std::string MapJoin(Iter begin, Iter end, const std::string& tag1="=", const std::string& tag2="&")
    {
        // k1=v1&k2=v2&k3=v3
        std::stringstream ss;
        for(auto i=begin;i!=end;i++)
        {
            if(i!=begin)
            {
                ss<<tag2;
            }
            ss<<i->first<<tag1<<i->second;
        }
        return ss.str();
    }
    //生成哈希摘要函数
    std::string sha1sum(const void* data,size_t len);
    std::string sha1sum(std::string data);
    //进行base64编码
    std::string base64encode(const std::string& data);
}
#endif