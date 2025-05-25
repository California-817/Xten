#pragma once
#include "const.h"
namespace Xten
{
    class FileUtil
    {
    public:
        static bool OpenForWrite(std::ofstream& file_stream,const std::string& file_name,std::ios_base::openmode mode);
    };
    class TimeUitl
    {
    public:
        static uint64_t NowTime_to_uint64();
    };
}