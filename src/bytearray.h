#ifndef __XTEN_BYETEARRAY_H__
#define __XTEN_BYETEARRAY_H__
#include <vector>
#include <string>
#include <memory>
#include <endian.h>
#include <sys/types.h>
#include <sys/socket.h>
#define XTEN_LITTLE_ENDIAN 1
#define XTEN_BIG_ENDIAN 2
// 判断本机字节序
#if BYTE_ORDER == BIG_ENDIAN
#define XTEN_BYTE_ORDER XTEN_BIG_ENDIAN
#else
#define XTEN_BYTE_ORDER XTEN_LITTLE_ENDIAN
#endif
namespace Xten
{
    class ByteArray
    {
    public:
        typedef std::shared_ptr<ByteArray> ptr;
        // 构造函数 默认节点空间为4kB
        ByteArray(size_t node_size = 4096);
        ~ByteArray();
        // 判断是否是小端
        bool IsLittleEndian();
        // 设置小端
        void SetLittleEndian(bool val);
        // 通用的写入数据接口
        void Write(const void *buf, size_t size);
        // 通用的读取数据接口
        void Read(void *buf, size_t size);
        // 从指定位置开始读取数据（不更新position和cur的位置）
        void Read(void *buf, size_t size, size_t position);
        // 返回可读数据大小
        size_t GetReadSize();
        // 清空bytearray _position和_size置零(只留下一个Node节点) 而不是真正清楚数据
        void Clear();
        // 获取当前的bytearray的position
        size_t GetPosition() const
        {
            return _position;
        }
        //获取第一个节点的内存空间的首地址指针
        void* GetBeginNodePtr() const
        {
            if(!_root)
                return nullptr;
            return _root->memory;
        }
        // 获取当前bytearray的Node大小
        size_t GetNodeSize() const
        {
            return _nodeSize;
        }
        // 返回数据的长度
        size_t GetSize() const
        {
            return _size;
        }
        // 设置当前bytearray的position
        void SetPosition(size_t pos);
        // 将bytearray里的[_position,_size)的数据转换成字符串
        std::string ToString();
        // 将bytearray里的[_position,_size)的数据转换成16进制表示 (格式：FF 11 22 3a )
        std::string ToHexString();
        // 将该bytearray中的数据写入文件（不改变position和cur的位置，从position开始）
        bool WriteToFile(const std::string &file, bool with_md5 = false);
        // 从文件读取数据到该bytearray,从position开始 (改变position和cur的位置)
        bool ReadFromFile(const std::string &file);
        // 获取指定len长度的Node缓冲区到buffer中
        uint64_t GetReadBuffers(std::vector<iovec> &buffers, uint64_t len);
        // 从pos位置获取指定len长度的Node缓冲区到buffer中
        uint64_t GetReadBuffers(std::vector<iovec> &buffers, uint64_t len, size_t pos);
        // 获取指定len大小写入数据缓冲区到buffers中
        uint64_t GetWriteBuffers(std::vector<iovec> &buffers, uint64_t len);
        // 写入固定长度int8_t数据
        void WriteFint8(int8_t val);
        // 写入固定长度uint8_t数据
        void WriteFUint8(uint8_t val);

        // 写入固定长度int16_t数据
        void WriteFint16(int16_t val);
        // 写入固定长度uint16_t数据
        void WriteFUint16(uint16_t val);

        // 写入固定长度int32_t数据
        void WriteFint32(int32_t val);
        // 写入固定长度uint32_t数据
        void WriteFUint32(uint32_t val);
        // 写入变长int32_t数据
        void WriteVarint32(int32_t val);
        // 写入变长uint32_t数据
        void WriteVarUint32(uint32_t val);

        // 写入固定长度int64_t数据
        void WriteFint64(int64_t val);
        // 写入固定长度uint64_t数据
        void WriteFUint64(uint64_t val);
        // 写入变长int64_t数据
        void WriteVarint64(int64_t val);
        // 写入变长uint64_t数据
        void WriteVarUint64(uint64_t val);

        // 写入float数，定长
        void WriteFloat(float val);
        // 写入double数，定长
        void WriteDouble(double val);

        // 写入字符串，由uint16保存长度
        void WriteStringF16(const std::string &str);
        // 写入字符串，由uint32保存长度
        void WriteStringF32(const std::string &str);
        // 写入字符串，由uint64保存长度
        void WriteStringF64(const std::string &str);
        // 写入字符串，由varint64保存长度
        void WriteStringVar64(const std::string &str);

        // 读取固定长度int8_t数据
        int8_t ReadFint8();
        // 读取固定长度uint8_t数据
        uint8_t ReadFUint8();

        // 读取固定长度int16_t数据
        int16_t ReadFint16();
        // 读取固定长度uint16_t数据
        uint16_t ReadFUint16();

        // 读取固定长度int32_t数据
        int32_t ReadFint32();
        // 读取固定长度uint32_t数据
        uint32_t ReadFUint32();
        // 读取变长int32_t数据
        int32_t ReadVarint32();
        // 读取变长uint32_t数据
        uint32_t ReadVarUint32();

        // 读取固定长度int64_t数据
        int64_t ReadFint64();
        // 读取固定长度uint64_t数据
        uint64_t ReadFUint64();
        // 读取变长int64_t数据
        int64_t ReadVarint64();
        // 读取变长uint64_t数据
        uint64_t ReadVarUint64();

        // 读取float数，定长
        float ReadFloat();
        // 读取double数，定长
        double ReadDouble();

        // 读取字符串，由uint16保存长度
        std::string ReadStringF16();
        // 读取字符串，由uint32保
        std::string ReadStringF32();
        // 读取字符串，由uint64保存长度
        std::string ReadStringF64();
        // 读取字符串，由varint64保存长度
        std::string ReadStringVar64();

    private:
        // 获取可用空间的大小
        size_t getFreeCapacity();
        // 可用空间扩容(看size是否大于剩余free空间)
        void addFreeCapacity(size_t size);
        // 获取当前bytearray中数据的md5值
        std::string getMd5();

    private:
        // 存放数据的节点(链表组织)
        struct Node
        {
            Node();
            Node(size_t sz);
            ~Node();
            // 内存地址
            char *memory;
            // 内存大小
            size_t size;
            // 下一个节点
            Node *next;
        };
        // 每个节点大小
        size_t _nodeSize;
        // 总空间容量
        size_t _capacity;
        // 总的填入数据大小 （该位置还未写入数据）
        size_t _size;
        // 当前操作的位置(read和write操作都会移动该position)该位置还未写入或者被读取
        size_t _position;
        // 字节序
        int8_t _endian;
        // 头结点
        Node *_root;
        // 当前操作节点
        Node *_cur;
    };
}
#endif