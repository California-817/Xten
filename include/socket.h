#ifndef __XTEN_SOCKET_H__
#define __XTEN_SOCKET_H__
#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
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
        Socket(Socket::FAMILY family,Socket::TYPE type,int protocol=0);
        // bind绑定
        bool Bind(Address::ptr addr);
        // 设置监听
        bool Listen(int size);
        // connect发起连接
        bool Connect(Address::ptr addr,uint64_t timeout=-1);
        // 检查是否连接
        bool CheckConnected();
        // 返回是否连接
        bool IsConnected();
        // 关闭socket连接
        bool Close();
        // 接收连接
        std::shared_ptr<Socket> Accept();
        //设置socket属性
        template<class T>
        bool Setsockopt(int level, int optname,T& value)
        {
            return Setsockopt(_sockfd,sockfd,level,optname,(void*)&value,sizeof T);
        }
        //获取socket属性
        template<class T>
        bool Getsockopt(int level,int optname,T& value)
        {
            socklen_t len=sizeof value;
            return Getsockopt(_sockfd,level,optname,(void*)&value,&len);
        }

        // 获取本地address
        Address::ptr GetLocalAddress();
        // 获取对端address
        Address::ptr GetPeerAddress();
        virtual ~Socket();

    private:
        //内部使用的获取属性方法
        bool Getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
        //内部使用的设置属性方法
        bool Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
        // 通过sockfd进行初始化
        bool init(int sockfd);
        //初始化sockfd属性 （1.设置端口复用 2.设置禁用nagle算法）
        bool initSocket();
        //是否sockfd有效
        bool isValid();
        //创建socket
        bool newSocket();
    private:
        int _sockfd;                // socket句柄
        int _family;                // 协议家族
        int _type;                  // 类型
        int _protocol;              //协议
        bool _isConnect;            // 是否连接成功
        Address::ptr _localAddress; // 本地地址
        Address::ptr _peerAddress;  // 远端地址
    };

}
#endif