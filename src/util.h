#ifndef __XTEN_UTIL_H__
#define __XTEN_UTIL_H__
#include "const.h"
#include <sys/syscall.h>

#if defined(__linux__) && defined(__aarch64__)
#include <jsoncpp/json/json.h>
#elif defined(__linux__) && defined(__x86_64__)
#include <json/json.h>
#endif

namespace Xten
{
    class FileUtil
    {
    public:
        // 以读方式打开文件
        static bool OpenForWrite(std::ofstream &file_stream, const std::string &file_name, std::ios_base::openmode mode);
        // 获取路径名
        static std::string DirName(const std::string &filename);
        // 递归创建目录
        static bool MakeDir(const std::string &dirname);
        // 列出指定路径下的所有符合后缀的文件
        static void ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix);
        // 删除指定的文件
        static bool UnLink(const std::string &name, bool exist = false);
        static std::string Basename(const std::string &filename);
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
        // 获取当前的毫秒数
        static uint64_t GetCurrentMS();
        // 获取当前微秒数
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
        static int64_t Atoi(const std::string &str);
        static double Atof(const char *str);
    };
    // T类的构造函数是protected保护的 通过这个函数绕过保护实现创建shared_ptr智能指针 <模板函数定义内联在调用处直接展开，防止多重定义>
    template <class T, class... Args>
    inline std::shared_ptr<T> protected_make_shared(Args &&...args)
    {
        // 子类
        struct Helper : public T
        {
            Helper(Args &&...args)
                : T(std::forward<Args>(args)...) // 对子类而言 父类protected函数可访问
            {
            }
        };
        return std::make_shared<Helper>(std::forward<Args>(args)...);
    }
    class StringUtil
    {
    public:
        // 进行url编码
        static std::string UrlEncode(const std::string &str, bool space_as_plus = true);
        // 对浏览器的url编码后的字符串进行解码
        static std::string UrlDecode(const std::string &str, bool space_as_plus = true);
        static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string Format(const char *fmt, ...);
        static std::string Formatv(const char *fmt, va_list ap);
    };
    // 时间转化成字符串
    std::string Time2Str(time_t ts, const std::string &format = "%Y-%m-%d %H:%M:%S");
    template <class Iter>
    inline std::string MapJoin(Iter begin, Iter end, const std::string &tag1 = "=", const std::string &tag2 = "&")
    {
        // k1=v1&k2=v2&k3=v3
        std::stringstream ss;
        for (auto i = begin; i != end; i++)
        {
            if (i != begin)
            {
                ss << tag2;
            }
            ss << i->first << tag1 << i->second;
        }
        return ss.str();
    }
    // 生成哈希摘要函数
    std::string sha1sum(const void *data, size_t len);
    std::string sha1sum(std::string data);
    // 进行base64编码
    std::string base64encode(const std::string &data);
    class Atomic
    {
    public:
        template <class T, class S>
        static bool compareAndSwapBool(volatile T &t, S old_val, S new_val)
        {
            return __sync_bool_compare_and_swap(&t, (T)old_val, (T)new_val);
        }
        template <class T, class S = T>
        static T addFetch(volatile T &t, S v = 1)
        {
            return __sync_add_and_fetch(&t, (T)v);
        }
        template <class T, class S = T>
        static T subFetch(volatile T &t, S v = 1)
        {
            return __sync_sub_and_fetch(&t, (T)v);
        }
    };
    std::string GetHostName();

    // 作为智能指针的自定义删除器------不做任何回收工作
    template <class T>
    void nop(T *t)
    {
        (void *)t;
    }

    template <class V, class Map, class K>
    V GetParamValue(const Map &m, const K &k, const V &def = V())
    {
        auto it = m.find(k);
        if (it == m.end())
        {
            return def;
        }
        try
        {
            return boost::lexical_cast<V>(it->second);
        }
        catch (...)
        {
        }
        return def;
    }

    std::string replace(const std::string &str, char find, char replaceWith);
    std::string replace(const std::string &str, char find, const std::string &replaceWith);
    std::string replace(const std::string &str, const std::string &find, const std::string &replaceWith);

    std::string random_string(size_t len, const std::string &chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

    time_t Str2Time(const char *str, const char *format = "%Y-%m-%d %H:%M:%S");

    /* 随机生成 uint32，输入为任意字符串 */
    inline uint32_t uint32_from_string(const char *s)
    {
        /* 1. FNV-1a 哈希：把字符串打散成 32 bit */
        uint32_t h = 2166136261u;
        for (const unsigned char *p = (const unsigned char *)s; *p; ++p)
        {
            h ^= *p;
            h *= 16777619u;
        }

        /* 2. 再来一轮 xorshift32，让分布更均匀 */
        h ^= h << 13;
        h ^= h >> 17;
        h ^= h << 5;
        return h;
    }
    class JsonUtil
    {
        static std::string ToString(const Json::Value &json, bool emit_utf8 = true)
        {
            Json::StreamWriterBuilder builder;
            builder["commentStyle"] = "None";
            builder["indentation"] = "";
            builder["emitUTF8"] = emit_utf8;
            return Json::writeString(builder, json);
        }
    };
    std::string ToLower(const std::string &name);
    std::string ToUpper(const std::string &name);
    std::vector<std::string> split(const std::string &str, char delim, size_t max = ~0);
    std::vector<std::string> split(const std::string &str, const char *delims, size_t max = ~0);
}
#endif
