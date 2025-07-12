#include "../include/stream.h"
#include "../include/log.h"
#include "../include/config.h"
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    // 默认缓冲区的最大长度是16KB
    static ConfigVar<int32_t>::ptr g_buffer_size = Config::LookUp("Stream_buffer.size",
                                                                  (int32_t)1024 * 16, "stream buffer size");
    static int32_t s_bufferSize = g_buffer_size->GetValue();
    namespace
    {
        struct Init
        {
            Init()
            {
                g_buffer_size->AddListener([](const int32_t &old_val, const int32_t &new_val)
                                           {
                    XTEN_LOG_INFO(g_logger)<<"Stream_buffer_size changed from "<<old_val
                                            <<" to "<<new_val;
                                            s_bufferSize=new_val; });
            }
        };
        Init __init;
    }
    // 读取指定长度数据到buffer中
    ssize_t Stream::ReadFixSize(void *buffer, size_t len)
    {
        size_t offset = 0;
        size_t left = len;
        while (left > 0)
        {
            ssize_t tmp = Read((char *)buffer + offset, std::min(left, (size_t)s_bufferSize));
            if (tmp <= 0)
            {
                XTEN_LOG_ERROR(g_logger) << "ReadFixSize fail length=" << len
                                         << " len=" << tmp << " errno=" << errno << " errstr=" << strerror(errno);
                return tmp;
            }
            offset += tmp;
            left -= tmp;
        }
        return len;
    }
    // 读取指定长度数据到二进制序列数组bytearray中
    ssize_t Stream::ReadFixSize(ByteArray::ptr ba, size_t len)
    {
        size_t left = len;
        while (left > 0)
        {
            ssize_t tmp = Read(ba, std::min(left, (size_t)s_bufferSize));
            if (tmp <= 0)
            {
                XTEN_LOG_ERROR(g_logger) << "ReadFixSize fail length=" << len
                                         << " len=" << tmp << " errno=" << errno << " errstr=" << strerror(errno);
                return tmp;
            }
            left -= tmp;
        }
        return len;
    }
    // 将buffer中指定长度数据写入
    ssize_t Stream::WriteFixSize(const void *buffer, size_t len)
    {
        size_t offset = 0;
        size_t left = len;
        while (left > 0)
        {
            ssize_t tmp = Write((char *)buffer + offset, std::min(left, (size_t)s_bufferSize));
            if (tmp <= 0)
            {
                XTEN_LOG_ERROR(g_logger) << "WriteFixSize fail length=" << len
                                         << " len=" << tmp << " errno=" << errno << " errstr=" << strerror(errno);
                return tmp;
            }
            offset += tmp;
            left -= tmp;
        }
        return len;
    }
    // 将二进制序列数组中指定长度数据写入
    ssize_t Stream::WriteFixSize(ByteArray::ptr ba, size_t len)
    {
        size_t left = len;
        while (left > 0)
        {
            ssize_t tmp = Write(ba, std::min(left, (size_t)s_bufferSize));
            if (tmp <= 0)
            {
                XTEN_LOG_ERROR(g_logger) << "WriteFixSize fail length=" << len
                                         << " len=" << tmp << " errno=" << errno << " errstr=" << strerror(errno);
                return tmp;
            }
            left -= tmp;
        }
        return len;
    }
}