#ifndef __XTEN_UTIL_H__
#define __XTEN_UTIL_H__
#include "const.h"
#include <sys/syscall.h>
namespace Xten
{
    class FileUtil
    {
    public:
        static bool OpenForWrite(std::ofstream &file_stream, const std::string &file_name, std::ios_base::openmode mode);
        static std::string DirName(const std::string &filename);
        static bool MakeDir(const std::string &dirname);
        static void ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix);
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
    //T类的构造函数是protected保护的 通过这个函数绕过保护实现创建shared_ptr智能指针
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
}
#endif