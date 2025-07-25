#include "zlib_stream.h"
namespace Xten
{
    ZlibStream::ptr ZlibStream::CreateZlib(bool encode, uint32_t iovSize)
    {
        return Create(encode, iovSize, ZLIB_Type::ZLIB);
    }
    ZlibStream::ptr ZlibStream::Creategzip(bool encode, uint32_t iovSize)
    {
        return Create(encode, iovSize, ZLIB_Type::GZIP);
    }
    ZlibStream::ptr ZlibStream::CreateDeflate(bool encode, uint32_t iovSize)
    {
        return Create(encode, iovSize, ZLIB_Type::DEFLATE);
    }
    ZlibStream::ptr ZlibStream::Create(bool encode, uint32_t buff_size,
                                       ZLIB_Type type, ZLIB_CompressLevel level,
                                       int window_bits, int memlevel, ZLIB_Strategy strategy)
    {
        ZlibStream::ptr zstream = std::make_shared<ZlibStream>(encode, buff_size);
        if (zstream)
        {
            zstream->Init(type, strategy, level, window_bits, memlevel);
        }
        return zstream;
    }
    ZlibStream::ZlibStream(bool encode, uint32_t iovSize)
        : _isEncode(encode),
          _iovSize(iovSize)
    {
    }
    // 初始化这个zstream
    int ZlibStream::Init(ZLIB_Type type, ZLIB_Strategy strategy, ZLIB_CompressLevel level,
                         int window_bits, int memlevel)
    {
        XTEN_ASSERT((level >= 0 && level <= 9) || level == DEFAULT_COMPRESSION);
        XTEN_ASSERT((window_bits >= 8 && window_bits <= 15));
        XTEN_ASSERT((memlevel >= 1 && memlevel <= 9));
        memset(&_zstream, 0, sizeof(_zstream));
        _zstream.zalloc = Z_NULL;
        _zstream.zfree = Z_NULL;
        _zstream.opaque = Z_NULL;
        // 复用窗口大小来区分压缩格式
        switch (type)
        {
        case ZLIB_Type::DEFLATE:
            window_bits = -window_bits;
            break;
        case ZLIB_Type::GZIP:
            window_bits += 16;
            break;
        case ZLIB_Type::ZLIB:
        default:
            break;
        }
        if (_isEncode)
        {
            return deflateInit2(&_zstream, level, Z_DEFLATED, window_bits, memlevel, (int)strategy);
        }
        else
        {
            return inflateInit2(&_zstream, window_bits);
        }
    }
    // 将buffer中数据写入(进行编码)
    ssize_t ZlibStream::Write(const void *buffer, size_t len)
    {
        iovec iov;
        iov.iov_base = (void *)buffer;
        iov.iov_len = len;
        if (_isEncode)
        {
            // 编码操作
            return encode(&iov, 1, false);
        }
        else
        {
            // 解码操作
            return decode(&iov, 1, false);
        }
    }
    // 将二进制序列数组中数据写入(进行编码)
    ssize_t ZlibStream::Write(ByteArray::ptr ba, size_t len)
    {
        std::vector<iovec> iovs;
        ba->GetReadBuffers(iovs, len);
        if (_isEncode)
        {
            return encode(&iovs[0], iovs.size(), false);
        }
        else
        {
            return decode(&iovs[0], iovs.size(), false);
        }
    }
    // 编码函数
    int ZlibStream::encode(const iovec *iovs, const size_t &len, bool finish)
    {
        int rlflush = 0;
        int ret = 0;
        // 循环遍历iovec数据进行编码
        for (int i = 0; i < len; i++)
        {
            // 设置输入数据（内部处理一定数据后自动更新这两个值）
            _zstream.next_in = (Bytef *)iovs[i].iov_base;
            _zstream.avail_in = iovs[i].iov_len;
            rlflush = finish ? (i == len - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;
            // 对一个iovec进行压缩编码可能需要多次进行（输出缓冲区不一定一次就够了）
            iovec *out = nullptr;
            do
            {
                if (!_outBuffer.empty() && _outBuffer.back().iov_len < _iovSize)
                {
                    // 可以复用最后一个iovec
                    out = &_outBuffer.back();
                }
                else
                {
                    // 不可复用
                    iovec iov;
                    iov.iov_base = (void *)new char[_iovSize];
                    iov.iov_len = 0; // 已用大小
                    _outBuffer.push_back(iov);
                    out = &_outBuffer.back();
                }
                // 拿到了输出缓冲区
                _zstream.next_out = (Bytef *)((char *)out->iov_base + out->iov_len); // 位置
                _zstream.avail_out = _iovSize - out->iov_len;                        // 大小
                // 进行压缩
                ret = deflate(&_zstream, rlflush); // 不会重复压缩（内部维护了压缩进度）
                if (ret == Z_STREAM_ERROR)
                {
                    // 压缩出错了
                    return ret;
                }
                out->iov_len = _iovSize - _zstream.avail_out; // 更新已用数据长度
            } while (_zstream.avail_out == 0); // 说明这此压缩out缓冲区满了
        }
        if (rlflush == Z_FINISH)
        {
            deflateEnd(&_zstream);
        }
        return Z_OK;
    }
    // 解码函数
    int ZlibStream::decode(const iovec *iovs, const size_t &len, bool finish)
    {
        int ret = 0;
        int flush = 0;
        for (int i = 0; i < len; i++)
        {
            _zstream.next_in = (Bytef *)iovs[i].iov_base;
            _zstream.avail_in = iovs[i].iov_len;
            flush = finish ? (i == len - 1 ? Z_FINISH : Z_NO_FLUSH) : Z_NO_FLUSH;
            iovec *out = nullptr;
            do
            {
                if (!_outBuffer.empty() && _outBuffer.back().iov_len < _iovSize)
                {
                    out = &_outBuffer.back();
                }
                else
                {
                    iovec iov;
                    iov.iov_base = (void *)new char[_iovSize];
                    iov.iov_len = 0;
                    _outBuffer.push_back(iov);
                    out = &_outBuffer.back();
                }
                _zstream.avail_out = _iovSize - out->iov_len;
                _zstream.next_out = (Bytef *)((char *)out->iov_base + out->iov_len);
                ret = inflate(&_zstream, flush);
                if (ret == Z_STREAM_ERROR)
                {
                    return ret;
                }
                out->iov_len = _iovSize - _zstream.avail_out;
            } while (_zstream.avail_out == 0); // 再次输出
        }
        if (flush == Z_FINISH)
        {
            inflateEnd(&_zstream);
        }
        return Z_OK;
    }
    // 刷新zstream缓存的处理过的数据到outBuffer中
    int ZlibStream::Flush()
    {
        iovec iov;
        iov.iov_base = nullptr;
        iov.iov_len = 0;
        if (_isEncode)
        {
            return encode(&iov, 1, true);
        }
        else
        {
            return decode(&iov, 1, true);
        }
    }
    // 获取zlib处理后数据string
    std::string ZlibStream::GetResult()
    {
        uint32_t total = 0;
        for (auto &iov : _outBuffer)
        {
            total += iov.iov_len;
        }
        std::string result;
        result.reserve(total);
        for (auto &iov : _outBuffer)
        {
            result.append((char *)iov.iov_base, iov.iov_len);
        }
        return result;
    }
    // 获取zlib处理后数据并返回bytearray
    ByteArray::ptr ZlibStream::GetByteArray()
    {
        ByteArray::ptr ba = std::make_shared<ByteArray>();
        for (auto &iov : _outBuffer)
        {
            ba->Write(iov.iov_base, iov.iov_len);
        }
        ba->SetPosition(0); // 重置position位置
        return ba;
    }
    // 关闭流结构
    void ZlibStream::Close()
    {
        Flush();
    }
    ZlibStream::~ZlibStream()
    {
        for (auto &iov : _outBuffer)
        {
            free(iov.iov_base);
        }
        _outBuffer.clear();
        if (_isEncode)
        {
            deflateEnd(&_zstream);
        }
        else
        {
            inflateEnd(&_zstream);
        }
    }
}