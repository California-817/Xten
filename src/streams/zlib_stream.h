#include "../stream.h"
#include <list>
#include <zlib.h>
#include "../macro.h"

// LZ77压缩编码原理（滑动窗口+引用指针）
// (offset,length,char):
// 1.offset表示当前位置到前面字典中匹配字符偏移量 2.字典中匹配字符串的长度 3.当字典中没有匹配字符时,(0,0,char)表示
// +----------------------------------------------------------------------------------------------------+
// |                                              滑动窗口                                               |
// +----------------------------------------------------------------------------------------------------+
// |                      查找缓存区(已编码)                |               先行缓存区(未编码)             |
// |                        Dictionary(字典)               |              look ahead buffer             |
// +-------------------------------------+-----------------+--------------------------------------------+------+
// |      |      |      |      |    |    | A   |  A |  B   | C    | B    | B    | A    | B    | C    |  .....  |
// +-------------------------------------+-----------------+--------------------------------------------+------+
// ------------<<<<<<<<<<<<<<<<<<<<<<<------------data移动方向-------------<<<<<<<<<<<<<<<<<<<-------------------
//滑动窗口小：压缩速度快，压缩后文件大（有较少的重复序列）
//滑动窗口大：压缩速度慢，压缩后文件小（花较长时间寻找重复序列）

namespace Xten
{
    class ZlibStream : public Stream
    {
    public:
        typedef std::shared_ptr<ZlibStream> ptr;
        // 压缩格式
        enum ZLIB_Type
        {
            ZLIB,
            GZIP,
            DEFLATE
        };
        // 压缩策略
        enum ZLIB_Strategy
        {
            DEFAULT = Z_DEFAULT_STRATEGY,
            FILTERED = Z_FILTERED,
            HUFFMAN = Z_HUFFMAN_ONLY,
            FIXED = Z_FIXED,
            RLE = Z_RLE
        };
        // 压缩级别(影响压缩速度和压缩率)
        enum ZLIB_CompressLevel
        {
            NO_COMPRESSION = Z_NO_COMPRESSION,
            BEST_SPEED = Z_BEST_SPEED,
            BEST_COMPRESSION = Z_BEST_COMPRESSION,
            DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
        };
        //静态工厂方法创建zlib压缩流（三种压缩格式）
        static ZlibStream::ptr CreateZlib(bool encode,uint32_t iovSize=4096);
        static ZlibStream::ptr Creategzip(bool encode,uint32_t iovSize=4096);
        static ZlibStream::ptr CreateDeflate(bool encode,uint32_t iovSize=4096);
        static ZlibStream::ptr Create(bool encode, uint32_t buff_size = 4096,
                               ZLIB_Type type = DEFLATE, ZLIB_CompressLevel level = DEFAULT_COMPRESSION,
                               int window_bits = 15, int memlevel = 8, ZLIB_Strategy strategy = DEFAULT);

        ZlibStream(bool encode, uint32_t iovSize = 4096);
        // 初始化这个zstream
        int Init(ZLIB_Type type = DEFLATE, ZLIB_Strategy strategy = DEFAULT, ZLIB_CompressLevel level = DEFAULT_COMPRESSION,
                 int window_bits = 15, int memlevel = 8);
        // 关闭流结构
        virtual void Close() override;
        virtual ~ZlibStream();
        // 读取数据到buffer中
        virtual ssize_t Read(void *buffer, size_t len)
        {
            XTEN_ASSERTINFO(false, "ZlibStream::Read form buffer is Invaild");
        }
        // 读取数据到二进制序列数组bytearray中
        virtual ssize_t Read(ByteArray::ptr ba, size_t len)
        {
            XTEN_ASSERTINFO(false, "ZlibStream::Read form Bytearray is Invaild");
        }
        // 将buffer中数据写入(进行压缩或者解压编码)
        virtual ssize_t Write(const void *buffer, size_t len) override;
        // 将二进制序列数组中数据写入(进行压缩或者解压编码)
        virtual ssize_t Write(ByteArray::ptr ba, size_t len) override;
        // 刷新zstream缓存的处理过的数据
        int Flush();
        // 获取zlib处理后数据string
        std::string GetResult();
        // 获取zlib处理后数据并返回bytearray
        ByteArray::ptr GetByteArray();

    private:
        // 编码函数
        int encode(const iovec *iovs, const size_t &len, bool finish);
        // 解码函数
        int decode(const iovec *iovs, const size_t &len, bool finish);

    private:
        z_stream _zstream;           // 压缩流结构
        std::list<iovec> _outBuffer; // 编解码后数据输出缓冲区
        uint32_t _iovSize;           // 标准的iovec缓冲区大小
        bool _isEncode;              // 是否压缩编码or解压解码
    };
}