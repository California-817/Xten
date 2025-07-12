#ifndef __XTEN_SOCKET_STREAM_H__
#define __XTEN_SOCKET_STREAM_H__
#include "../socket.h"
#include "../stream.h"
namespace Xten
{
    class SocketStream : public Stream
    {
    public:
        typedef std::shared_ptr<SocketStream> ptr;
        SocketStream(Socket::ptr socket,bool is_owner=true);
        virtual ~SocketStream();
        virtual void Close() override;
        // 读取数据到buffer中
        virtual ssize_t Read(void *buffer, size_t len) override;
        // 读取数据到二进制序列数组bytearray中
        virtual ssize_t Read(ByteArray::ptr ba, size_t len) override;
        // 将buffer中数据写入
        virtual ssize_t Write(const void *buffer, size_t len) override;
        // 将二进制序列数组中数据写入
        virtual ssize_t Write(ByteArray::ptr ba, size_t len) override;
        // 获取socket结构
        Socket::ptr GetSocket();
        // 获取id
        uint64_t GetId();
        // 获取本地地址
        Address::ptr GetLocalAddr();
        // 获取远端地址
        Address::ptr GetPeerAddr();
        // 获取本地地址string
        std::string GetLocalAddrString();
        // 获取远端地址string
        std::string GetPeerAddrString();
        //判断是否建立连接
        bool IsConnected();
        //系统层面检查是否连接
        bool CheckConnected();
    private:
        Socket::ptr _socket; // socket结构
        uint64_t _id : 63;   // 这个socket对应id（63bit）  位域
        bool _owner : 1;     // 是否主控(1bit)
    };
}
#endif