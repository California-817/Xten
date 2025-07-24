#include "bytearray.h"
#include "log.h"
#include <arpa/inet.h>
#include <endian.h>
#include <iomanip>
#include <sstream>
namespace Xten
{
    static Xten::Logger::ptr g_logger = XTEN_LOG_NAME("system");
    ByteArray::Node::Node()
        : next(nullptr), size(0), memory(nullptr)
    {
    }
    ByteArray::Node::Node(size_t sz)
        : next(nullptr), size(sz), memory(new char[sz])
    {
    }
    ByteArray::Node::~Node()
    {
        if (memory)
        {
            delete[] memory;
        }
    }
    // 对负数进行zigzag压缩编码 32位
    static uint32_t EncodeZigzag32(const int32_t &val)
    {
        if (val < 0)
        {
            // 对于负数的zigzag编码：将符号位移动到最后 + 数值位取反
            //  11111111 11111111 11111111 11111111    ---> -1的补码，以varint方式编码需要5字节（编码效率低，空间占用大）
            // 经过zigzag编码后的uint32位数据为：00000000 00000000 00000000 00000001 --->以varint方式编码只需1字节
            return ((uint32_t)(-val)) * 2 - 1;
        }
        else
            // 正数的zigzag编码：符号位移动到最后（0移动到最后，就是左移一位）
            return val * 2;
    }
    // 对负数进行zigzag压缩编码 64位
    static uint64_t EncodeZigzag64(const int64_t &val)
    {
        if (val < 0)
        {
            return ((uint64_t)(-val)) * 2 - 1;
        }
        else
            return val * 2;
    }
    // 对负数进行zigzag压缩解码 32位
    static int32_t DecodeZigzag32(uint32_t val)
    {
        //-(val & 1)为：全1 or 全0
        return (val >> 1) ^ -(val & 1);
    }
    // 对负数进行zigzag压缩解码 64位
    static int64_t DecodeZigzag64(uint64_t val)
    {
        return (val >> 1) ^ -(val & 1);
    }
    ByteArray::ByteArray(size_t node_size)
        : _root(new Node(node_size)),
          _nodeSize(node_size),
          _position(0),
          _size(0),
          _cur(_root),
          _capacity(node_size),
          _endian(XTEN_BIG_ENDIAN)
    {
    }
    ByteArray::~ByteArray()
    {
        Node *next = nullptr;
        while (_root)
        {
            next = _root->next;
            delete _root;
            _root = next;
        }
        _cur = nullptr;
    }
    // 判断是否是小端
    bool ByteArray::IsLittleEndian()
    {
        return _endian == XTEN_LITTLE_ENDIAN;
    }
    // 设置小端
    void ByteArray::SetLittleEndian(bool val)
    {
        if (val)
        {
            _endian = XTEN_LITTLE_ENDIAN;
        }
        else
        {
            _endian = XTEN_BIG_ENDIAN;
        }
    }
    // 写入固定长度int8_t数据
    void ByteArray::WriteFint8(int8_t val)
    {
        Write(&val, sizeof(int8_t));
    }
    // 写入固定长度uint8_t数据
    void ByteArray::WriteFUint8(uint8_t val)
    {
        Write(&val, sizeof(uint8_t));
    }
    // 写入固定长度int16_t数据
    void ByteArray::WriteFint16(int16_t val)
    {
        // 本机小端 目标大端
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 转大端
            val = (int16_t)htons((uint16_t)(val));
        }
        Write(&val, sizeof(int16_t));
    }
    // 写入固定长度uint16_t数据
    void ByteArray::WriteFUint16(uint16_t val)
    {
        // 本机小端 目标大端
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 转大端
            val = htons(val);
        }
        Write(&val, sizeof(uint16_t));
    }
    // 写入固定长度int32_t数据
    void ByteArray::WriteFint32(int32_t val)
    {
        // 本机小端 目标大端
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 转大端
            val = (int32_t)htonl((uint32_t)(val));
        }
        Write(&val, sizeof(int32_t));
    }
    // 写入固定长度uint32_t数据
    void ByteArray::WriteFUint32(uint32_t val)
    {
        // 本机小端 目标大端
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 转大端
            val = htonl(val);
        }
        Write(&val, sizeof(uint32_t));
    }
    // 写入变长int32_t数据
    void ByteArray::WriteVarint32(int32_t val)
    {
        WriteVarUint32(EncodeZigzag32(val));
    }
    // 写入变长uint32_t数据
    void ByteArray::WriteVarUint32(uint32_t val)
    {
        // varint的思想就是最高位不存放值，用来表示该字节后是否还有字节（32/7取整为5字节）
        uint8_t tmp[5];
        int i = 0;
        while (val >= 0x80) // 10000000 说明数据位数大于7位需要分多个字节存储
        {
            tmp[i++] = (val & 0x7F) | 0x80; // 低7位为数据位，高位的1表示后面还有字节
            val >>= 7;
        }
        // 最后一组数据(小于8位)
        tmp[i++] = val;
        // 对于varint多字节数据的写入 并不需要考虑大小端问题 因为对方解析是按字节为单位解析
        Write(tmp, i);
    }
    // 写入固定长度int64_t数据
    void ByteArray::WriteFint64(int64_t val)
    {
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 主机转大端字节序
            val = htobe64(val);
        }
        Write(&val, sizeof(int64_t));
    }
    // 写入固定长度uint64_t数据
    void ByteArray::WriteFUint64(uint64_t val)
    {
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 主机转大端字节序
            val = htobe64(val);
        }
        Write(&val, sizeof(uint64_t));
    }
    // 写入变长int64_t数据
    void ByteArray::WriteVarint64(int64_t val)
    {
        WriteVarUint64(EncodeZigzag64(val));
    }
    // 写入变长uint64_t数据
    void ByteArray::WriteVarUint64(uint64_t val)
    {
        // 使用varint存储64位数据 (64/7取整为10)
        uint8_t tmp[10];
        int i = 0;
        while (val >= 0x80)
        {
            tmp[i++] = (val & 0x7F) | 0x80;
            val >>= 7;
        }
        tmp[i++] = val;
        Write(tmp, i);
    }
    // 写入float数，定长
    void ByteArray::WriteFloat(float val)
    {
        uint32_t tmp;
        memcpy(&tmp, &val, sizeof(float));
        WriteFUint32(tmp);
    }
    // 写入double数，定长
    void ByteArray::WriteDouble(double val)
    {
        uint64_t tmp;
        memcpy(&tmp, &val, sizeof(double));
        WriteFUint64(tmp);
    }
    // 写入字符串，由uint16保存长度
    void ByteArray::WriteStringF16(const std::string &str)
    {
        // 写入长度
        WriteFUint16(str.size());
        // 写入字符串
        Write(str.c_str(), str.size());
    }
    // 写入字符串，由uint32保存长度
    void ByteArray::WriteStringF32(const std::string &str)
    {
        // 写入长度
        WriteFUint32(str.size());
        // 写入字符串
        Write(str.c_str(), str.size());
    }
    // 写入字符串，由uint64保存长度
    void ByteArray::WriteStringF64(const std::string &str)
    {
        // 写入长度
        WriteFUint32(str.size());
        // 写入字符串
        Write(str.c_str(), str.size());
    }
    // 写入字符串，由varint64保存长度
    void ByteArray::WriteStringVar64(const std::string &str)
    {
        WriteVarUint64(str.size());
        Write(str.c_str(), str.size());
    }
    // 通用的写入数据接口
    void ByteArray::Write(const void *buf, size_t size)
    {
        if (size == 0)
        {
            return;
        }
        // 写入前判断空间是否需要扩容
        addFreeCapacity(size);
        // 当前节点位置
        size_t npos = _position % _nodeSize;
        // 当前节点剩余空间
        size_t ncap = _cur->size - npos;
        // 数据拷贝位置
        size_t cp_pos = 0;
        while (size > 0)
        {
            if (ncap >= size)
            {
                memcpy(_cur->memory + npos, (const char *)buf + cp_pos, size);
                if (ncap == size)
                {
                    // 当前节点空间使用完
                    _cur = _cur->next;
                }
                _position += size;
                cp_pos += size;
                size = 0;
            }
            else
            {
                memcpy(_cur->memory + npos, (const char *)buf + cp_pos, ncap);
                size -= ncap;
                _cur = _cur->next;
                _position += ncap;
                cp_pos += ncap;
                ncap = _cur->size;
                npos = 0;
            }
        }
        // 拷贝完毕
        if (_position > _size)
        {
            _size = _position; // 更新数据大小
        }
    }
    // 返回可读数据大小
    size_t ByteArray::GetReadSize()
    {
        return _size - _position;
    }
    // 通用的读取数据接口
    void ByteArray::Read(void *buf, size_t size)
    {
        if (size > GetReadSize())
        {
            throw std::out_of_range("no enough readSize");
        }
        size_t npos = _position % _nodeSize;
        size_t ncap = _cur->size - npos;
        size_t rd_pos = 0;
        while (size > 0)
        {
            if (size <= ncap)
            {
                memcpy((char *)buf + rd_pos, _cur->memory + npos, size);
                if (size == ncap)
                {
                    _cur = _cur->next;
                }
                rd_pos += size;
                _position += size;
                size = 0;
            }
            else
            {
                memcpy((char *)buf + rd_pos, _cur->memory + npos, ncap);
                rd_pos += ncap;
                _position += ncap;
                size -= ncap;
                _cur = _cur->next;
                ncap = _cur->size;
                npos = 0;
            }
        }
    }
    // 从指定位置开始读取数据 （不更新position和cur的位置）
    void ByteArray::Read(void *buf, size_t size, size_t position)
    {
        if (size > (_size - position))
        {
            throw std::out_of_range("no enough readSize");
        }
        size_t npos = position % _nodeSize;
        size_t rd_pos = 0;
        // 根据position确定cur的Node节点位置
        Node *cur = _root;
        int64_t count = position / _nodeSize;
        while (count > 0)
        {
            cur = cur->next;
            count--;
        }
        size_t ncap = cur->size - npos;
        while (size > 0)
        {
            if (size <= ncap)
            {
                memcpy((char *)buf + rd_pos, cur->memory + npos, size);
                if (size == ncap)
                {
                    cur = cur->next;
                }
                rd_pos += size;
                position += size;
                size = 0;
            }
            else
            {
                memcpy((char *)buf + rd_pos, cur->memory + npos, ncap);
                rd_pos += ncap;
                position += ncap;
                size -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
        }
    }
    // 读取固定长度int8_t数据
    int8_t ByteArray::ReadFint8()
    {
        int8_t tmp;
        Read(&tmp, sizeof(int8_t));
        return tmp;
    }
    // 读取固定长度uint8_t数据
    uint8_t ByteArray::ReadFUint8()
    {
        uint8_t tmp;
        Read(&tmp, sizeof(uint8_t));
        return tmp;
    }

    // 读取固定长度int16_t数据
    int16_t ByteArray::ReadFint16()
    {
        int16_t tmp;
        Read(&tmp, sizeof(tmp));
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 大端转小端
            tmp = (int16_t)ntohs((uint16_t)(tmp));
        }
        return tmp;
    }
    // 读取固定长度uint16_t数据
    uint16_t ByteArray::ReadFUint16()
    {
        uint16_t tmp;
        Read(&tmp, sizeof(tmp));
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 大端转小端
            tmp = ntohs(tmp);
        }
        return tmp;
    }
    // 读取固定长度int32_t数据
    int32_t ByteArray::ReadFint32()
    {
        int32_t tmp;
        Read(&tmp, sizeof(tmp));
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 大端转小端
            tmp = (int32_t)ntohl((uint32_t)tmp);
        }
        return tmp;
    }

    // 读取固定长度uint32_t数据
    uint32_t ByteArray::ReadFUint32()
    {
        uint32_t tmp;
        Read(&tmp, sizeof(tmp));
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 大端转小端
            tmp = ntohl(tmp);
        }
        return tmp;
    }
    // 读取变长int32_t数据
    int32_t ByteArray::ReadVarint32()
    {
        uint32_t tmp = ReadVarUint32();
        return DecodeZigzag32(tmp);
    }
    // 读取变长uint32_t数据
    uint32_t ByteArray::ReadVarUint32()
    {
        uint32_t val = 0;
        int i = 0;
        while (true)
        {
            uint8_t tmp = ReadFUint8(); // 每次读取一个字节数据
            if (tmp < 0x80)
            {
                // 该数据后面没有字节了
                val |= ((uint32_t)tmp) << i;
                break;
            }
            // 该字节后还有数据
            else
            {
                val |= (((uint32_t)(tmp & 0x7F)) << i);
                i += 7;
            }
        }
        return val;
    }

    // 读取固定长度int64_t数据
    int64_t ByteArray::ReadFint64()
    {
        int64_t tmp;
        Read(&tmp, sizeof(tmp));
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 大端转小端
            tmp = be64toh(tmp);
        }
        return tmp;
    }
    // 读取固定长度uint64_t数据
    uint64_t ByteArray::ReadFUint64()
    {
        uint64_t tmp;
        Read(&tmp, sizeof(tmp));
        if (_endian != XTEN_BYTE_ORDER)
        {
            // 大端转小端
            tmp = be64toh(tmp);
        }
        return tmp;
    }
    // 读取变长int64_t数据
    int64_t ByteArray::ReadVarint64()
    {
        uint64_t tmp = ReadVarUint64();
        return DecodeZigzag64(tmp);
    }
    // 读取变长uint64_t数据
    uint64_t ByteArray::ReadVarUint64()
    {
        uint64_t val = 0;
        int i = 0;
        while (true)
        {
            uint8_t tmp = ReadFUint8(); // 每次读取一个字节数据
            if (tmp < 0x80)
            {
                // 该数据后面没有字节了
                val |= ((uint64_t)tmp) << i;
                break;
            }
            // 该字节后还有数据
            else
            {
                val |= (((uint64_t)(tmp & 0x7F)) << i);
                i += 7;
            }
        }
        return val;
    }
    // 读取float数，定长
    float ByteArray::ReadFloat()
    {
        float val;
        uint32_t tmp = ReadFUint32();
        memcpy(&val, &tmp, sizeof(val));
        return val;
    }
    // 读取double数，定长
    double ByteArray::ReadDouble()
    {
        double val;
        uint64_t tmp = ReadFUint64();
        memcpy(&val, &tmp, sizeof(val));
        return val;
    }
    // 读取字符串，由uint16保存长度
    std::string ByteArray::ReadStringF16()
    {
        uint16_t len = ReadFUint16();
        char buf[len + 1];
        buf[len] = '\0'; // 最后一个字符设置为\0
        Read(buf, len);
        return std::string(buf);
    }
    // 读取字符串，由uint32保存长度
    std::string ByteArray::ReadStringF32()
    {
        uint32_t len = ReadFUint32();
        char buf[len + 1];
        buf[len] = '\0'; // 最后一个字符设置为\0
        Read(buf, len);
        return std::string(buf);
    }
    // 读取字符串，由uint64保存长度
    std::string ByteArray::ReadStringF64()
    {
        uint64_t len = ReadFUint64();
        char buf[len + 1];
        buf[len] = '\0'; // 最后一个字符设置为\0
        Read(buf, len);
        return std::string(buf);
    }
    // 读取字符串，由varint64保存长度
    std::string ByteArray::ReadStringVar64()
    {
        uint64_t len = ReadVarUint64();
        char buf[len + 1];
        buf[len] = '\0';
        Read(buf, len);
        return std::string(buf);
    }
    void ByteArray::Clear()
    {
        _position = _size = 0;
        Node *tmp = _root->next;
        _root->next = nullptr;
        while (tmp)
        {
            _cur = tmp->next;
            delete tmp;
            tmp = _cur;
        }
        _cur = _root;
        _capacity = _nodeSize; // 只有一个节点
    }
    // 设置当前bytearray的position
    void ByteArray::SetPosition(size_t pos)
    {
        if (pos > _capacity)
        {
            throw std::out_of_range("position out of range");
        }
        _position = pos;
        if (_position > _size)
        {
            _size = _position;
        }
        // 根据新的position设置cur位置
        _cur = _root;
        while (pos > _cur->size)
        {
            pos -= _cur->size;
            _cur = _cur->next;
        }
        if (pos == _cur->size)
        {
            _cur = _cur->next;
        }
    }
    // 将bytearray里的[_position,_size)的数据转换成字符串
    std::string ByteArray::ToString()
    {
        std::string str;
        str.resize(GetReadSize());
        if (str.empty())
        {
            return str;
        }
        // 读取数据 但是不会改变position和cur的位置
        Read(&str[0], str.size(), _position);
        return str;
    }
    // 将bytearray里的[_position,_size)的数据转换成16进制表示 (格式：FF 11 22 3a )
    std::string ByteArray::ToHexString()
    {
        std::string str = ToString();
        std::stringstream ss;
        for (size_t i = 0; i < str.size(); i++)
        {
            if (i > 0 && i % 32 == 0)
            {
                ss << std::endl; // 一行32字节 每个字节都是 F3 这种格式
            }
            // 设置宽度2  不足填充0   以16进制输出
            ss << std::setw(2) << std::setfill('0') << std::hex << (int)(uint8_t)str[i] << " ";
        }
        return ss.str();
    }
    // 将该bytearray中的数据写入文件
    bool ByteArray::WriteToFile(const std::string &file, bool with_md5)
    {
        std::ofstream fs;
        fs.open(file, std::ios::trunc | std::ios::binary);
        if (!fs.is_open())
        {
            XTEN_LOG_ERROR(g_logger) << "WriteToFile name=" << file
                                     << " error , errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        int64_t position = _position;
        size_t npos = position % _nodeSize;
        Node *cur = _cur;
        int64_t readSize = GetReadSize();
        size_t ncap = cur->size - npos;
        while (readSize > 0)
        {
            if (readSize <= ncap)
            {
                // 当前一次读完
                fs.write(cur->memory + npos, readSize);
                readSize = 0;
            }
            else
            {
                fs.write(cur->memory + npos, ncap);
                position += ncap;
                readSize -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
        }
        if (with_md5)
        {
            std::ofstream ofs_md5(file + ".md5");
            ofs_md5 << getMd5();
        }
        return true;
    }
    // 从文件读取数据到该bytearray
    bool ByteArray::ReadFromFile(const std::string &file)
    {
        std::ifstream ifs;
        ifs.open(file, std::ios::binary);
        if (!ifs.is_open())
        {
            XTEN_LOG_ERROR(g_logger) << "ReadFromFile name=" << file
                                     << " error , errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }
        std::shared_ptr<char> buffer(new char[_nodeSize], [](char *ptr)
                                     { delete[] ptr; });
        while (!ifs.eof()) // 未到文件结尾
        {
            // 读取数据到buffer中
            ifs.read(buffer.get(), _nodeSize);
            // gcount 返回上一次输入操作（如 read 或 get）中实际读取的字符数
            Write(buffer.get(), ifs.gcount());
        }
        return true;
    }
    // 获取可用空间的大小
    size_t ByteArray::getFreeCapacity()
    {
        return _capacity - _position;
    }
    // 可用空间扩容(看size是否大于剩余free空间)
    void ByteArray::addFreeCapacity(size_t size)
    {
        // 写入数据空间小于剩余free空间
        size_t old_cap = getFreeCapacity();
        if (size <= old_cap)
        {
            return;
        }
        // 写入数据大于剩余空间 扩容
        size = size - old_cap;
        // 计算扩容节点个数，小数向上取整
        size_t count = ceil((1.0 * size) / _nodeSize);
        Node *tmp = _root;
        while (tmp->next)
        {
            tmp = tmp->next;
        }
        // 此时tmp指向最后一个节点
        Node *first = nullptr;
        for (int i = 0; i < count; i++)
        {
            tmp->next = new Node(_nodeSize);
            if (first == nullptr)
            {
                first = tmp->next; // 记录新增的第一个节点
            }
            tmp = tmp->next;
            _capacity += _nodeSize;
        }
        if (old_cap == 0)
        {
            _cur = first; // 更新操作节点位置
        }
    }
    // 获取指定len长度的Node缓冲区到buffer中
    uint64_t ByteArray::GetReadBuffers(std::vector<iovec> &buffers, uint64_t len)
    {
        len = GetReadSize() > len ? len : GetReadSize();
        if (len == 0)
        {
            return 0;
        }
        size_t size = len;
        size_t npos = _position % _nodeSize;
        size_t ncap = _cur->size - npos;
        Node *cur = _cur;
        struct iovec iov;
        while (len > 0)
        {
            if (len <= ncap)
            {
                iov.iov_base = cur->memory + npos;
                iov.iov_len = len;
                len = 0;
            }
            else
            {
                iov.iov_base = cur->memory + npos;
                iov.iov_len = ncap;
                len -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
            buffers.push_back(iov);
        }
        return size;
    }
    // 从pos位置获取指定len长度的Node缓冲区到buffer中
    uint64_t ByteArray::GetReadBuffers(std::vector<iovec> &buffers, uint64_t len, size_t pos)
    {
        // 看len是否可读数据的长度
        size_t can_read = _size - pos;
        len = can_read > len ? len : can_read;
        if (len == 0)
        {
            return 0;
        }
        size_t size = len;
        Node *cur = _root;
        size_t npos = pos % _nodeSize;
        int64_t count = pos / _nodeSize;
        while (count > 0)
        {
            cur = cur->next;
            count--;
        }
        size_t ncap = cur->size - npos;
        struct iovec iov;
        while (len > 0)
        {
            if (len <= ncap)
            {
                iov.iov_base = cur->memory + npos;
                iov.iov_len = len;
                len = 0;
            }
            else
            {
                iov.iov_base = cur->memory + npos;
                iov.iov_len = ncap;

                len -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
            buffers.push_back(iov);
        }
        return size;
    }
    // 获取指定len大小写入数据缓冲区到buffers中
    uint64_t ByteArray::GetWriteBuffers(std::vector<iovec> &buffers, uint64_t len)
    {
        if (len == 0)
        {
            return 0;
        }
        addFreeCapacity(len);
        size_t size = len;
        size_t npos = _position % _nodeSize;
        size_t ncap = _cur->size - npos;
        Node *cur = _cur;
        struct iovec iov;
        while (len > 0)
        {
            if (len <= ncap)
            {
                iov.iov_base = cur->memory + npos;
                iov.iov_len = ncap;
                len = 0;
            }
            else
            {
                iov.iov_base = cur->memory + npos;
                iov.iov_len = ncap;
                len -= ncap;
                cur = cur->next;
                ncap = cur->size;
                npos = 0;
            }
            buffers.push_back(iov);
        }
        return size;
    }
    // 获取当前bytearray中数据的md5值
    std::string ByteArray::getMd5()
    {
        // Todo
        return "MD5";
    }
}