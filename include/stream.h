#ifndef __XTEN_STREAM_H__
#define __XTEN_STREAM_H__
#include <memory>
#include "bytearray.h"
namespace Xten
{
    class Stream
    {
    public:
        typedef std::shared_ptr<Stream> ptr;
        Stream()=default;
        // 关闭流结构
        virtual void Close() = 0;
        virtual ~Stream()=default;
        
        // 读取数据到buffer中
        virtual ssize_t Read(void *buffer, size_t len) = 0;
        // 读取数据到二进制序列数组bytearray中
        virtual ssize_t Read(ByteArray::ptr ba, size_t len) = 0;
        // 将buffer中数据写入
        virtual ssize_t Write(const void *buffer, size_t len) = 0;
        // 将二进制序列数组中数据写入
        virtual ssize_t Write(ByteArray::ptr ba, size_t len) = 0;

        // 读取指定长度数据到buffer中 (retval<=0失败 retval==len成功)
        virtual ssize_t ReadFixSize(void *buffer, size_t len);
        // 读取指定长度数据到二进制序列数组bytearray中 (retval<=0失败 retval==len成功)
        virtual ssize_t ReadFixSize(ByteArray::ptr ba, size_t len);
        // 将buffer中指定长度数据写入 (retval<=0失败 retval==len成功)
        virtual ssize_t WriteFixSize(const void *buffer, size_t len);
        // 将二进制序列数组中指定长度数据写入 (retval<=0失败 retval==len成功)
        virtual ssize_t WriteFixSize(ByteArray::ptr ba, size_t len);
    };
}
#endif