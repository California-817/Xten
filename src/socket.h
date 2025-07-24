#ifndef __XTEN_SOCKET_H__
#define __XTEN_SOCKET_H__
#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "nocopyable.hpp"
#include "address.h"
namespace Xten
{
    /// @brief 封装socket结构
    class Socket : public std::enable_shared_from_this<Socket>, public NoCopyable
    {
    public:
        typedef std::shared_ptr<Socket> ptr;
        enum TYPE
        {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM
        };
        enum FAMILY
        {
            IPv4 = AF_INET,
            IPv6 = AF_INET6,
            UNIX = AF_UNIX
        };
        //提供静态工厂方法快速创建Socket
        // 创建ipv4tcp套接字
        static Socket::ptr CreateTCP(Address::ptr addr);
        // 创建ipv4udp套接字
        static Socket::ptr CreateUDP(Address::ptr addr);
        // 创建ipv4tcp套接字
        static Socket::ptr CreateTCPSocket();
        // 创建ipv4udp套接字
        static Socket::ptr CreateUDPSocket();
        // 创建ipv6tcp套接字
        static Socket::ptr CreateTCPSocketIPv6();
        // 创建ipv6udp套接字
        static Socket::ptr CreateUDPSocketIPv6();
        // 创建unix类似tcp套接字
        static Socket::ptr CreateUnixTCPSocket();
        // 创建unix类似udp套接字
        static Socket::ptr CreateUnixUDPSocket();
        Socket(int family, int type, int protocol = 0);
        // bind绑定
        bool Bind(Address::ptr addr);
        // 设置监听
        bool Listen(int size = SOMAXCONN); // max全连接队列长度为4096
        // connect发起连接
        virtual bool Connect(Address::ptr addr, uint64_t timeout = -1);
        // 重新建立连接
        bool ReConnect(uint64_t timeout = -1);
        // 检查是否连接
        bool CheckConnected();
        // 返回是否连接
        bool IsConnected();
        // tcp读函数-单缓冲区
        virtual ssize_t Recv(void *buf, size_t len, int flags = 0);
        // tcp读函数-多缓冲区(iovec *iov为iov数组的指针,iovcnt为数组的大小)
        virtual ssize_t RecvV(struct iovec *iov, int iovcnt, int flags = 0);
        // udp读函数-单缓冲区
        virtual ssize_t RecvFrom(void *buf, size_t len, Address::ptr from, int flags = 0);
        // udp读函数-多缓冲区(iovec *iov为iov数组的指针,iovcnt为数组的大小)
        virtual ssize_t RecvFromV(struct iovec *iov, int iovcnt, Address::ptr from, int flags = 0);
        // tcp写函数-单缓冲区
        virtual ssize_t Send(const void *msg, size_t len, int flags = 0);
        // tcp写函数-多缓冲区
        virtual ssize_t SendV(const struct iovec *iov, int iovcnt, int flags = 0);
        // udp写函数-单缓冲区
        virtual ssize_t SendTo(const void *msg, size_t len, Address::ptr to, int flags = 0);
        // udp写函数-多缓冲区
        virtual ssize_t SendToV(const struct iovec *iov, int iovcnt, Address::ptr to, int flags = 0);
        //  关闭socket连接
        bool Close();
        // 接收连接
        virtual std::shared_ptr<Socket> Accept();
        // 设置socket属性
        template <class T>
        bool Setsockopt(int level, int optname, T &value)
        {
            return Setsockopt(_sockfd, level, optname, (void *)&value, sizeof(value));
        }
        // 获取socket属性
        template <class T>
        bool Getsockopt(int level, int optname, T &value)
        {
            socklen_t len = sizeof value;
            return Getsockopt(_sockfd, level, optname, (void *)&value, &len);
        }
        // 获取本地address
        Address::ptr GetLocalAddress();
        // 获取对端address
        Address::ptr GetPeerAddress();
        // 获取读超时时间
        int64_t GetRecvTimeOut();
        // 设置读超时时间
        bool SetRecvTimeOut(int64_t timeout);
        // 获取写超时时间
        int64_t GetSendTimeOut();
        // 设置写超时时间
        bool SetSendTimeOut(int64_t timeout);
        // 获取错误
        int GetError();
        virtual ~Socket();
        // 获取sockt句柄
        int GetSockFd()
        {
            return _sockfd;
        }
        // 获取协议家族
        int GetFamily()
        {
            return _family;
        }
        // 获取连接类型 tcp or udp
        int GetType()
        {
            return _type;
        }
        // 获取协议类型
        int GetProtocol()
        {
            return _protocol;
        }
        // 取消读事件
        bool CancelRead();
        // 取消写事件
        bool CancelWrite();
        // 取消所有事件
        bool CancelAll();
        // 取消accept
        bool CancelAccept();
        // 输出信息
        virtual std::ostream &dump(std::ostream &os) const;
        virtual std::string tostring() const;

    protected:
        // 内部使用的获取属性方法
        bool Getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
        // 内部使用的设置属性方法
        bool Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
        // 通过sockfd进行初始化
        virtual bool init(int sockfd);
        // 初始化sockfd属性 （1.设置端口复用 2.设置禁用nagle算法）
        bool initSocket();
        // 是否sockfd有效
        bool isValid();
        // 创建socket
        bool newSocket();

    protected:
        int _sockfd;                // socket句柄
        int _family;                // 协议家族
        int _type;                  // 类型（tcp udp）
        int _protocol;              // 协议
        bool _isConnect;            // 是否连接成功
        Address::ptr _localAddress; // 本地地址
        Address::ptr _peerAddress;  // 远端地址
    };
    // SSL安全的socket
    class SSLSocket : public Socket
    {
    public:
        typedef std::shared_ptr<SSLSocket> ptr;
        // 创建加密ipv4tcp套接字
        static SSLSocket::ptr CreateTCP(Address::ptr addr);
        // 创建加密ipv4tcp套接字
        static SSLSocket::ptr CreateTCPSocket();
        // 创建加密ipv6tcp套接字
        static SSLSocket::ptr CreateTCPSocketIPv6();
        SSLSocket(int family, int type, int protocol = 0);
        // connect发起连接
        virtual bool Connect(Address::ptr addr, uint64_t timeout = -1) override;
        // 接收连接
        virtual std::shared_ptr<Socket> Accept() override;
        // tcp读函数-单缓冲区
        virtual ssize_t Recv(void *buf, size_t len, int flags = 0) override;
        // tcp读函数-多缓冲区(iovec *iov为iov数组的指针,iovcnt为数组的大小)
        virtual ssize_t RecvV(struct iovec *iov, int iovcnt, int flags = 0) override;
        // udp读函数-单缓冲区
        virtual ssize_t RecvFrom(void *buf, size_t len, Address::ptr from, int flags = 0) override;
        // udp读函数-多缓冲区(iovec *iov为iov数组的指针,iovcnt为数组的大小)
        virtual ssize_t RecvFromV(struct iovec *iov, int iovcnt, Address::ptr from, int flags = 0) override;
        // tcp写函数-单缓冲区
        virtual ssize_t Send(const void *msg, size_t len, int flags = 0) override;
        // tcp写函数-多缓冲区
        virtual ssize_t SendV(const struct iovec *iov, int iovcnt, int flags = 0) override;
        // udp写函数-单缓冲区
        virtual ssize_t SendTo(const void *msg, size_t len, Address::ptr to, int flags = 0) override;
        // udp写函数-多缓冲区
        virtual ssize_t SendToV(const struct iovec *iov, int iovcnt, Address::ptr to, int flags = 0) override;
        // 加载证书和私钥文件（服务端调用)
        bool LoadCertificates(const std::string &cert_file, const std::string &key_file);
        // 输出信息
        virtual std::ostream &dump(std::ostream &os) const override;
        virtual std::string tostring() const override;

    private:
        // 通过sockfd进行初始化
        virtual bool init(int sockfd) override;

    private:
        // ssl上下文结构
        std::shared_ptr<SSL_CTX> _ctx;
        // ssl加密的操作系统层socket结构（由socketfd进行初始化）
        std::shared_ptr<SSL> _ssl;
    };
    // 流式输出socket内容
    std::ostream &operator<<(std::ostream &os, const Xten::Socket &socket);
    // 流式输出SSLsocket内容
    std::ostream &operator<<(std::ostream &os, const Xten::SSLSocket &socket);
}

#endif