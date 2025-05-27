#include "../include/util.h"
namespace Xten
{
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
    static int __lstat(const char* dirname){
        //判断文件目录是否存在
        struct stat sta;
        int ret=lstat(dirname,&sta);
        return ret;
    }
    static int __mkdir(const char* dirname){
        //创建目录
        if(access(dirname,F_OK)==0){
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
        { //递归创建多层目录
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

}