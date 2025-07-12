#include "../include/streams/socket_stream.h"
#include "../include/log.h"
#include <atomic>
namespace Xten
{
    static Logger::ptr g_logger = XTEN_LOG_NAME("system");
    static std::atomic_uint64_t s_id = 0;
    SocketStream::SocketStream(Socket::ptr socket, bool is_owner)
        : _socket(socket), _owner(is_owner), _id(s_id++)
    {
    }
    SocketStream::~SocketStream()
    {
        if (_owner)
        {
            Close();
        }
    }
    void SocketStream::Close()
    {
        if (_socket)
        {
            _socket->Close();
        }
    }
    // 读取数据到buffer中
    ssize_t SocketStream::Read(void *buffer, size_t len)
    {
        if (!IsConnected())
        {
            return -1;
        }
        return _socket->Recv(buffer, len);
    }
    // 读取数据到二进制序列数组bytearray中
    ssize_t SocketStream::Read(ByteArray::ptr ba, size_t len)
    {
        if (!IsConnected())
        {
            return -1;
        }
        // 连接存在
        // 先获取bytearray的指定len长度写缓冲区
        std::vector<iovec> iovs;
        ba->GetWriteBuffers(iovs, len);
        // 此时iovs的内存就是ba中的写缓冲区
        ssize_t ret = _socket->RecvV(&iovs[0], iovs.size());
        // 根据返回值表示的写入数据量设置bytearray的position位置
        if (ret > 0)
        {
            ba->SetPosition(ba->GetPosition() + ret);
        }
        return ret;
    }
    // 将buffer中数据写入
    ssize_t SocketStream::Write(const void *buffer, size_t len)
    {
        if (!IsConnected())
        {
            return -1;
        }
        return _socket->Send(buffer, len);
    }
    // 将二进制序列数组中数据写入
    ssize_t SocketStream::Write(ByteArray::ptr ba, size_t len)
    {
        if (!IsConnected())
        {
            return -1;
        }
        // 获取bytearray的读缓冲区
        std::vector<iovec> iovs;
        ba->GetReadBuffers(iovs, len);
        // iovs指向可读数据缓冲区
        ssize_t ret = _socket->SendV(&iovs[0], iovs.size());
        if (ret > 0)
        {
            ba->SetPosition(ba->GetPosition() + ret);
        }
        else
        {
            XTEN_LOG_ERROR(g_logger) << "write fail length=" << len
                                     << " errno=" << errno << ", " << strerror(errno);
        }
        return ret;
    }
    // 获取socket结构
    Socket::ptr SocketStream::GetSocket()
    {
        return _socket;
    }
    // 获取id
    uint64_t SocketStream::GetId()
    {
        return _id;
    }
    // 获取本地地址
    Address::ptr SocketStream::GetLocalAddr()
    {
        if (_socket)
        {
            return _socket->GetLocalAddress();
        }
        return nullptr;
    }
    // 获取远端地址
    Address::ptr SocketStream::GetPeerAddr()
    {
        if (_socket)
        {
            return _socket->GetPeerAddress();
        }
        return nullptr;
    }
    // 获取本地地址string
    std::string SocketStream::GetLocalAddrString()
    {
        if (_socket)
        {
            return _socket->GetLocalAddress()->toString();
        }
        return "";
    }
    // 获取远端地址string
    std::string SocketStream::GetPeerAddrString()
    {
        if (_socket)
        {
            return _socket->GetPeerAddress()->toString();
        }
        return "";
    }
    // 判断是否建立连接
    bool SocketStream::IsConnected()
    {
        if (_socket)
        {
            return _socket->IsConnected();
        }
        return false;
    }
    // 系统层面检查是否连接
    bool SocketStream::CheckConnected()
    {
        if (_socket)
        {
            return _socket->CheckConnected();
        }
        return false;
    }
}