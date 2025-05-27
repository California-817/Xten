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
    
}