#include "util.h"
#include <execinfo.h>
#include "log.h"
#include "fiber.h"
#include <sys/time.h>
#include<openssl/md5.h>
#include<openssl/sha.h>
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
    //将字面的值转化成底层该4bit真正的内存中的值 （字面值!=内存值）
    static const char xdigit_chars[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,            // index[48-57] 对应 内存二进制值[0-9] 
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,      //index[65-70](大写A-F) 对应 内存二进制值[10-15]
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,      //index[97-102](小写a-f) 对应 内存二进制值[10-15]
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
//当一个字符的值是在为1下标处的时候，这个字符不需要被编码，比如数字，英文字符，等
    static const char uri_chars[256] = {
    /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
    /* 64 */
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
    /* 128 */ //一般需要编码的字符对应一个字节的值一般都大于0x7f
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};
//无需编码字符：-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
#define CHAR_IS_UNRESERVED(c) \
    (uri_chars[(unsigned char)(c)])
    std::string StringUtil::UrlEncode(const std::string &str, bool space_as_plus)
    {
        static const char *hexdigits = "0123456789ABCDEF";
        std::string *ss = nullptr;
        const char *end = str.c_str() + str.length();
        for (const char *c = str.c_str(); c < end; ++c)
        {
            if (!CHAR_IS_UNRESERVED(*c))
            {
                if (!ss)
                {
                    ss = new std::string;
                    ss->reserve(str.size() * 1.2);
                    ss->append(str.c_str(), c - str.c_str());
                }
                if (*c == ' ' && space_as_plus)
                {
                    //空格转+
                    ss->append(1, '+');
                }
                else
                {
                    //汉字转utf-8内存值
                    ss->append(1, '%');
                    ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                    ss->append(1, hexdigits[*c & 0xf]);
                }
            }
            else if (ss)
            {
                ss->append(1, *c);
            }
        }
        if (!ss)
        {
            return str;
        }
        else
        {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }
 std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus)
{
   std::string* ss = nullptr;
   const char* end = str.c_str() + str.length();
   for(const char* c = str.c_str(); c < end; ++c) {
    //url编码会将空格编码成+
       if(*c == '+' && space_as_plus) {
           if(!ss) {
               ss = new std::string;
               ss->append(str.c_str(), c - str.c_str());
           }
           ss->append(1, ' ');
       }
       //对于汉字的编码：%+utf-8编码内存值   '世'-->%E5%8C%97  其中E5 8C 97对应的就是这个汉字的内存中二进制值     
       else if(*c == '%' && (c + 2) < end
                   && isxdigit(*(c + 1)) && isxdigit(*(c + 2))){
           if(!ss) {
               ss = new std::string;
               ss->append(str.c_str(), c - str.c_str());
           }
           //%F5 将一个这个两字节F 5 转化成内存中的该字符字面值
           ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
           c += 2;
       } else if(ss) {
           ss->append(1, *c);
       }
   }
   if(!ss) {
       return str;
   } else {
       std::string rt = *ss;
       delete ss;
       return rt;
   }
}
  std::string StringUtil::Trim(const std::string& str, const std::string& delimit )
  {
        auto begin = str.find_first_not_of(delimit);
      if(begin == std::string::npos) {
              return "";
              }
      auto end = str.find_last_not_of(delimit);
      return str.substr(begin, end - begin + 1);
  }
    std::string Time2Str(time_t ts, const std::string& format)
    {
        struct tm tm;
        localtime_r(&ts, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format.c_str(), &tm);
        return buf;
    }
    std::string sha1sum(std::string data)
    {
        return sha1sum(data.c_str(),data.size());
    }
    std::string sha1sum(const void* data,size_t len)
    {
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx,data,len);
        std::string ret;
        ret.resize(SHA_DIGEST_LENGTH);
        SHA1_Final((unsigned char*)&ret[0],&ctx);
        return ret;
    }

    std::string base64encode(const std::string& data)
    {
        // base64编码的字符集
        const char* base64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        // 经过base64编码后的字符串的长度（减少扩容次数）
        ret.reserve(data.size()*4/3+2); // 注意!!!：不能使用resize 导致是在这些空间之后追加数据
        const unsigned char* begin=(const unsigned char*)data.c_str();
        const unsigned char* end=begin+data.size();
        while(begin<end)
        {
            unsigned int packed=0; //存放一组3字节数据
            int i=0; //i表示当前组的字节个数(不一定满3字节)
            int append=0; //表示要追加的=的数量
            for( ;i<3 && begin<end ; i++,begin++) //每次遍历最多3字节数据
            {
                packed = (packed<<8) | *begin;   
            }
            if(i==1)
            {
                // 这一组只有一个字节数据
                append=2;
            }
            else if(i==2)
            {
                // 这一组只有两字节数据
                append=1;
            }
            for( ; i<3 ;i++)
            {
                packed =packed <<8; //保证即使不满3字节，也会将对应字节数据放到指定位置处
            }
            // 将packed的三字节数据转成4个6位的base64字符
            ret.append(1,base64[packed>>18]); //最高6位转字符
            ret.append(1,base64[(packed>>12) & 0x3F]); //次高6位
            if(append !=2 )
            {
                ret.append(1,base64[(packed>>6) & 0x3F]);
            }
            if(append==0)
            {
                ret.append(1,base64[packed&0x3F]);
            }
            // 在根据append进行追加=
            ret.append(append,'=');
        }
        return ret;
    }
    std::string GetHostName()
    {
        std::shared_ptr<char> buf(new char[512],[](char* ptr){delete[] ptr;});
        memset(buf.get(),0,512);
        gethostname(buf.get(),511);
        return buf.get();
    }

    int64_t TypeUtil::Atoi(const std::string& str) {
        if(str.empty()) {
            return 0;
        }
        return strtoull(str.c_str(), nullptr, 10);
}

std::string replace(const std::string &str1, char find, char replaceWith) {
    auto str = str1;
    size_t index = str.find(find);
    while (index != std::string::npos) {
        str[index] = replaceWith;
        index = str.find(find, index + 1);
    }
    return str;
}

std::string replace(const std::string &str1, char find, const std::string &replaceWith) {
    auto str = str1;
    size_t index = str.find(find);
    while (index != std::string::npos) {
        str = str.substr(0, index) + replaceWith + str.substr(index + 1);
        index = str.find(find, index + replaceWith.size());
    }
    return str;
}

std::string replace(const std::string &str1, const std::string &find, const std::string &replaceWith) {
    auto str = str1;
    size_t index = str.find(find);
    while (index != std::string::npos) {
        str = str.substr(0, index) + replaceWith + str.substr(index + find.size());
        index = str.find(find, index + replaceWith.size());
    }
    return str;
}
}