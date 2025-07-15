#include "../include/util.h"
#include <execinfo.h>
#include "log.h"
#include "fiber.h"
#include <sys/time.h>
namespace Xten
{
    static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");
    // 打开文件读
    bool FileUtil::OpenForWrite(std::ofstream &file_stream, const std::string &file_name, std::ios_base::openmode mode)
    {
        file_stream.open(file_name, mode);
        if (!file_stream.is_open())
        {
            // 打开失败
            // 1.获取目录
            std::string dirname = DirName(file_name);
            // 2.创建目录
            MakeDir(dirname);
            // 3.重新打开
            file_stream.open(file_name, mode);
        }
        return file_stream.is_open();
    }
    std::string FileUtil::DirName(const std::string &filename)
    {
        if (filename.empty())
        {
            return ".";
        }
        auto pos = filename.rfind("/");
        if (pos == 0)
        {
            // 最后一个/在开头 说明是根目录
            return "/";
        }
        else if (pos == std::string::npos)
        {
            // 没找到/ 说明就在当前目录
            return ".";
        }
        else
        {
            //    /usr/local/log/log.txt  截取最后一个/之前的作为目录
            return filename.substr(0, pos);
        }
        return ".";
    }
    static int __lstat(const char *dirname)
    {
        // 判断文件目录是否存在
        struct stat sta;
        int ret = lstat(dirname, &sta);
        return ret;
    }
    static int __mkdir(const char *dirname)
    {
        // 创建目录
        if (access(dirname, F_OK) == 0)
        {
            return 0;
        }
        return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    bool FileUtil::MakeDir(const std::string &dirname)
    {
        // 创建目录   /usr/local/log
        if (__lstat(dirname.c_str()) == 0)
        {
            return true;
        }
        char *path = strdup(dirname.c_str());
        char *ptr = strchr(path + 1, '/');
        do
        { // 递归创建多层目录
            for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/'))
            {
                *ptr = '\0';
                if (__mkdir(path) != 0)
                {
                    break;
                }
            }
            if (ptr != nullptr)
            {
                break;
            }
            else if (__mkdir(path) != 0)
            {
                break;
            }
            free(path);
            return true;
        } while (0);
        free(path);
        return false;
    }
    // 存储指定路径下符合后缀的文件名及其路径
    void FileUtil::ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix)
    {
        if (access(path.c_str(), 0) != 0)
        {
            return;
        }
        DIR *dir = opendir(path.c_str());
        if (dir == nullptr)
        {
            return;
        }
        struct dirent *dp = nullptr;
        while ((dp = readdir(dir)) != nullptr)
        {
            if (dp->d_type == DT_DIR)
            {
                if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
                {
                    continue;
                }
                ListAllFile(files, path + "/" + dp->d_name, subfix);
            }
            else if (dp->d_type == DT_REG)
            {
                std::string filename(dp->d_name);
                if (subfix.empty())
                {
                    files.push_back(path + "/" + filename);
                }
                else
                {
                    if (filename.size() < subfix.size())
                    {
                        continue;
                    }
                    if (filename.substr(filename.length() - subfix.size()) == subfix)
                    {
                        files.push_back(path + "/" + filename);
                    }
                }
            }
        }
        closedir(dir);
    }
    bool FileUtil::UnLink(const std::string &name, bool exist)
    {
        if (!exist && __lstat(name.c_str()))
        {
            return true;
        }
        return (::unlink(name.c_str()) == 0);
    }

    pid_t ThreadUtil::GetThreadId()
    {
        return syscall(SYS_gettid); // 系统调用 syscall 获取当前线程的线程ID（TID），并将其返回。
    }
    uint64_t TimeUitl::NowTime_to_uint64()
    {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
        // 转换为自1970年1月1日以来的时间戳（秒）
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        // 将时间戳转换为 uint64_t
        uint64_t timestamp_uint64 = static_cast<uint64_t>(timestamp);
        return timestamp_uint64;
    }
    static std::string demangle(const char *str)
    {
        size_t size = 0;
        int status = 0;
        std::string rt;
        rt.resize(256);
        if (1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0]))
        {
            char *v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
            if (v)
            {
                std::string result(v);
                free(v);
                return result;
            }
        }
        if (1 == sscanf(str, "%255s", &rt[0]))
        {
            return rt;
        }
        return str;
    }
    void BackTraceUtil::backtrace(std::vector<std::string> &bt, int depth, int skip)
    {
        void **buffer = (void **)malloc(sizeof(void *) * depth);
        int ret = ::backtrace(buffer, depth);
        char **strings = backtrace_symbols(buffer, ret);
        if (strings == NULL)
        {
            XTEN_LOG_ERROR(g_logger) << "backtrace_synbols error";
            return;
        }

        for (size_t i = skip; i < ret; ++i)
        {
            bt.push_back(demangle(strings[i]));
        }

        free(strings);
        free(buffer);
    }
    std::string BackTraceUtil::backtraceTostring(int depth, int skip, const std::string &prefix)
    {
        std::vector<std::string> bt;
        backtrace(bt, depth, skip);
        std::stringstream ss;
        for (int i = 0; i < bt.size(); i++)
        {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
    int64_t FiberUtil::GetFiberId()
    {
        return Fiber::GetFiberId();
    }
    uint64_t TimeUitl::GetCurrentMS()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
    }
    // 获取当前微秒数
    uint64_t TimeUitl::GetCurrentUS()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul * 1000ul + tv.tv_usec;
    }

}