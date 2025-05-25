#include "../include/util.h"
namespace Xten
{
    // 打开文件读
    bool FileUtil::OpenForWrite(std::ofstream &file_stream, const std::string &file_name, std::ios_base::openmode mode)
    {
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